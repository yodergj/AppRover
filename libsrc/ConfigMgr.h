///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004-2008 Gabriel Yoder
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
#ifndef CONFIG_MGR_H
#define CONFIG_MGR_H

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

//! Enum specifying the possible exit conditions of config file opertations
typedef enum
{
  CFG_FILE_NOT_SPECIFIED,
  CFG_FILE_MISSING,
  CFG_FILE_FORMAT_ERROR,
  CFG_FILE_NOT_WRITABLE,
  CFG_OK
} ConfigMgrStatusType;

#define FEATURE_ENABLED 1
#define FEATURE_DISABLED 0
#define FEATURE_USE_DEFAULT -1

//! Class responsible for managing all system specific AppRover configuration details
class ConfigMgr
{
public:
  ConfigMgr();
  ~ConfigMgr();
  ConfigMgrStatusType Load(string filename);
  ConfigMgrStatusType Save(string filename);
  ConfigMgrStatusType Save();
  string GetInstallLogFilename() const;
  int GetNumRepositories() const;
  string GetRepositoryDirectory(int repositoryNumber) const;
  bool AddRepositoryDirectory(string directory);
  string GetStorageDirectory() const;
  void SetStorageDirectory(string directory);
  string GetWorkDirectory() const;
  void SetWorkDirectory(string directory);
  string GetInstallFileDirectory() const;
  void SetInstallFileDirectory(string directory);
  int GetNumProcessors();
  int GetFeatureState(string featureName);
  void SetFeatureState(string featureName, int state);
private:
  void DetectNumProcessors();

  //! List of the location of all repositories in this system
  vector<string> mRepositoryDirectories;
  //! Name of the file containing the install log
  string mInstallLogFilename;
  //! The directory where are fetched files are stored
  string mStorageDirectory;
  //! The top level directory where all file operations are performed prior to installing a package
  string mWorkDirectory;
  //! The directory containing the lists of files installed by packages
  string mInstallFileDirectory;
  //! The file where this configuration data is stored
  string mCfgFilename;
  //! The number processing cores in the system
  int mNumProcessors;
  //! The system wide default values for a set of optional features
  map<string,bool> mFeatureSettings;
};

#endif
