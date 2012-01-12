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
#include "InstallLog.h"
#include "XMLUtils.h"
#include "Labels.h"
#include "Interpreter.h"
#include <string.h>

// Public----------------------------------------------------------------------

InstallLog::InstallLog()
{
}

InstallLog::~InstallLog()
{
  int i, size;

  size = mInstallEntries.size();
  for (i=0; i<size; i++)
    delete mInstallEntries[i];
}

/*!
  @param filename - the name of the file to hold the installation log
  @return LOG_FILE_NOT_SPECIFIED if filename is ""
  @return LOG_FILE_NOT_WRITABLE if the file could not be opened for writing
  @return LOG_OK if successful

  Saves the installation log into the specified file.
*/
InstallLogStatusType InstallLog::Save(string filename)
{
  int i;
  FILE *fp;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  int numEntries;

  if ( filename == "" )
    return LOG_FILE_NOT_SPECIFIED;

  fp = fopen(filename.c_str(), "w");
  if ( !fp )
    return LOG_FILE_NOT_WRITABLE;

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document,NULL,(const xmlChar *)LOG_HDR_STR,NULL);
  SetDoubleValue(rootNode, VERSION_STR, 1);
  xmlDocSetRootElement(document,rootNode);

  numEntries = mInstallEntries.size();
  for (i=0; i<numEntries; i++)
    mInstallEntries[i]->Save(rootNode);

  xmlDocFormatDump(fp,document,1);
  fclose(fp);
  xmlFreeDoc(document);
  return LOG_OK;
}

/*!
  @param filename - the name of the file containing the installation log
  @return LOG_FILE_NOT_SPECIFIED if filename is ""
  @return LOG_FILE_MISSING if the file does not exist
  @return LOG_FILE_FORMAT_ERROR if there is a problem with the data
  @return LOG_OK if successful

  Loads the installation log from the specified file.
*/
InstallLogStatusType InstallLog::Load(string filename)
{
  xmlDocPtr document;
  xmlNodePtr currentNode;
  InstallEntry *entry;
  double version;

  if ( filename == "" )
    return LOG_FILE_NOT_SPECIFIED;

  document = xmlParseFile(filename.c_str());

  if ( !document )
    return LOG_FILE_MISSING;

  currentNode = xmlDocGetRootElement(document);
  if ( !currentNode )
  {
    xmlFreeDoc(document);
    return LOG_FILE_FORMAT_ERROR;
  }

  version = GetDoubleValue(currentNode,VERSION_STR,1.0);

  currentNode = currentNode->children;
  while ( currentNode )
  {
    if ( !strcmp((char *)currentNode->name,ENTRY_STR) )
    {
      entry = new InstallEntry();
      if ( entry->Load(currentNode) )
	mInstallEntries.push_back(entry);
      else
	delete entry;
    }
    currentNode = currentNode->next;
  }

  xmlFreeDoc(document);
  return LOG_OK;
}

/*!
  @param dependency - the dependency that need to be resolved
  @return true if a package has been installed that satisfies the dependency

  Determines whether the dependency has been satisfied by the installed packages.
*/
bool InstallLog::Satisfies(const PackageDependency* dependency) const
{
  int i, numEntries;
  //! @todo Need to add support here for block packages

  numEntries = mInstallEntries.size();
  for (i=0; i<numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == dependency->GetName()) &&
         mInstallEntries[i]->IsInstalled() &&
         dependency->IsSatisfiedBy(mInstallEntries[i]) )
      return true;
  }
  return false;
}

/*!
  @param newEntry - the entry to add to the log
  @return false if the entry is NULL

  Adds an entry to the list of entries.
*/
bool InstallLog::AddEntry(InstallEntry *newEntry)
{
  if ( !newEntry )
    return false;
  mInstallEntries.push_back(newEntry);
  return true;
}

/*!
  @param packageName - name of the package recorded in the entry
  @param packageRevision - revision of the package recorded in the entry
  @return the entry contaiing the specified package and revision
  @return NULL if no entry records a package with the specified name and revision
*/
InstallEntry* InstallLog::FindEntry(string packageName, string packageRevision)
{
  int i, numEntries;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == packageName) &&
         (mInstallEntries[i]->GetRevision() == packageRevision) )
      return mInstallEntries[i];
  }
  return NULL;
}

/*!
  @param packageName - name of the package
  @param results - (output) the installed entries of the specified package

  Finds all installed versions of the package and stores the entries in results.
*/
void InstallLog::GetInstalledEntries(string packageName, vector<InstallEntry*>& results)
{
  int i, numEntries;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == packageName) &&
         mInstallEntries[i]->IsInstalled() )
      results.push_back(mInstallEntries[i]);
  }
}

/*!
  @param packageName - name of the package recorded in the entry
  @param packageRevision - revision of the package recorded in the entry
  @return the entry contaiing the specified package and revision
  @return NULL if no entry records a package with the specified name and revision
*/
InstallEntry* InstallLog::FindPartiallyInstalledEntry(string packageName, string packageRevision)
{
  int i, numEntries;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == packageName) &&
         (mInstallEntries[i]->GetRevision() == packageRevision) &&
         !mInstallEntries[i]->IsInstalled() )
      return mInstallEntries[i];
  }
  return NULL;
}

/*!
  @param packageName - name of the package recorded in the entry
  @return the entry contaiing the specified package and a revision older than the revision of any other entry for the specified package
  @return NULL if no entry records a package with the specified name
*/
InstallEntry* InstallLog::FindOldestInstalledEntry(string packageName)
{
  int i, numEntries;
  InstallEntry* oldestEntry = NULL;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == packageName) &&
         mInstallEntries[i]->IsInstalled() &&
         ( !oldestEntry ||
           (mInstallEntries[i]->GetRevision() < oldestEntry->GetRevision()) ) )
      oldestEntry = mInstallEntries[i];
  }
  return oldestEntry;
}

/*!
  @param entry - the install entry used as a reference for find the next oldest installed entry
  @return the oldest installed entry which is newer than the reference entry
  @return NULL if entry is NULL or is the newest installed entry
*/
InstallEntry* InstallLog::FindNextOldestInstalledEntry(InstallEntry* entry)
{
  int i, numEntries;
  InstallEntry* oldestEntry = NULL;

  if ( !entry )
    return NULL;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == entry->GetName()) &&
         mInstallEntries[i]->IsInstalled() &&
         (mInstallEntries[i]->GetRevision() > entry->GetRevision()) &&
         ( !oldestEntry ||
           (mInstallEntries[i]->GetRevision() < oldestEntry->GetRevision()) ) )
      oldestEntry = mInstallEntries[i];
  }
  return oldestEntry;
}

/*!
  @param packageName - name of the package
  @param slot - the slot which the requested package occupies
  @return the installed entry with the given name that occupies the specified slot
  @return NULL if the package has not been installed or does not occupy the specifed slot
*/
InstallEntry* InstallLog::FindInstalledSlotOccupant(string packageName, string slot)
{
  int i, numEntries;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == packageName) &&
         (mInstallEntries[i]->GetSlot() == slot) &&
         mInstallEntries[i]->IsInstalled() )
      return mInstallEntries[i];
  }
  return NULL;
}

/*!
  @param packageName - name of the package recorded in the entry
  @param packageRevision - revision of the package recorded in the entry
  @return false if no entry records a package with the specified name and revision

  Deletes an entry containing a package with the specified name and revision.  Removes all reverse dependencies which link to this entry.
*/
bool InstallLog::DeleteEntry(string packageName, string packageRevision)
{
  int i, numEntries;
  bool entryRemoved = false;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == packageName) &&
         (mInstallEntries[i]->GetRevision() == packageRevision) )
    {
      delete mInstallEntries[i];
      mInstallEntries.erase(mInstallEntries.begin()+i);
      entryRemoved=true;
      numEntries = mInstallEntries.size();
      i--;
    }
    else
      mInstallEntries[i]->RemoveReverseDependency(packageName,packageRevision,-1);
  }
  return entryRemoved;
}

/*!
  @param entry - the entry to remove from the log
  @param removeRevDeps - should reverse dependencies that point to this package be removed
  @return false if the entry was not found in the log

  Removes the specifed entry from the log and removes the package from reverse dependencies of all other installed packages.
*/
bool InstallLog::DeleteEntry(InstallEntry* entry, bool removeRevDeps)
{
  int i, numEntries;
  bool entryRemoved = false;
  bool entryHasDeps = false;
  string packageName, packageRevision;

  if ( entry->GetNumDependencies() > 0 )
    entryHasDeps = true;
  packageName = entry->GetName();
  packageRevision = entry->GetRevision().GetString();

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( mInstallEntries[i] == entry )
    {
      delete mInstallEntries[i];
      mInstallEntries.erase(mInstallEntries.begin()+i);
      entryRemoved=true;
      numEntries = mInstallEntries.size();
      i--;
    }
    else if ( removeRevDeps && entryHasDeps )
    {
      mInstallEntries[i]->RemoveReverseDependency(packageName,packageRevision);
    }
  }
  return entryRemoved;
}

/*!
  @param packageName - name of the package recorded in the entry
  @return "" if the package has not been installed in the system
  @return Otherwise, the revision string of the specified installed package
*/
string InstallLog::GetRevisionInstalled(string packageName)
{
  int i, numEntries;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == packageName) &&
         mInstallEntries[i]->IsInstalled() )
      return mInstallEntries[i]->GetRevision().GetString();
  }
  return "";
}

/*!
  @param entry - the package occupying the slot to be cleaned
  @param workDirectory - the top level directory where builds are done
  @return false if entry is NULL

  Examines the package slot for the specified entry and clears out any other entries which occupy the same slot and are in a partially installed state.
*/
bool InstallLog::RemovePartiallyInstalledSlotMates(InstallEntry *entry, string workDirectory)
{
  int i, numEntries;
  string packageWorkDirectory;

  if ( !entry )
    return false;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; )
  {
    if ( (mInstallEntries[i]->GetName() == entry->GetName()) &&
         (mInstallEntries[i]->GetSlot() == entry->GetSlot()) &&
         !mInstallEntries[i]->IsInstalled() && (mInstallEntries[i] != entry) )
    {
      packageWorkDirectory = workDirectory + mInstallEntries[i]->GetName() + "-" + mInstallEntries[i]->GetRevision().GetString();
      gCommandInterpreter->Interpret("rm -r " + packageWorkDirectory);
      DeleteEntry(mInstallEntries[i]);
      numEntries--;
    }
    else
      i++;
  }
  return true;
}

/*!
  @param packageName - the name of the package with the specified dependency
  @param revision - the revision of the package with the specified dependency
  @param dependency - package dependency used to determine the recipient of the reverse dependency

  Uses the dependency to locate a satisfiying package and associates the specified package name and revision as a reverse dependency.
*/
void InstallLog::RegisterReverseDependency(string packageName, string revision, const PackageDependency* dependency)
{
  int i, numEntries;
  string depType;

  if ( !dependency )
    return;

  depType = dependency->GetType();

  //! Install dependencies are moot after package install, so no reverse dependencies are generated for them.
  if ( depType == "install" )
    return;

  //! @todo Need to find best entry which satifies the dependency
  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( (mInstallEntries[i]->GetName() == dependency->GetName()) &&
         dependency->IsSatisfiedBy(mInstallEntries[i]) )
    {
      mInstallEntries[i]->AddReverseDependency(packageName,revision,*dependency);
      return;
    }
  }
}

/*!
  @param packageName - name of the target package
  @param packageRevision - revision of the target package

  Deletes reverse dependencies which point to a package with the specified name
  and revision.  If a single entry has more than one applicable reverse
  dependency, only one of them is removed.
*/
void InstallLog::ReleaseReverseDependenciesTo(string packageName,
                                              string packageRevision)
{
  int i, numEntries;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    mInstallEntries[i]->RemoveReverseDependency(packageName,packageRevision,-1);
  }
}

/*!
  @param featureName - the name of the optional feature to locate in the entries
  @return list of packages can use the optional feature
*/
InstallEntryVector InstallLog::GetPackagesUsingFeature(string featureName)
{
  int i, numEntries;
  InstallEntryVector packages;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( mInstallEntries[i]->HasFeature(featureName) )
      packages.push_back(mInstallEntries[i]);
  }
  return packages;
}

/*!
  @return list of all packages which have only been partially installed
*/
InstallEntryVector InstallLog::GetPartiallyInstalledPackages()
{
  int i, numEntries;
  InstallEntryVector packages;

  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
  {
    if ( !mInstallEntries[i]->IsInstalled() )
      packages.push_back(mInstallEntries[i]);
  }
  return packages;
}

/*!
  @return list of all packages which were not explicitly installed and are not dependencies of explicitly installed packages
*/
InstallEntryVector InstallLog::GetUnneededPackages()
{
  int i, numEntries;
  int numReverseDeps;
  bool listUpdated = true;
  vector<bool> packageAddedToList;
  InstallEntryVector packages;
  //! @todo Add an input list of things to ignore

  // We start with a list (potentially empty) of packages which we do not
  // want to ignore in reverse dependencies.  We then add to the list, packages
  // which have no reverse dependencies or only reverse dependencies that are
  // in the list.  We keep looping through the entries until we have a pass
  // where no more entries are added to the list.
  numEntries = mInstallEntries.size();
  for (i = 0; i < numEntries; i++)
    packageAddedToList.push_back(false);
  while ( listUpdated )
  {
    listUpdated = false;
    for (i = 0; i < numEntries; i++)
    {
      if ( !packageAddedToList[i] &&
           mInstallEntries[i]->IsInstalled() &&
           !mInstallEntries[i]->IsExplicitlyInstalled() )
      {
        numReverseDeps = mInstallEntries[i]->GetNumReverseDependencies();
        if ( !numReverseDeps ||
            mInstallEntries[i]->ReverseDependenciesAreInList(packages) )
        {
          packageAddedToList[i] = true;
          packages.push_back(mInstallEntries[i]);
          listUpdated = true;
        }
      }
    }
  }
  return packages;
}

// Private---------------------------------------------------------------------
