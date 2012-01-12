///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004-2009 Gabriel Yoder
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////
#include "AppRover.h"
#include "Interpreter.h"

// Public----------------------------------------------------------------------

/*!
 @param cfgFilename - File containing the AppRover XML configuration data
*/
AppRover::AppRover(string cfgFilename)
{
  string location;
  int i, numRepositories;
  Repository* repository;
  RepositoryStatusType retcode;
  ConfigMgrStatusType configRetcode;
  InstallLogStatusType logRetcode;

  mPackageListIsInvalid = true;

  configRetcode = mConfig.Load(cfgFilename);
  if ( configRetcode == CFG_FILE_MISSING )
  {
    mConfig.Save(cfgFilename);
    mConfig.Load(cfgFilename);
    configRetcode = mConfig.Load(cfgFilename);
  }
  if ( configRetcode != CFG_OK )
    exit(1);
  location = mConfig.GetInstallLogFilename();
  logRetcode = mInstallLog.Load(location);
  if ( logRetcode == LOG_FILE_MISSING )
    logRetcode = mInstallLog.Save(location);
  if ( logRetcode != LOG_OK )
    exit(1);
  numRepositories = mConfig.GetNumRepositories();
  for (i = 0; i < numRepositories; i++)
  {
    location = mConfig.GetRepositoryDirectory(i);
    if ( location != "" )
    {
      repository = new Repository();
      retcode = repository->Load(location);
      if ( retcode == REPO_OK )
      {
	mRepositories.push_back(repository);
      }
      else if ( retcode == REPO_FILE_MISSING )
      {
	mRepositories.push_back(repository);
	repository->Save();
      }
      else
	delete repository;
    }
  }
  PopulatePackageList();
}

AppRover::~AppRover()
{
  int i,numRepositories;

  numRepositories = mRepositories.size();
  for (i = 0; i < numRepositories; i++)
    delete mRepositories[i];
}

/*!
  @param pretend - if true, only generate a list of actions that would be performed
  @param actionList - List to receive the actions if operating in pretend mode
  @return error message

  Installs the latest version of each of the installed packages.  If a package is
  already at the latest revisiion, it is NOT rebuilt.
*/
string AppRover::Update(bool pretend, ActionDescriptionVector& actionList)
{
  string errorMessage = "";
  string revisionStr, packageName;
  int i, numPackages;
  Package* package;
  InstallEntry* oldLogEntry;

  numPackages = NumPackages();
  for (i = 0; i < numPackages; i++)
  {
    revisionStr = GetInstalledRevisionNumber(i);
    if ( revisionStr != "" )
    {
      packageName = GetPackageName(i);
      package = GetPackage(packageName);
      if ( package->GetRevision().GetString() != revisionStr )
      {
        oldLogEntry = mInstallLog.FindEntry(packageName, revisionStr);
        /* The log entry could be missing if this package was already updated as a
           dependency of a previously updated package */
        if ( oldLogEntry && !oldLogEntry->IsExcludedFromUpdates() )
          errorMessage += Install(GetPackageName(i), "", pretend, actionList, STAGE_INSTALL, false, oldLogEntry->IsExplicitlyInstalled());
      }
    }
  }

  return errorMessage;
}

/*!
  @param packageName name of package
  @param state true to exclude, false to include
  @return error message

  Flags whether the specified package should or should not be processed by the Update function.
*/
string AppRover::ExcludeFromUpdates(string packageName, bool state)
{
  string errorMessage = "";
  int i, numEntries;
  vector<InstallEntry*> entries;

  mInstallLog.GetInstalledEntries(packageName, entries);
  numEntries = entries.size();
  if ( numEntries == 0 )
  {
    errorMessage = "No installed versions of " + packageName + " were found\n";
  }
  else
  {
    for (i = 0; i < numEntries; i++)
      entries[i]->SetExcludeFromUpdates(state);
  }

  mInstallLog.Save(mConfig.GetInstallLogFilename());
  return errorMessage;
}

/*!
  @param packageName name of package
  @return error message

  Removes leftover files and rebuilds the package in order to purge remnants of older installed versions of the package.
*/
string AppRover::Purify(string packageName)
{
  string errorMessage = "";
  string installMessage;
  string revisionStr;
  int i, numEntries;
  vector<InstallEntry*> entries;
  ActionDescriptionVector unusedVector;

  mInstallLog.GetInstalledEntries(packageName, entries);
  numEntries = entries.size();
  if ( numEntries == 0 )
  {
    errorMessage = "No installed versions of " + packageName + " were found\n";
  }
  else
  {
    for (i = 0; i < numEntries; i++)
    {
      revisionStr = entries[i]->GetRevision().GetString();
      entries[i]->ConsolidateFiles();
      if ( !entries[i]->PurgeLegacyFiles() )
      {
        errorMessage += "Couldn't remove all legacy files for " + packageName + " " + revisionStr + "\n";
      }
      installMessage = Install(packageName, revisionStr, false, unusedVector, STAGE_INSTALL, true);
      if ( installMessage != "" )
        errorMessage += installMessage;
    }
  }

  mInstallLog.Save(mConfig.GetInstallLogFilename());
  return errorMessage;
}

/*!
  @param packageName - The name of the package to install
  @param pretend - if true, only generate a list of actions that would be performed
  @param actionList - List to receive the actions if operating in pretend mode
  @param haltStage - The last stage of the installation process which should be executed
  @param allowReinstall - If true, the package should be installed even if it has already been installed
  @param explicitInstall - If true, the user has explicitly requested the package to be installed, otherwise it is assumed to be a dependency of another package
*/
string AppRover::Install(string packageName, string revisionStr, bool pretend, ActionDescriptionVector& actionList, AppRoverHaltStageType haltStage, bool allowReinstall, bool explicitInstall)
{
  Package* packageToInstall;
  InstallEntry* logEntry;
  InstallEntry* oldLogEntry = NULL;
  InstallEntry* slotMateEntry;
  int i, numPackageDependencies, numReverseDependencies, numFeatures;
  const PackageDependency *currentDependency;
  InstallEntry pretendEntry;
  ActionDescription* action = NULL;
  FeatureOption* feature = NULL;
  string dependencyResult;
  string listFilename;
  int state;
  bool reconfigured = false;
  bool actionAdded = false;
  InstallEntry* revDepHolder = NULL;
  bool entryIsNew = true;
  Package* depPackage = NULL;
  string errorMsg;

  if ( packageName == "" )
    return "Invalid package name";

  if ( revisionStr == "" )
  {
    packageToInstall = FindMostCurrentPackage(packageName);
    if ( !packageToInstall )
      return "Unknown package";
  }
  else
  {
    packageToInstall = FindPackage(packageName, revisionStr);
    if ( !packageToInstall )
      return "Unknown package or revision";
  }

  logEntry = mInstallLog.FindEntry(packageToInstall->GetName(),packageToInstall->GetRevision().GetString());

  if ( !allowReinstall && logEntry && logEntry->IsInstalled() )
    return "";

  if ( pretend )
  {
    action = new ActionDescription();
    action->SetPackageName(packageName);
    action->SetNewRevision(packageToInstall->GetRevision().GetString());
  }

  if ( !logEntry || logEntry->IsInstalled() )
  {
    if ( pretend )
    {
      if ( logEntry )
      {
        action->SetOldRevision(logEntry->GetRevision().GetString());
        oldLogEntry = logEntry;
      }
      logEntry = &pretendEntry;
    }
    else
    {
      if ( logEntry )
      {
	oldLogEntry = logEntry;
        logEntry = mInstallLog.FindPartiallyInstalledEntry(packageToInstall->GetName(),packageToInstall->GetRevision().GetString());
        if ( logEntry )
        {
          switch ( haltStage )
          {
            case STAGE_FETCH:
              logEntry = NULL;
              break;
            case STAGE_UNPACK:
              if ( logEntry->IsAtLeastUnpacked() )
                logEntry = NULL;
              break;
            case STAGE_CONFIGURE:
              if ( logEntry->IsAtLeastConfigured() )
                logEntry = NULL;
              break;
            case STAGE_BUILD:
              if ( logEntry->IsBuilt() )
                logEntry = NULL;
              break;
            case STAGE_INSTALL:
              entryIsNew = false;
              break;
          }
        }
      }
      if ( !logEntry )
        logEntry = new InstallEntry();
    }

    logEntry->SetName(packageToInstall->GetName());
    logEntry->SetRevision(packageToInstall->GetRevision().GetString());
    logEntry->SetSlot(packageToInstall->GetSlot());
    logEntry->SetBinaryCompatabilityRevision(packageToInstall->GetBinaryCompatabilityRevision());

    if ( !pretend && entryIsNew )
    {
      listFilename = mConfig.GetInstallFileDirectory()+packageToInstall->GetName()+"-"+packageToInstall->GetRevision().GetString();
      logEntry->SetInstallListFilename(listFilename);
      mInstallLog.AddEntry(logEntry);
    }
    numFeatures = packageToInstall->GetNumFeatures();
    for (i = 0; i < numFeatures; i++)
    {
      feature = new FeatureOption(packageToInstall->GetFeature(i));
      if ( oldLogEntry && oldLogEntry->HasFeature(feature->GetName()) )
      {
	if ( oldLogEntry->GetFeatureState(feature->GetName()) )
	  feature->Enable();
	else
	  feature->Disable();
      }
      else
      {
	state = mConfig.GetFeatureState(feature->GetName());
	if ( state != FEATURE_USE_DEFAULT )
	{
	  if ( state == FEATURE_ENABLED )
	    feature->Enable();
	  else
	    feature->Disable();
	}
      }
      logEntry->AddFeature(feature);
    }
  }
  if ( explicitInstall && !logEntry->IsExplicitlyInstalled() )
    logEntry->SetExplicitlyInstalled(true);
  if ( pretend )
    action->SetFeatures(logEntry->GetFeatures());

  if ( !logEntry->IsAtLeastUnpacked() || (haltStage == STAGE_UNPACK) )
  {
    if ( !pretend )
      if ( !FetchPackage(packageToInstall, logEntry) )
        return logEntry->GetErrorMessage();

    if ( haltStage == STAGE_FETCH )
    {
      if ( pretend )
      {
        action->SetAction(ACTION_FETCH);
        actionList.push_back(action);
      }
      return "";
    }

    if ( !pretend )
      mInstallLog.RemovePartiallyInstalledSlotMates(logEntry,mConfig.GetWorkDirectory());

    if ( !pretend )
      if ( !UnpackPackage(packageToInstall, logEntry) )
        return logEntry->GetErrorMessage();

    if ( haltStage == STAGE_UNPACK )
    {
      if ( pretend )
      {
        action->SetAction(ACTION_UNPACK);
        actionList.push_back(action);
      }
      return "";
    }
  }

  if ( !logEntry->IsAtLeastConfigured() || (haltStage == STAGE_CONFIGURE) )
  {
    if ( logEntry->IsAtLeastConfigured() )
    {
      reconfigured = true;
      logEntry->ClearDependencies(mInstallLog);
    }
    numPackageDependencies = packageToInstall->GetNumDependencies();
    for (i = 0; i < numPackageDependencies; i++)
    {
      //! @todo This does a minimal update.  Need to make an option to do a full update or a full reinstall
      currentDependency = packageToInstall->GetDependency(i);
      if ( currentDependency->IsActive(logEntry->GetFeatures()) )
      {
        if ( !mInstallLog.Satisfies(currentDependency) )
        {
          depPackage = FindMostCurrentPackage(currentDependency->GetName(), currentDependency);
          if ( !depPackage )
          {
            if ( pretend )
            {
              errorMsg = "Can't satisfy dependency ";
              errorMsg += currentDependency->GetString();
              action->SetError(errorMsg);
              actionList.push_back(action);
            }
            return "For package " + packageName + ", unable to satisfy dependency: " + currentDependency->GetString();
          }
          dependencyResult = Install(depPackage->GetName(),depPackage->GetRevision().GetString(),pretend,actionList,STAGE_INSTALL,false,false);
          if ( dependencyResult != "" )
          {
            if ( pretend )
            {
              errorMsg = "Can't satisfy dependency ";
              errorMsg += currentDependency->GetString();
              action->SetError(errorMsg);
              actionList.push_back(action);
            }
            return dependencyResult;
          }
        }
      }
    }

    if ( !pretend )
    {
      if ( !ConfigurePackage(packageToInstall, logEntry) )
        return logEntry->GetErrorMessage();

      numPackageDependencies = packageToInstall->GetNumDependencies();
      for (i = 0; i < numPackageDependencies; i++)
      {
        currentDependency = packageToInstall->GetDependency(i);
        if ( currentDependency->IsActive(logEntry->GetFeatures()) )
          logEntry->AddDependency(*currentDependency);
      }

      if ( !reconfigured )
      {
        for (i = 0; i < numPackageDependencies; i++)
        {
          currentDependency = packageToInstall->GetDependency(i);
          if ( currentDependency->IsActive(logEntry->GetFeatures()) )
            mInstallLog.RegisterReverseDependency(packageToInstall->GetName(),packageToInstall->GetRevision().GetString(),currentDependency);
        }
      }
      mInstallLog.Save(mConfig.GetInstallLogFilename());
    }

    if ( haltStage == STAGE_CONFIGURE )
    {
      if ( pretend )
      {
        action->SetAction(ACTION_CONFIGURE);
        actionList.push_back(action);
      }
      return "";
    }
  }

  if ( !logEntry->IsBuilt() || (haltStage == STAGE_BUILD) )
  {
    if ( !pretend )
      if ( !BuildPackage(packageToInstall, logEntry) )
        return logEntry->GetErrorMessage();

    if ( haltStage == STAGE_BUILD )
    {
      if ( pretend )
      {
        action->SetAction(ACTION_BUILD);
        actionList.push_back(action);
      }
      return "";
    }
  }

  slotMateEntry = mInstallLog.FindInstalledSlotOccupant(packageToInstall->GetName(), packageToInstall->GetSlot());
  if ( slotMateEntry )
  {
    if ( pretend )
    {
      if ( slotMateEntry->GetRevision() == packageToInstall->GetRevision() )
        action->SetAction(ACTION_REINSTALL);
      else
        action->SetAction(ACTION_UPDATE);
      actionList.push_back(action);
      actionAdded = true;
      revDepHolder = slotMateEntry;
    }
    else
    {
      slotMateEntry->TransferReverseDependenciesTo(logEntry);
      slotMateEntry->TransferFilesTo(logEntry);
      revDepHolder = logEntry;
    }

    numReverseDependencies = revDepHolder->GetNumReverseDependencies();
    for (i=0; i<numReverseDependencies; i++)
    {
      if ( revDepHolder->ReverseDependencyIsStatic(i) ||
           (slotMateEntry->GetBinaryCompatabilityRevision() !=
            logEntry->GetBinaryCompatabilityRevision()) )
        Install(revDepHolder->GetReverseDependencyName(i),"",pretend,actionList,
                STAGE_INSTALL,true);
    }

    if ( !pretend )
      mInstallLog.DeleteEntry(slotMateEntry,false);
  }
  if ( pretend )
  {
    if ( !actionAdded )
    {
      if ( action->GetAction() == ACTION_NONE )
        action->SetAction(ACTION_INSTALL);
      actionList.push_back(action);
    }
    return "";
  }
  if ( !InstallPackage(packageToInstall, logEntry) )
  {
    PopulatePackageList();
    return logEntry->GetErrorMessage();
  }

  PopulatePackageList();
  return "";
}

/*!
  @param packageName - The name of the package to update
  @param pretend - If true, generates a list of actions which would be performed
  @param actionList - List which receives the actions if operating in pretend mode
  @return string containing an error message ("" if successful)

  Uninstalls all versions of the specified package.
*/
string AppRover::Uninstall(string packageName, bool pretend, ActionDescriptionVector& actionList)
{
  InstallEntry* logEntry;
  ActionDescription* action;
  string errorMsg;
  //! @todo May want to add a means to protect some versions

  logEntry = mInstallLog.FindOldestInstalledEntry(packageName);
  if ( !logEntry )
    return "Package is not installed";
  if ( pretend )
  {
    while ( logEntry )
    {
      action = new ActionDescription();
      action->SetPackageName(packageName);
      action->SetOldRevision(logEntry->GetRevision().GetString());
      action->SetAction(ACTION_UNINSTALL);
      actionList.push_back(action);
      logEntry = mInstallLog.FindNextOldestInstalledEntry(logEntry);
    }
    return "";
  }
  while ( logEntry )
  {
    errorMsg = logEntry->Uninstall(false);
    if ( errorMsg != "" )
    {
      PopulatePackageList();
      mInstallLog.Save(mConfig.GetInstallLogFilename());
      return errorMsg;
    }
    mInstallLog.DeleteEntry(logEntry);
    logEntry = mInstallLog.FindOldestInstalledEntry(packageName);
  }
  PopulatePackageList();
  mInstallLog.Save(mConfig.GetInstallLogFilename());
  return "";
}

/*!
  @param pretend - If true, generates a list of actions to be performed
  @param actionList - List which receives the actions if operating in pretend mode
  @param keepPartials - If true, partially installed packages are unaffected
  @return string containing any error message

  Removes all partially installed packages and uninstalls packages which are no longer needed (i.e. not explicity installed and is not a dependency of an explicity installed package).
*/
string AppRover::Clean(bool pretend, ActionDescriptionVector& actionList, bool keepPartials)
{
  int i, numPackages;
  InstallEntryVector packages;
  ActionDescription* action;
  string errorMsg = "";

  if ( !keepPartials )
  {
    packages = mInstallLog.GetPartiallyInstalledPackages();
    numPackages = packages.size();
    for (i = 0; i < numPackages; i++)
    {
      if ( pretend )
      {
        action = new ActionDescription();
        action->SetPackageName(packages[i]->GetName());
        action->SetOldRevision(packages[i]->GetRevision().GetString());
        action->SetAction(ACTION_CLEAN);
        actionList.push_back(action);
      }
      else
      {
        mInstallLog.DeleteEntry(packages[i]);
        mInstallLog.Save(mConfig.GetInstallLogFilename());
      }
    }
    if ( !pretend )
    {
      if ( !gCommandInterpreter->Interpret("rm -rf " + mConfig.GetWorkDirectory() + "/*") )
        return "Unable to clear out " + mConfig.GetWorkDirectory();
    }
  }

  packages = mInstallLog.GetUnneededPackages();
  numPackages = packages.size();
  for (i = 0; i < numPackages; i++)
  {
    if ( pretend )
    {
      action = new ActionDescription();
      action->SetPackageName(packages[i]->GetName());
      action->SetOldRevision(packages[i]->GetRevision().GetString());
      action->SetAction(ACTION_UNINSTALL);
      actionList.push_back(action);
    }
    else
    {
      errorMsg = packages[i]->Uninstall(false);
      if ( errorMsg != "" )
        return errorMsg;
      mInstallLog.DeleteEntry(packages[i]);
      mInstallLog.Save(mConfig.GetInstallLogFilename());
    }
  }

  PopulatePackageList();
  return errorMsg;
}

/*!
  @param baseLocation - The directory containing the repository.cfg file
  @return false if directory is not specified, is not writable, or the repository already exists

  Creates an empty repository in the specified directory.
*/
bool AppRover::AddNewRepository(string baseLocation)
{
  int i, numRepositories;

  if ( baseLocation == "" )
    return false;

  numRepositories = mRepositories.size();
  for (i=0; i<numRepositories; i++)
    if ( baseLocation == mRepositories[i]->GetDirectory() )
      return false;

  Repository *newRepository = new Repository();
  newRepository->SetDirectory(baseLocation);
  if ( newRepository->Save() != REPO_OK )
    return false;
  mRepositories.push_back(newRepository);
  mConfig.AddRepositoryDirectory(baseLocation);
  mConfig.Save();
  return true;
}

/*!
  @param packageName - the name of the application whose package should be retrieved
  @param revision - the revision of the application handled by the package
  @param repositoryNumber - the index representing the repository from which the package should be retrieved.
*/
Package* AppRover::GetPackage(string packageName, string revision, int repositoryNumber)
{
  if ( (repositoryNumber >= (int)mRepositories.size()) || (repositoryNumber < 0) )
    return NULL;
  return mRepositories[repositoryNumber]->GetPackage(packageName, revision);
}

/*!
  @param packageName - the name of the application whose package should be retrieved
  @param revision - the upper bound revision of the application handled by the package
  @param repositoryNumber - the index representing the repository from which the package should be retrieved.
*/
Package* AppRover::GetPrecedingPackage(string packageName, string revision, int repositoryNumber)
{
  if ( (repositoryNumber >= (int)mRepositories.size()) || (repositoryNumber < 0) )
    return NULL;
  return mRepositories[repositoryNumber]->GetPrecedingPackage(packageName, revision);
}

/*!
  @param packageName - the name of the application whose latest package should be retrieved
  @param repositoryNumber - the index representing the repository from which the package should be retrieved.
*/
Package* AppRover::GetPackage(string packageName, int repositoryNumber)
{
  if ( (repositoryNumber >= (int)mRepositories.size()) || (repositoryNumber < 0) )
    return NULL;
  return mRepositories[repositoryNumber]->FindMostCurrentPackage(packageName);
}

/*!
  @param newPackage - the package to insert into the repository
  @param repositoryNumber - the index of the repository to receive the package
  @return false if null package, invalid repositoryNumber or package is a
          duplicate of another in the repository

  Adds a package into the specified repository
*/
bool AppRover::AddNewPackage(Package* newPackage, int repositoryNumber)
{
  if ( !newPackage || (repositoryNumber >= (int)mRepositories.size()) )
    return false;
  newPackage->RecordSourceChecksums(mConfig.GetStorageDirectory());
  if ( mRepositories[repositoryNumber]->AddPackage(newPackage) )
  {
    mPackageListIsInvalid = true;
    PopulatePackageList();
    return true;
  }
  return false;
}

/*!
  @param package - the package whose source file checksums need to be updated

  Recalculates and records the checksums for all files which are required to install the package
*/
void AppRover::UpdatePackageChecksums(Package* package)
{
  if ( package )
    package->RecordSourceChecksums(mConfig.GetStorageDirectory());
}

/*!
  @return The number of unique package names in all combined repositories
*/
int AppRover::NumPackages()
{
  return mPackageList.size();
}

/*!
  @param packageIndex - the index into the list of package names
  @return "" if invalid index, otherwise the name of an available package
*/
string AppRover::GetPackageName(int packageIndex)
{
  if ( packageIndex >= (int)mPackageList.size() )
    return "";
  return mPackageList[packageIndex];
}

/*!
  @param packageIndex - the index into the list of package names
  @return string containing the revision number that has been installed for the specified package ("" if invalid index or package has not been installed)
*/
string AppRover::GetInstalledRevisionNumber(int packageIndex)
{
  if ( packageIndex >= (int)mRevisionList.size() )
    return "";
  return mRevisionList[packageIndex];
}

/*!
  @param featureName - the name of the optional feature to activate
  @param activationState - the state to which the feature should be set (true to activate, false to deactivate)
  @return the names of the installed packages which may use the optional feature

  Sets the default state for whether the specified optional feature should be activated for a clean install of a package.  Packages which are already installed are not affected.
*/
vector<string> AppRover::ActivateFeature(string featureName, bool activationState)
{
  int i, numPackages;
  InstallEntryVector packages;
  vector<string> packageNames;

  mConfig.SetFeatureState(featureName,activationState);
  mConfig.Save();
  packages = mInstallLog.GetPackagesUsingFeature(featureName);

  numPackages = packages.size();
  for (i = 0; i < numPackages; i++)
  {
    if ( packages[i]->GetFeatureState(featureName) == !activationState )
    {
      // Partially installed packages must either be completed and then
      // activated, or else they must be manually purged and reinstalled
      // in order to receive the effect of the activation.
      if ( packages[i]->IsInstalled() )
        packageNames.push_back(packages[i]->GetName());
    }
  }

  return packageNames;
}

/*!
  @todo this function should be removed and activated feature should be passed as an argument to Install
*/
void AppRover::ActivateFeatureForPackage(string packageName, string featureName, bool activationState)
{
  int i, numPackages;
  InstallEntryVector packages;

  packages = mInstallLog.GetPackagesUsingFeature(featureName);

  numPackages = packages.size();
  for (i = 0; i < numPackages; i++)
  {
    if ( (packages[i]->GetName() == packageName) &&
         packages[i]->IsInstalled() )
    {
      if ( activationState )
	packages[i]->ActivateFeature(featureName);
      else
	packages[i]->DeactivateFeature(featureName);
    }
  }
}

/*!
  @return string containing any error message

  Updates all repositories to match their sources (not applicable for local repositories).
*/
string AppRover::SynchronizeRepositories()
{
  int i, numRepositories, retcode;
  string location;
  string errorMsg = "";
  Repository* repository = NULL;
  string resultStr;

  numRepositories = mRepositories.size();
  for (i = 0; i < numRepositories; i++)
  {
    resultStr = mRepositories[i]->Synchronize();
    if ( resultStr != "" )
    {
      if ( errorMsg != "" )
        errorMsg += "\n";
      errorMsg += resultStr;
    }
    delete mRepositories[i];
  }
  mRepositories.clear();

  numRepositories = mConfig.GetNumRepositories();
  for (i=0; i<numRepositories; i++)
  {
    location = mConfig.GetRepositoryDirectory(i);
    if ( location != "" )
    {
      repository = new Repository();
      retcode = repository->Load(location);
      if ( retcode == REPO_OK )
        mRepositories.push_back(repository);
      else if ( retcode == REPO_FILE_MISSING )
      {
        mRepositories.push_back(repository);
        repository->Save();
      }
      else
      {
        if ( errorMsg != "" )
          errorMsg += "\n";
        errorMsg += "Error loading repository from " + repository->GetDirectory();
        delete repository;
      }
    }
  }

  return errorMsg;
}

// Private---------------------------------------------------------------------

/*!
  @param packageName - the name of the package
  @param dependency - the dependency requirements
  @return  The package with the highest revision or the package with the
           highest revision which satisfies the dependency

  Locates the most current revision of the package with the specified name.
  If dependency is non-null, it locates the most current revision which
  statisfies the dependency.
*/
Package* AppRover::FindMostCurrentPackage(string packageName,
                                          const PackageDependency *dependency)
{
  int i, numRepositories;
  Package* currentPackage = NULL;
  Package* bestPackage = NULL;

  numRepositories = mRepositories.size();
  for (i = 0; i < numRepositories; i++)
  {
    currentPackage = mRepositories[i]->FindMostCurrentPackage(packageName, dependency);
    if ( currentPackage && currentPackage->IsNewerThan(bestPackage) &&
         (!dependency || dependency->IsSatisfiedBy(currentPackage)) )
      bestPackage = currentPackage;
  }
  return bestPackage;
}

/*!
  @param packageName - the name of the package
  @param revisionStr - the desired revision of the package
  @return  The package specified name and revision
*/
Package* AppRover::FindPackage(string packageName, string revisionStr)
{
  int i, numRepositories;
  Package* currentPackage = NULL;

  numRepositories = mRepositories.size();
  for (i = 0; i < numRepositories; i++)
  {
    currentPackage = mRepositories[i]->GetPackage(packageName, revisionStr);
    if ( currentPackage )
      return currentPackage;
  }
  return NULL;
}

/*!
  @param package - the package to fetch
  @param logEntry - the entry in the install log corresponding to the package
  @param force - if true, fetch all files even if a local copy exists
  @return false if there is a failure in fetching the files

  Downloads all files needed to install the package and places them in the
  storage directory.  If force is false, only the files which are not already
  stored locally will be downloaded.
*/
bool AppRover::FetchPackage(const Package* package, InstallEntry* logEntry, bool force)
{
  bool retCode;

  retCode = package->Fetch(logEntry, mConfig.GetStorageDirectory(), force);
  mInstallLog.Save(mConfig.GetInstallLogFilename());

  return retCode;
}

/*!
  @param package - the package to unpack
  @param logEntry - the entry in the install log corresponding to the package

  @return false if there is a failure

  Fetches the package if needed.
  Creates a working directory for the package and copies all of the needed files from the storage directory to the working directory.
  If the package contains any specific unpacking instructions (like untarring, uncompressing, or patching files) they are then executed in the working directory.
*/
bool AppRover::UnpackPackage(Package* package, InstallEntry* logEntry)
{
  bool retCode;

  if ( !FetchPackage(package, logEntry) )
    return false;

  retCode = package->Unpack(logEntry, mConfig.GetStorageDirectory(), mConfig.GetWorkDirectory(), mConfig.GetNumProcessors());
  mInstallLog.Save(mConfig.GetInstallLogFilename());

  return retCode;
}

/*!
  @param package - the package to configure
  @param logEntry - the entry in the install log corresponding to the package
  @return false if there is a failure

  Unpacks the package if needed.
  Processes any package specific configuration instructions and executes them from the package's working directory.  This should only be needed for source-based packages.  In most cases where this is used, the instructions will call ./configure with correct options.
*/
bool AppRover::ConfigurePackage(Package* package, InstallEntry* logEntry)
{
  bool retCode;

  if ( !logEntry->IsAtLeastUnpacked() && !UnpackPackage(package, logEntry) )
    return false;

  retCode = package->Configure(logEntry, mConfig.GetWorkDirectory(), mConfig.GetNumProcessors());
  mInstallLog.Save(mConfig.GetInstallLogFilename());

  return retCode;
}

/*!
  @param package - the package to build
  @param logEntry - the entry in the install log corresponding to the package
  @return - false if there is a failure

  Configures the package if needed.
  Processes any package specific building instructions and executes them from the package's working directory.  This should only be needed for source-based packages.
*/
bool AppRover::BuildPackage(Package* package, InstallEntry* logEntry)
{
  bool retCode;

  if ( !logEntry->IsAtLeastConfigured() && !ConfigurePackage(package, logEntry) )
    return false;

  retCode = package->Build(logEntry, mConfig.GetWorkDirectory(), mConfig.GetNumProcessors());
  mInstallLog.Save(mConfig.GetInstallLogFilename());

  return retCode;
}

/*!
  @param package - the package to build
  @param logEntry - the entry in the install log corresponding to the package
  @return false if there is a failure

  Builds the package if needed.
  Processes any package specific building instructions and executes them from the package's working directory.
*/
bool AppRover::InstallPackage(Package* package, InstallEntry* logEntry)
{
  bool retCode;

  if ( !logEntry->IsBuilt() && !BuildPackage(package, logEntry) )
    return false;

  retCode = package->Install(logEntry, mConfig.GetWorkDirectory(), mConfig.GetNumProcessors());
  mInstallLog.Save(mConfig.GetInstallLogFilename());

  return retCode;
}

/*!
  Generates a consolidated list of packages and revisions for use with the functions GetNumPackages, GetPackageName, and GetInstalledRevisionNumber.
*/
void AppRover::PopulatePackageList()
{
  int i, j;
  int numPackages, numRepositories;

  if ( mPackageListIsInvalid )
  {
    mPackageList.clear();
    mRevisionList.clear();
    //! @todo Sort the list and make sure there are no duplicate entries
    numRepositories = mRepositories.size();
    for (i = 0; i < numRepositories; i++)
    {
      numPackages = mRepositories[i]->NumPackages();
      for (j = 0; j < numPackages; j++)
      {
        mPackageList.push_back(mRepositories[i]->GetPackageName(j));
        mRevisionList.push_back("");
      }
    }
    mPackageListIsInvalid = false;
  }
  numPackages = mPackageList.size();
  for (i = 0; i < numPackages; i++)
    mRevisionList[i] = mInstallLog.GetRevisionInstalled(mPackageList[i]);
}
