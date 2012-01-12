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
#ifndef INSTALL_LOG_H
#define INSTALL_LOG_H

#include <string>
#include <vector>
#include "InstallEntry.h"
#include "PackageDependency.h"

using std::string;
using std::vector;

//! Enum specifying the possible exit conditions of log file operations
typedef enum
{
  LOG_FILE_NOT_SPECIFIED,
  LOG_FILE_MISSING,
  LOG_FILE_FORMAT_ERROR,
  LOG_FILE_NOT_WRITABLE,
  LOG_OK
} InstallLogStatusType;

//! Class for managing the entries of all packages which have been at least partially installed
class InstallLog
{
public:
  InstallLog();
  ~InstallLog();
  InstallLogStatusType Save(string filename);
  InstallLogStatusType Load(string filename);
  bool Satisfies(const PackageDependency* dependency) const;
  bool AddEntry(InstallEntry *newEntry);
  InstallEntry* FindEntry(string packageName, string packageRevision);
  void GetInstalledEntries(string packageName, vector<InstallEntry*>& results);
  InstallEntry* FindPartiallyInstalledEntry(string packageName, string packageRevision);
  InstallEntry* FindOldestInstalledEntry(string packageName);
  InstallEntry* FindNextOldestInstalledEntry(InstallEntry* entry);
  InstallEntry* FindInstalledSlotOccupant(string packageName, string slot);
  bool DeleteEntry(string packageName, string packageRevision);
  bool DeleteEntry(InstallEntry *entry, bool removeRevDeps=true);
  string GetRevisionInstalled(string packageName);
  bool RemovePartiallyInstalledSlotMates(InstallEntry *entry, string workDirectory);
  void RegisterReverseDependency(string packageName, string revision, const PackageDependency* dependency);
  void ReleaseReverseDependenciesTo(string packageName, string revision);
  InstallEntryVector GetPackagesUsingFeature(string featureName);
  InstallEntryVector GetPartiallyInstalledPackages();
  InstallEntryVector GetUnneededPackages();
private:
  //! List of all [partially] installed packages in the system
  InstallEntryVector mInstallEntries;
};

#endif
