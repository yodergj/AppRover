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
#ifndef PACKAGE_INDEX_H
#define PACKAGE_INDEX_H

#include <string>
#include <vector>
#include <libxml/tree.h>
#include "Package.h"
#include "Revision.h"

using std::string;
using std::vector;

//! A storage class for tracking and maintaining package revisions
class PackageIndex
{
public:
  PackageIndex();
  ~PackageIndex();
  bool Save(xmlNodePtr parentNode) const;
  bool Load(xmlNodePtr packageIndexNode);
  Package* GetPackage(string revision);
  Package* GetMostCurrentPackage(const PackageDependency *dependency=NULL);
  Package* GetPrecedingPackage(string revision);
  void SetBaseLocation(string baseLocation);
  bool AddPackage(Package* package);
  string GetName() const;
  bool ContainsRevision(string revision) const;
private:
  //! The name of the application pertaining to the list of packages
  string mPackageName;
  //! The directory containing the list file for this index
  string mBaseLocation;
  //! The list of revisions available for this application
  vector<Revision> mPackageRevisions;
  //! The list of files containing package data for this application
  vector<string> mPackageFilenames;
  //! The list of packages whose info has been already loaded from disk
  PackageVector mLoadedPackages;
};

typedef vector<PackageIndex *> PackageIndexVector;

#endif
