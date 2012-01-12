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
#ifndef APP_ROVER_H
#define APP_ROVER_H

#include <string>
#include <vector>
#include "ActionDescription.h"
#include "ConfigMgr.h"
#include "Repository.h"
#include "InstallLog.h"
#include "InstallEntry.h"
#include "Package.h"
#include "PackageDependency.h"

using std::string;
using std::vector;

//! Enum containing possible places for Install to stop
typedef enum
{
  STAGE_FETCH,
  STAGE_UNPACK,
  STAGE_CONFIGURE,
  STAGE_BUILD,
  STAGE_INSTALL
} AppRoverHaltStageType;

//! The main workhorse of the AppRover library
class AppRover
{
public:
  AppRover(string cfgFilename);
  ~AppRover();
  string Update(bool pretend, ActionDescriptionVector& actionList);
  string ExcludeFromUpdates(string packageName, bool state);
  string Purify(string packageName);
  string Install(string packageName, string revisionStr, bool pretend, ActionDescriptionVector& actionList, AppRoverHaltStageType haltStage=STAGE_INSTALL, bool allowReinstall=false, bool explicitInstall=true);
  string Uninstall(string packageName, bool pretend, ActionDescriptionVector& actionList);
  string Clean(bool pretend, ActionDescriptionVector& actionList, bool keepPartials=false);
  bool AddNewRepository(string baseLocation);
  bool AddNewPackage(Package* newPackage, int repositoryNumber=0);
  void UpdatePackageChecksums(Package* package);
  Package* GetPackage(string packageName, string revision, int repositoryNumber=0);
  Package* GetPrecedingPackage(string packageName, string revision, int repositoryNumber=0);
  Package* GetPackage(string packageName, int repositoryNumber=0);
  int NumPackages();
  string GetPackageName(int packageIndex);
  string GetInstalledRevisionNumber(int packageIndex);
  vector<string> ActivateFeature(string featureName, bool activationState);
  void ActivateFeatureForPackage(string packageName, string featureName, bool activationState);
  string SynchronizeRepositories();
private:
  Package* FindMostCurrentPackage(string packageName, const PackageDependency *dependency=NULL);
  Package* FindPackage(string packageName, string revisionStr);
  bool FetchPackage(const Package* package, InstallEntry* logEntry, bool force = false);
  bool UnpackPackage(Package* package, InstallEntry* logEntry);
  bool ConfigurePackage(Package* package, InstallEntry* logEntry);
  bool BuildPackage(Package* package, InstallEntry* logEntry);
  bool InstallPackage(Package* package, InstallEntry* logEntry);
  void PopulatePackageList();
  //! the configuration manager for the system
  ConfigMgr mConfig;
  //! the package repositories that are present in this system
  RepositoryVector mRepositories;
  //! the log of all applications that have been [partially] installed
  InstallLog mInstallLog;
  //! the list of names of packages available from all of the repositories
  vector<string> mPackageList;
  //! the list of revisions of packages available from all of the repositories
  vector<string> mRevisionList;
  //! indiciates whether the name and revision lists need to be updated
  bool mPackageListIsInvalid;
};

#endif
