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
#include <string.h>
#include <unistd.h>
#include "InstallEntry.h"
#include "InstallLog.h"
#include "Labels.h"
#include "Interpreter.h"
#include "DirectoryUtils.h"
#include "XMLUtils.h"
#include "FeatureOption.h"

// Public----------------------------------------------------------------------

InstallEntry::InstallEntry()
{
  mPackageName = "";
  mPackageSlot = "";
  mBinaryCompatabilityRevision = 0;
  mPackageStatus = "";
  mInstalledInThisSession = false;
  mErrorMessage = "";
  mExplicitInstall = false;
  mExcludeFromUpdates = false;
  mInstallListFilename = "";
}

InstallEntry::~InstallEntry()
{
  int i, numDependencies, numFeatures;

  numDependencies = mDependencies.size();
  for (i = 0; i < numDependencies; i++)
    delete mDependencies[i];
  numFeatures = mFeatures.size();
  for (i = 0; i < numFeatures; i++)
    delete mFeatures[i];
  numDependencies = mReverseDependencies.size();
  for (i = 0; i < numDependencies; i++)
    delete mReverseDependencies[i];
}

/*!
  @param parentNode - the parent DOM node to hold the InstallEntry node
  @return false if parentNode is NULL
  @return true otherwise

  Saves the install entry data as an XML DOM node that is a child of the specified parent node.
*/
bool InstallEntry::Save(xmlNodePtr parentNode)
{
  FILE *fp;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  xmlNodePtr entryNode;
  xmlNodePtr childNode;
  int i, numFiles, numDirectories, numSymLinks, numDependencies, numFeatures;

  if ( !parentNode )
    return false;

  entryNode = xmlNewNode(NULL,(const xmlChar*)ENTRY_STR);

  SetStringValue(entryNode, PACKAGE_NAME_STR, mPackageName);
  SetStringValue(entryNode, REVISION_STR, mPackageRevision.GetString());
  SetStringValue(entryNode, SLOT_STR, mPackageSlot);
  SetBoolValue(entryNode, EXPLICIT_INSTALL_STR, mExplicitInstall);
  SetBoolValue(entryNode, EXCLUDE_STR, mExcludeFromUpdates);
  SetIntValue(entryNode, BINARY_COMPATABILITY_REVISION_STR, mBinaryCompatabilityRevision);
  SetStringValue(entryNode, STATUS_STR, mPackageStatus);
  SetStringValue(entryNode, INSTALL_LIST_FILE_STR, mInstallListFilename);
  SetStringValue(entryNode, ERROR_MSG_STR, mErrorMessage);

  numDependencies = mDependencies.size();
  for (i = 0; i < numDependencies; i++)
    if ( !mDependencies[i]->Save(entryNode) )
      return false;

  numFeatures = mFeatures.size();
  for (i = 0; i < numFeatures; i++)
    if ( !mFeatures[i]->Save(entryNode) )
      return false;

  numDependencies = mReverseDependencies.size();
  for (i = 0; i < numDependencies; i++)
    if ( !mReverseDependencies[i]->Save(entryNode) )
      return false;

  if ( !mUninstallInstructions.IsEmpty() )
    mUninstallInstructions.Save(entryNode,UNINSTALL_STR);

  xmlAddChild(parentNode,entryNode);

  if ( IsInstalled() && mInstalledInThisSession )
  {
    numFiles = mInstalledFiles.size();
    numSymLinks = mInstalledSymLinks.size();
    numDirectories = mInstalledDirectories.size();
    fp = fopen(mInstallListFilename.c_str(),"w");
    if ( !fp )
      return false;

    document = xmlNewDoc(NULL);
    rootNode = xmlNewDocNode(document,NULL,(const xmlChar *)LIST_HDR_STR,NULL);
    SetDoubleValue(rootNode, VERSION_STR, 1);
    xmlDocSetRootElement(document,rootNode);

    for (i = 0; i < numFiles; i++)
      mInstalledFiles[i]->Save(rootNode);
    for (i = 0; i < numSymLinks; i++)
    {
      childNode = xmlNewNode(NULL,(const xmlChar*)SYM_LINK_STR);
      SetStringValue(childNode, LOCATION_STR, mInstalledSymLinks[i]);
      xmlAddChild(rootNode,childNode);
    }
    for (i = 0; i < numDirectories; i++)
    {
      childNode = xmlNewNode(NULL,(const xmlChar*)DIR_STR);
      SetStringValue(childNode, LOCATION_STR, mInstalledDirectories[i]);
      xmlAddChild(rootNode,childNode);
    }

    xmlDocFormatDump(fp,document,1);
    fclose(fp);
    xmlFreeDoc(document);
  }

  return true;
}

/*!
  @param entryNode - the xml DOM node to containing the install entry data
  @return false if entryNode is NULL, or there is a probelm with the data
  @return true otherwise

  Loads the install entry data from an XML DOM node.
*/
bool InstallEntry::Load(xmlNodePtr entryNode)
{
  xmlNodePtr currentNode;
  PackageDependency* dependency;
  FeatureOption* feature;
  string location;
  string packageName, packageRevision;
  string depType;

  if ( !entryNode )
    return false;

  mPackageName = GetStringValue(entryNode, PACKAGE_NAME_STR);
  if ( mPackageName == "" )
    return false;

  packageRevision = GetStringValue(entryNode, REVISION_STR);
  if ( packageRevision == "" )
    return false;
  mPackageRevision.Set(packageRevision);

  mPackageSlot = GetStringValue(entryNode, SLOT_STR);
  mExplicitInstall = GetBoolValue(entryNode, EXPLICIT_INSTALL_STR, true);
  mExcludeFromUpdates = GetBoolValue(entryNode, EXCLUDE_STR, false);
  mBinaryCompatabilityRevision = GetIntValue(entryNode,BINARY_COMPATABILITY_REVISION_STR, 0);

  mPackageStatus = GetStringValue(entryNode, STATUS_STR);
  if ( mPackageStatus == "" )
    return false;

  mInstallListFilename = GetStringValue(entryNode, INSTALL_LIST_FILE_STR);
  mErrorMessage = GetStringValue(entryNode, ERROR_MSG_STR);

  currentNode = entryNode->children;
  while ( currentNode )
  {
    if ( !strcmp((char *)currentNode->name, REVERSE_DEPENDENCY_STR) )
    {
      dependency = new PackageDependency();
      if ( !dependency )
        return false;
      if ( !dependency->Load(currentNode) )
      {
        delete dependency;
        return false;
      }
      mReverseDependencies.push_back(dependency);
    }
    else if ( !strcmp((char *)currentNode->name, DEPENDENCY_STR) )
    {
      dependency = new PackageDependency();
      if ( !dependency )
        return false;
      if ( !dependency->Load(currentNode) )
      {
        delete dependency;
        return false;
      }
      mDependencies.push_back(dependency);
    }
    else if ( !strcmp((char *)currentNode->name, INSTRUCTION_STR) )
    {
      if ( !mUninstallInstructions.Load(currentNode) )
        return false;
    }
    else if ( !strcmp((char *)currentNode->name, FEATURE_STR) )
    {
      feature = new FeatureOption();
      if ( !feature->Load(currentNode) )
      {
        delete feature;
        return false;
      }
      mFeatures.push_back(feature);
    }
    currentNode = currentNode->next;
  }

  return true;
}

/*!
  @return the name of the package recorded in this log entry
*/
string InstallEntry::GetName() const
{
  return mPackageName;
}

/*!
  @param name - name of the package stored in this entry

  Records the package name into the install entry.
*/
void InstallEntry::SetName(string name)
{
  mPackageName = name;
}

/*!
  return the revision of the package recorded in this log entry
*/
const Revision& InstallEntry::GetRevision() const
{
  return mPackageRevision;
}

/*!
  @param revision - revision of the package stored in this entry

  Records the package revision into the install entry.
*/
void InstallEntry::SetRevision(string revision)
{
  mPackageRevision.Set(revision);
}

/*!
  @return string containing the slot occupied by this package
*/
string InstallEntry::GetSlot() const
{
  return mPackageSlot;
}

/*!
  @param slot - the slot occupied by this package
*/
void InstallEntry::SetSlot(string slot)
{
  mPackageSlot = slot;
}

/*!
  @return integer used to track whether to revisions have binary compatable interfaces
*/
int InstallEntry::GetBinaryCompatabilityRevision() const
{
  return mBinaryCompatabilityRevision;
}

/*!
  @param revision - integer used to track whether to revisions have binary compatable interfaces
*/
void InstallEntry::SetBinaryCompatabilityRevision(int revision)
{
  mBinaryCompatabilityRevision = revision;
}

/*!
  @return true if this package was installed due to an explicit request from the user
*/
bool InstallEntry::IsExplicitlyInstalled()
{
  return mExplicitInstall;
}

/*!
  @param state - true if this package was installed due to an explicit request from the user
*/
void InstallEntry::SetExplicitlyInstalled(bool state)
{
  mExplicitInstall = state;
}

/*!
  @return true if this package should not be auto-updated
*/
bool InstallEntry::IsExcludedFromUpdates()
{
  return mExcludeFromUpdates;
}

/*!
  @param state - true if this package should not be auto-updated
*/
void InstallEntry::SetExcludeFromUpdates(bool state)
{
  mExcludeFromUpdates = state;
}

/*!
  @return true if the package recorded in this entry has just completed the unpack stage of installation
*/
bool InstallEntry::IsUnpacked() const
{
  return (mPackageStatus == STATUS_UNPACKED_STR);
}

/*!
  @return true if the package recorded in this entry has completed the unpack stage and possibly the stages following unpack
*/
bool InstallEntry::IsAtLeastUnpacked() const
{
  return (mPackageStatus != "");
}

/*!
  Flags the log indicating that the package has just completed the unpacking stage.
*/
void InstallEntry::SetUnpacked()
{
  mPackageStatus = STATUS_UNPACKED_STR;
}

/*!
  @return - true if the package recorded in this entry has just completed the configure stage of installation
*/
bool InstallEntry::IsConfigured() const
{
  return (mPackageStatus == STATUS_CONFIGURED_STR);
}

/*!
  @return - true if the package recorded in this entry has completed the configure stage and possibly the stages following configure
*/
bool InstallEntry::IsAtLeastConfigured() const
{
  return (IsAtLeastUnpacked() && (mPackageStatus != STATUS_UNPACKED_STR));
}

/*!
  Flags the log indicating that the package has just completed the configuration stage.
*/
void InstallEntry::SetConfigured()
{
  mPackageStatus = STATUS_CONFIGURED_STR;
}

/*!
  @return - true if the package recorded in this entry has just completed the build stage of installation
*/
bool InstallEntry::IsBuilt() const
{
  return (mPackageStatus == STATUS_BUILT_STR);
}

/*!
  Flags the log indicating that the package has just completed the build stage.
*/
void InstallEntry::SetBuilt()
{
  mPackageStatus = STATUS_BUILT_STR;
}

/*!
  @return true if the package has been installed on the system
*/
bool InstallEntry::IsInstalled() const
{
  return (mPackageStatus == STATUS_INSTALLED_STR);
}

/*!
  Flags the log indicating that the package has been installed
*/
void InstallEntry::SetInstalled()
{
  mPackageStatus = STATUS_INSTALLED_STR;
  mInstalledInThisSession = true;
}

/*!
  return the last recorded error message from one of the installation stages
*/
string InstallEntry::GetErrorMessage() const
{
  return mErrorMessage;
}

/*!
  @param errMsg - the error message

  Records the error message produced by one of the installation stages
*/
void InstallEntry::SetErrorMessage(string errMsg)
{
  mErrorMessage = errMsg;
}

/*!
  @return the error string or "" if successful

  Executes the recorded uinstall instructions if they have been specified.  Otherwise, removes all files and directories installed by the package.  If backup is true (default), any files which have a different md5sum than when they were installed will be backed up to files named .&lt;original name>.arbak
*/
string InstallEntry::Uninstall(bool backup)
{
  int i, numFiles, numDirectories, numSymLinks, numReverseDependencies;

  numReverseDependencies = mReverseDependencies.size();
  if ( numReverseDependencies )
  {
    mErrorMessage = "Package is needed by: ";
    for (i = 0; i < numReverseDependencies; i++)
    {
      if ( i )
        mErrorMessage += ", ";
      mErrorMessage += mReverseDependencies[i]->GetDependentString();
    }
    return mErrorMessage;
  }

  gShowProcessingStage("Uninstalling " + mPackageName + " " + mPackageRevision.GetString());

  if ( !mUninstallInstructions.IsEmpty() )
    mUninstallInstructions.Execute(this, 1, false);

  if ( mInstalledFiles.size() == 0 )
  {
    if ( !LoadFileList() )
    {
      mErrorMessage = "Could not load file list from " + mInstallListFilename;
      return mErrorMessage;
    }
  }
  numFiles = mInstalledFiles.size();
  for (i = 0; i < numFiles; i++)
    if ( backup && mInstalledFiles[i]->NeedsBackedUp() )
    {
      gCommandInterpreter->Interpret(MOVE_STR + ("\"" + mInstalledFiles[i]->GetFilename() + "\" " +
                                     mInstalledFiles[i]->GetBackupFilename() + "\""));
    }
    else
      gCommandInterpreter->Interpret(REMOVE_STR + ("\"" + mInstalledFiles[i]->GetFilename() + "\""));
  numSymLinks = mInstalledSymLinks.size();
  for (i = 0; i < numSymLinks; i++)
    gCommandInterpreter->Interpret(REMOVE_STR + ("\"" + mInstalledSymLinks[i] + "\""));
  numDirectories = mInstalledDirectories.size();
  for (i = 0; i < numDirectories; i++)
    gCommandInterpreter->Interpret(REMOVE_DIR_STR + ("\"" + mInstalledDirectories[i] + "\""));
  if ( mInstallListFilename != "" )
    gCommandInterpreter->Interpret(REMOVE_STR + mInstallListFilename);
  mPackageStatus = "";
  mErrorMessage = "";

  return mErrorMessage;
}

/*!
  @param filename - the file listing the files installed by this package
*/
void InstallEntry::SetInstallListFilename(string filename)
{
  mInstallListFilename = filename;
}

/*!
  @param filename - the full path of an installed file

  Records a file that has been installed by the package.
*/
void InstallEntry::AddInstalledFile(string filename)
{
  InstalledFile* file;
  string checksum = GetChecksum(filename);
  if ( checksum == "" )
    return;
  file = new InstalledFile(filename);
  file->SetChecksum(checksum);
  mInstalledFiles.push_back(file);
}

/*!
  @param linkname - the full path of an installed symbolic link

  Records a symbolic link that has been installed by the package.
*/
void InstallEntry::AddInstalledSymLink(string linkname)
{
  int i, size;

  size = mInstalledSymLinks.size();
  for (i = 0; i < size; i++)
  {
    if ( linkname == mInstalledSymLinks[i] )
      return;
  }
  mInstalledSymLinks.push_back(linkname);
}

/*!
  @param directory - the full path of an installed directory

  Records a directory that has been installed by the package.
*/
void InstallEntry::AddInstalledDirectory(string directory)
{
  int i, size;

  size = mInstalledDirectories.size();
  for (i=0; i<size; i++)
  {
    if ( directory == mInstalledDirectories[i] )
      return;
    if ( directory > mInstalledDirectories[i] )
    {
      mInstalledDirectories.insert(mInstalledDirectories.begin()+i,directory);
      return;
    }
  }
  mInstalledDirectories.push_back(directory);
}

/*!
  @param target - the install entry to accept ownership of the files, directories, and symlinks associated with this entry
  @return false if target is NULL

  Transfers ownership of all files, directories, and symbolic links associated with this entry to the target entry.  Since it is expected that these files are about to be overwritten by a new install, any files which have a different md5sum than when they were installed will be backed up to files named .&lt;original name>.arbak
*/
bool InstallEntry::TransferFilesTo(InstallEntry* target)
{
  int i, numFiles, numDirectories, numSymLinks;
  string backupFilename;
  InstalledFile* backupFile;

  if ( !target )
    return false;

  if ( mInstalledFiles.size() == 0 )
    LoadFileList(); // It doesn't matter much if this fails

  numFiles = mInstalledFiles.size();
  for (i = 0; i < numFiles; i++)
  {
    if ( !FileExists(mInstalledFiles[i]->GetFilename()) )
    {
      delete mInstalledFiles[i];
      continue;
    }
    if ( mInstalledFiles[i]->NeedsBackedUp() )
    {
      backupFilename = mInstalledFiles[i]->GetBackupFilename();
      gCommandInterpreter->Interpret(COPY_STR + ("\"" + mInstalledFiles[i]->GetFilename() + "\" " +
                                     "\"" + backupFilename + "\""));
      backupFile = new InstalledFile(backupFilename);
      backupFile->SetInherited(true);
      backupFile->SetBackup(true);
      backupFile->SetChecksum(GetChecksum(backupFilename));
      target->mInstalledFiles.push_back(backupFile);
    }
    mInstalledFiles[i]->SetInherited(true);
    target->mInstalledFiles.push_back(mInstalledFiles[i]);
  }
  mInstalledFiles.clear();

  //! @todo Should probably add inherited flag for symlinks and directories

  numSymLinks = mInstalledSymLinks.size();
  for (i = 0; i < numSymLinks; i++)
    target->AddInstalledSymLink(mInstalledSymLinks[i]);
  mInstalledSymLinks.clear();

  numDirectories = mInstalledDirectories.size();
  for (i = 0; i < numDirectories; i++)
    target->AddInstalledDirectory(mInstalledDirectories[i]);
  mInstalledDirectories.clear();

  if ( mInstallListFilename != "" )
    gCommandInterpreter->Interpret(REMOVE_STR + mInstallListFilename);

  return true;
}

/*! Removes duplicate and missing files from the file list.  Any backup files will be compared against the original file, and will be removed if they are the same or if the original file is missing (don't want backups of obsolete files). */
void InstallEntry::ConsolidateFiles()
{
  int i, j, numFiles, bestIndex;
  string filename;
  bool refFileFound, removeBackup;
  InstalledFile* tempFile;

  if ( mInstalledFiles.size() == 0 )
    LoadFileList();

  /* Purge duplicates */
  numFiles = mInstalledFiles.size();
  for (i = 0; i < numFiles; i++)
  {
    if ( !mInstalledFiles[i] )
      continue;

    filename = mInstalledFiles[i]->GetFilename();
    for (j = i + 1; j < numFiles; j++)
    {
      if ( !mInstalledFiles[j] )
        continue;

      if ( filename == mInstalledFiles[j]->GetFilename() )
      {
        if ( mInstalledFiles[i]->IsAnInheritedFile() )
        {
          delete mInstalledFiles[i];
          mInstalledFiles[i] = NULL;
        }
        else
        {
          delete mInstalledFiles[j];
          mInstalledFiles[j] = NULL;
        }
        break;
      }
    }
  }

  /* Remove missing files */
  for (i = 0; i < numFiles; i++)
  {
    if ( !mInstalledFiles[i] )
      continue;

    if ( !FileExists(mInstalledFiles[i]->GetFilename()) )
    {
      delete mInstalledFiles[i];
      mInstalledFiles[i] = NULL;
    }
  }

  /* Check backups, and remove if needed */
  for (i = 0; i < numFiles; i++)
  {
    if ( !mInstalledFiles[i] )
      continue;

    if ( !mInstalledFiles[i]->IsABackupFile() )
      continue;

    refFileFound = false;
    removeBackup = false;
    filename = mInstalledFiles[i]->GetFilename();
    for (j = 0; (j < numFiles) && !refFileFound; j++)
    {
      if ( !mInstalledFiles[j] || (j == i) )
        continue;

      if ( filename == mInstalledFiles[j]->GetBackupFilename() )
      {
        refFileFound = true;
        if ( mInstalledFiles[i]->GetChecksum() == mInstalledFiles[j]->GetChecksum() )
          removeBackup = true;
      }
    }
    if ( !refFileFound || removeBackup )
    {
      if ( unlink(filename.c_str()) == 0 )
      {
        delete mInstalledFiles[i];
        mInstalledFiles[i] = NULL;
      }
    }
  }

  /* Sort the list and remove the NULL entries */
  for (i = 0; i < numFiles; i++)
  {
    bestIndex = i;
    for (j = i + 1; j < numFiles; j++)
    {
      if ( mInstalledFiles[j] &&
           ( !mInstalledFiles[bestIndex] ||
             (*mInstalledFiles[j] < *mInstalledFiles[bestIndex]) ) )
        bestIndex = j;
    }
    if ( mInstalledFiles[bestIndex] )
    {
      tempFile = mInstalledFiles[i];
      mInstalledFiles[i] = mInstalledFiles[bestIndex];
      mInstalledFiles[bestIndex] = tempFile;
    }
    else
    {
      /* We couldn't find any remaining non-null files, so we are done */
      mInstalledFiles.erase(mInstalledFiles.begin() + i, mInstalledFiles.end());
      break;
    }
  }
}

/*! Deletes all non-backup inherited files from disk and removes them from the file list.
  @return true if all relevant files were successfully removed
*/
bool InstallEntry::PurgeLegacyFiles()
{
  int i, numFiles, numRemoved;
  bool retCode = true;

  if ( mInstalledFiles.size() == 0 )
    LoadFileList();

  numRemoved = 0;
  numFiles = mInstalledFiles.size();
  for (i = 0; i < numFiles; i++)
  {
    if ( mInstalledFiles[i]->IsAnInheritedFile() &&
         !mInstalledFiles[i]->IsABackupFile() )
    {
      if ( unlink(mInstalledFiles[i]->GetFilename().c_str()) == 0 )
      {
        delete mInstalledFiles[i];
        mInstalledFiles[i] = NULL;
        numRemoved++;
      }
      else
      {
        retCode = false;
        if ( numRemoved )
          mInstalledFiles[i - numRemoved] = mInstalledFiles[i];
      }
    }
    else if ( numRemoved )
      mInstalledFiles[i - numRemoved] = mInstalledFiles[i];
  }
  if ( numRemoved )
    mInstalledFiles.erase(mInstalledFiles.begin() + numFiles - numRemoved, mInstalledFiles.end());

  return retCode;
}

/*!
  @param instructions - the instructions for uninstalling the package

  Records the instructions for removing the package from the system.
*/
void InstallEntry::SetUninstallInstructions(const InstructionSequence& instructions)
{
  mUninstallInstructions = instructions;
}

/*!
  @param dependency - package dependency to be added to the list of this package's dependencies
*/
void InstallEntry::AddDependency(const PackageDependency& dependency)
{
  PackageDependency* newDependency;

  newDependency = new PackageDependency(dependency);
  if ( !newDependency )
    return;
  mDependencies.push_back(newDependency);
}

/*!
  Empties the list of dependencies associated with this package, and releases
  all reverse dependencies which point to this package.
*/
void InstallEntry::ClearDependencies(InstallLog& installLog)
{
  int i, numDependencies;

  numDependencies = mDependencies.size();
  for (i = 0; i < numDependencies; i++)
    delete mDependencies[i];
  mDependencies.clear();
  installLog.ReleaseReverseDependenciesTo(mPackageName,
                                          mPackageRevision.GetString());
}

/*!
  @return the number of dependencies associated with this package
*/
int InstallEntry::GetNumDependencies() const
{
  return mDependencies.size();
}

/*!
  @param depNumber - the index into the list of dependencies
  @return the dependency at the specified position in the list
  @return NULL if depNumber is invalid
*/
const PackageDependency* InstallEntry::GetDependency(int depNumber)
{
  if ( (depNumber < 0) || (depNumber >= (int)mDependencies.size()) )
    return NULL;
  return mDependencies[depNumber];
}

/*!
  @param packageName - name of the package which depends on this package
  @param revision - revision of the package with depends on this package
  @param forwardDependency - the dependency which the reverse dependency mirrors
*/
void InstallEntry::AddReverseDependency(string packageName, string revision, const PackageDependency& forwardDependency)
{
  if ( packageName == "" )
    return;

  PackageDependency *reverseDep = new PackageDependency(forwardDependency);

  if ( !reverseDep )
    return;

  reverseDep->SetDependentName(packageName);
  reverseDep->SetDependentRevision(revision);

  mReverseDependencies.push_back(reverseDep);
}

/*!
  @param packageName - name of the package which depends on this package
  @param revision - revision of the package with depends on this package
  @param maxEntries- the maximum number of matching reverse dependencies to remove

  This remove up to maxEntries number of reverse dependencies matching the specified name and revision.  There can be 2 matching reverse dependencies in the case where the revision has been installed and an attempt to reinstall has been at least configured.
*/
void InstallEntry::RemoveReverseDependency(string packageName, string revision, int maxEntries)
{
  int i, numDependencies;
  int numRemoved = 0;

  numDependencies = mReverseDependencies.size();
  for (i = 0; i < numDependencies; )
  {
    if ( (mReverseDependencies[i]->GetDependentName() == packageName) &&
         (mReverseDependencies[i]->GetDependentRevision() == revision) )
    {
      delete mReverseDependencies[i];
      mReverseDependencies.erase(mReverseDependencies.begin() + i);
      numDependencies--;

      numRemoved++;
      if ( (maxEntries > 0) && (numRemoved >= maxEntries) )
        return;
    }
    else
      i++;
  }
}

/*!
  @param target - the install entry to accept ownership of the reverse dependencies associated with this entry
  @return false if target is NULL
*/
bool InstallEntry::TransferReverseDependenciesTo(InstallEntry* target)
{
  int i, numDependencies;

  if ( !target )
    return false;

  numDependencies = mReverseDependencies.size();
  for (i = 0; i < numDependencies; i++)
    target->mReverseDependencies.push_back(mReverseDependencies[i]);
  mReverseDependencies.clear();

  return true;
}

/*!
  @return the number of reverse dependencies pertaining to this package
*/
int InstallEntry::GetNumReverseDependencies() const
{
  return mReverseDependencies.size();
}

/*!
  @param depNumber - the index into the list of reverse dependency names
  @return the name of the reverse dependency at the specified position in the list
  @return NULL if depNumber is invalid
*/
string InstallEntry::GetReverseDependencyName(int depNumber) const
{
  if ( (depNumber < 0) || (depNumber >= (int)mReverseDependencies.size()) )
    return "";
  return mReverseDependencies[depNumber]->GetDependentName();
}

/*!
  @param depNumber - the index into the list of reverse dependencies
  @return true if it is a static dependency
  @return false if it is not a static dependency or depNumber is invalid
*/
bool InstallEntry::ReverseDependencyIsStatic(int depNumber) const
{
  if ( (depNumber < 0) || (depNumber >= (int)mReverseDependencies.size()) )
    return false;
  return ( mReverseDependencies[depNumber]->GetType() == DEP_TYPE_STATIC );
}

/*!
  @param packages - list of [partially] installed packages which are possible rerse dependencies of this package
  @return false if any of this package's reverse dependencies are not contained in the list of packages
*/
bool InstallEntry::ReverseDependenciesAreInList(const InstallEntryVector& packages) const
{
  int i, j, numPackages, numDeps;
  bool matchFound;

  numPackages = packages.size();
  numDeps = mReverseDependencies.size();
  for (i = 0; i < numDeps; i++)
  {
    matchFound = false;
    for (j = 0; (j < numPackages) && !matchFound; j++)
    {
      if ( (packages[j]->GetName() == mReverseDependencies[i]->GetDependentName()) &&
           (packages[j]->GetRevision().GetString() == mReverseDependencies[i]->GetDependentRevision()) )
        matchFound = true;
    }
    if ( !matchFound )
      return false;
  }
  return true;
}

/*!
  @return list of the optional features and their states associated with this package
*/
const FeatureOptionVector& InstallEntry::GetFeatures() const
{
  return mFeatures;
}

/*!
  @param feature - optional feature to associate with this package
*/
void InstallEntry::AddFeature(FeatureOption* feature)
{
  if ( feature )
    mFeatures.push_back(feature);
}

/*!
  @param featureName - the name of the feature which is possibly used by this package
  @return true if the optional feature has been associated with this package
*/
bool InstallEntry::HasFeature(string featureName)
{
  int i, numFeatures;

  numFeatures = mFeatures.size();
  for (i = 0; i < numFeatures; i++)
    if ( mFeatures[i]->GetName() == featureName )
      return true;
  return false;
}

/*!
  @param featureName - the name of the feature which is possibly used by this package
  @return true if the optional feature has been associated with this package and has been activated
*/
bool InstallEntry::GetFeatureState(string featureName)
{
  int i, numFeatures;

  numFeatures = mFeatures.size();
  for (i = 0; i < numFeatures; i++)
    if ( mFeatures[i]->GetName() == featureName )
      return mFeatures[i]->IsEnabled();
  return false;
}

/*!
  @param featureName - the name of the feature which is possibly used by this package
  @return true if the optional feature has been associated

  Activates the feature in the list of features.  This does not affect the current installation, but whenever this package is upgraded or reinstalled, the feature will be used.
*/
bool InstallEntry::ActivateFeature(string featureName)
{
  int i, numFeatures;

  numFeatures = mFeatures.size();
  for (i = 0; i < numFeatures; i++)
    if ( mFeatures[i]->GetName() == featureName )
    {
      mFeatures[i]->Enable();
      return true;
    }

  return false;
}

/*!
  @param featureName - the name of the feature which is possibly used by this package
  @return true if the optional feature has been associated

  Deactivates the feature in the list of features.  This does not affect the current installation, but whenever this package is upgraded or reinstalled, the feature will no longer be used.
*/
bool InstallEntry::DeactivateFeature(string featureName)
{
  int i, numFeatures;

  numFeatures = mFeatures.size();
  for (i = 0; i < numFeatures; i++)
    if ( mFeatures[i]->GetName() == featureName )
    {
      mFeatures[i]->Disable();
      return true;
    }

  return false;
}

// Private---------------------------------------------------------------------

/*!
  @return true if the file list was successfully loaded from disk

  Loads the contents of the file described by mInstallListFilename into mInstalledFiles, mInstalledChecksums, and mInstalledDirectories.
*/
bool InstallEntry::LoadFileList()
{
  InstalledFile* file;
  xmlDocPtr document;
  xmlNodePtr currentNode;
  string location;

  if ( mInstallListFilename == "" )
    return false;

  document = xmlParseFile(mInstallListFilename.c_str());

  if ( !document )
    return false;

  currentNode = xmlDocGetRootElement(document);
  if ( !currentNode )
  {
    xmlFreeDoc(document);
    return false;
  }

  currentNode = currentNode->children;
  while ( currentNode )
  {
    if ( !strcmp((char *)currentNode->name, FILE_STR) )
    {
      file = new InstalledFile();
      if ( !file->Load(currentNode) )
        return false;
      mInstalledFiles.push_back(file);
    }
    else if ( !strcmp((char *)currentNode->name, SYM_LINK_STR) )
    {
      location = GetStringValue(currentNode, LOCATION_STR);
      if ( location == "" )
        return false;
      mInstalledSymLinks.push_back(location);
    }
    else if ( !strcmp((char *)currentNode->name, DIR_STR) )
    {
      location = GetStringValue(currentNode, LOCATION_STR);
      if ( location == "" )
        return false;
      mInstalledDirectories.push_back(location);
    }
    currentNode = currentNode->next;
  }

  xmlFreeDoc(document);
  return true;
}
