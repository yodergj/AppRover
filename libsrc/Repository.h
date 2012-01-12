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
#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <libxml/tree.h>
#include <vector>
#include <string>
#include "Package.h"
#include "PackageIndex.h"

//! Type for returning repository load & save errors
typedef enum
{
  REPO_DIR_NOT_SPECIFIED,
  REPO_FILE_MISSING,
  REPO_FILE_FORMAT_ERROR,
  REPO_FILE_NOT_WRITABLE,
  REPO_OK
} RepositoryStatusType;

using std::vector;
using std::string;

//! Container class for handling packages stored in a particular location
class Repository
{
public:
  Repository();
  ~Repository();
  RepositoryStatusType Save() const;
  RepositoryStatusType Load(string direcotry);
  Package* FindMostCurrentPackage(string packageName, const PackageDependency *dependency=NULL);
  Package* GetPackage(string packageName, string revision);
  Package* GetPrecedingPackage(string packageName, string revision);
  string GetDirectory() const;
  void SetDirectory(string directory);
  bool AddPackage(Package* package);
  int NumPackages() const;
  string GetPackageName(int packageIndex) const;
  string Synchronize();
  bool SetSyncProtocol(string protocol);
  bool SetSyncLocation(string location);
  bool IsLocal() const;
private:
  //! The name of the protocol used to synchronize with remotely
  string mSyncProtocol;
  //! Remote location to which this repository should be synchronized
  string mSyncLocation;
  //! Directory where the local copy of the repository is stored
  string mDirectory;
  //! List of packages available in this repository
  PackageIndexVector mPackages;
};

typedef vector<Repository *> RepositoryVector;

#endif
