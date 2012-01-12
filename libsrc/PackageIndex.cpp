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
#include "PackageIndex.h"
#include "DirectoryUtils.h"
#include "XMLUtils.h"
#include "Labels.h"
#include <string.h>

// Public----------------------------------------------------------------------

PackageIndex::PackageIndex()
{
  mPackageName = "";
  mBaseLocation = "";
}

PackageIndex::~PackageIndex()
{
  int i, numPackages;

  numPackages = mLoadedPackages.size();
  for (i=0; i<numPackages; i++)
    delete mLoadedPackages[i];
}

/*!
  @param parentNode - the parent XML DOM node for the package index node
  @return false if parentNode is NULL

  Saves the package index data as a child node of the specified parent.
*/
bool PackageIndex::Save(xmlNodePtr parentNode) const
{
  xmlNodePtr indexNode;
  xmlNodePtr revisionNode;
  int i, numRevisions, numLoadedPackages;
  string directory;

  if ( !parentNode )
    return false;

  indexNode = xmlNewNode(NULL,(const xmlChar*)PACKAGE_INDEX_STR);

  SetStringValue(indexNode, PACKAGE_NAME_STR, mPackageName);
  numRevisions = mPackageRevisions.size();
  for (i=0; i<numRevisions; i++)
  {
    revisionNode = xmlNewNode(NULL,(const xmlChar*)REVISION_STR);
    SetStringValue(revisionNode, FILE_STR, mPackageFilenames[i]);
    SetStringValue(revisionNode, REVISION_STR, mPackageRevisions[i].GetString());
    xmlAddChild(indexNode,revisionNode);
  }

  numLoadedPackages = mLoadedPackages.size();
  directory = mBaseLocation + mPackageName + SLASH_STR;
  for (i=0; i<numLoadedPackages; i++)
    mLoadedPackages[i]->Save(directory + mLoadedPackages[i]->GetName() + '-' + mLoadedPackages[i]->GetRevision().GetString() + PACKAGE_FILE_EXTENSION_STR);

  xmlAddChild(parentNode,indexNode);

  return true;
}

/*!
  @param packageIndexNode - the XML DOM node containing the package index data
  @return false if packageIndexNode is NULL or there is a format error

  Loads the package index data from the specified node.
*/
bool PackageIndex::Load(xmlNodePtr packageIndexNode)
{
  xmlNodePtr currentNode;
  string location, revisionStr;
  Revision revision;

  if ( !packageIndexNode )
    return false;

  mPackageName = GetStringValue(packageIndexNode, PACKAGE_NAME_STR);
  if ( mPackageName == "" )
    return false;

  currentNode = packageIndexNode->children;
  while ( currentNode )
  {
    if ( !strcmp((char *)currentNode->name,REVISION_STR) )
    {
      location = GetStringValue(currentNode, FILE_STR);
      if ( location == "" )
        return false;
      revisionStr = GetStringValue(currentNode, REVISION_STR);
      if ( revisionStr == "" )
        return false;
      revision.Set(revisionStr);
      mPackageRevisions.push_back(revision);
      mPackageFilenames.push_back(location);
    }
    currentNode = currentNode->next;
  }
  return true;
}

/*!
  @param revision - the revision of the package to retrieve
  @return the package with the specified revision
  @return NULL if the revision is unknown or the package could not be loaded
*/
Package* PackageIndex::GetPackage(string revision)
{
  int i, numRevisions, numLoaded, packageNumber;
  Package* package = NULL;

  packageNumber = -1;
  numRevisions = mPackageRevisions.size();

  if ( !numRevisions )
    return NULL;

  numLoaded = mLoadedPackages.size();
  for (i=0; i<numLoaded; i++)
  {
    if (mLoadedPackages[i]->GetRevision().GetString() == revision)
      return mLoadedPackages[i];
  }

  for (i=0; i<numRevisions; i++)
  {
    if ( mPackageRevisions[i].GetString() == revision )
    {
      packageNumber = i;
      break;
    }
  }

  if ( packageNumber == -1 )
    return NULL;

  package = new Package();
  if ( !package->Load(mBaseLocation + mPackageName + SLASH_STR + mPackageFilenames[packageNumber]) )
  {
    delete package;
    return NULL;
  }
  mLoadedPackages.push_back(package);
  return package;
}

/*!
  @param dependency - the dependency requirements  
  @return a pointer to the package with the latest revision number
  @return NULL if there are not entries in the index, or the package cannot be loaded.

  If dependency is non-null, it locates the most current revision which
  statisfies the dependency.  
*/
Package* PackageIndex::GetMostCurrentPackage(const PackageDependency *dependency)
{
  int i, numRevisions, numLoaded, newestRevision;
  Package* mostCurrentPackage = NULL;

  newestRevision = 0;
  numRevisions = mPackageRevisions.size();

  if ( !numRevisions )
    return NULL;

  if ( dependency )
  {
    newestRevision = -1;
    for (i=0; i<numRevisions; i++)
      if ( ( (newestRevision < 0) ||
             (mPackageRevisions[i] > mPackageRevisions[newestRevision]) ) &&
           dependency->IsSatisfiedBy(mPackageName, mPackageRevisions[i]) )
        newestRevision = i;

    if ( newestRevision == -1 )
      return NULL;
  }
  else
  {
    for (i=1; i<numRevisions; i++)
      if ( mPackageRevisions[i] > mPackageRevisions[newestRevision] )
        newestRevision = i;
  }

  numLoaded = mLoadedPackages.size();
  for (i=0; i<numLoaded; i++)
  {
    if (mLoadedPackages[i]->GetRevision() == mPackageRevisions[newestRevision])
      return mLoadedPackages[i];
  }

  mostCurrentPackage = new Package();
  if ( !mostCurrentPackage->Load(mBaseLocation + mPackageName + SLASH_STR + mPackageFilenames[newestRevision]) )
  {
    delete mostCurrentPackage;
    return NULL;
  }
  mLoadedPackages.push_back(mostCurrentPackage);
  return mostCurrentPackage;
}

/*!
  @param revision - a revision following the desired revision
  @return a pointer to the package with the latest revision number preceding the specified one
  @return NULL if there are not entries in the index, or the package cannot be loaded.
*/
Package* PackageIndex::GetPrecedingPackage(string revision)
{
  int i, numRevisions, numLoaded, newestRevision;
  Package* mostCurrentPackage = NULL;
  Revision refRevision;

  refRevision.Set(revision);
  numRevisions = mPackageRevisions.size();

  if ( !numRevisions )
    return NULL;

  newestRevision = -1;
  for (i=0; i<numRevisions; i++)
    if ( ( (newestRevision < 0) ||
           (mPackageRevisions[i] > mPackageRevisions[newestRevision]) ) &&
         (mPackageRevisions[i] < refRevision) )
      newestRevision = i;

  if ( newestRevision == -1 )
    return NULL;

  numLoaded = mLoadedPackages.size();
  for (i=0; i<numLoaded; i++)
  {
    if (mLoadedPackages[i]->GetRevision() == mPackageRevisions[newestRevision])
      return mLoadedPackages[i];
  }

  mostCurrentPackage = new Package();
  if ( !mostCurrentPackage->Load(mBaseLocation + mPackageName + SLASH_STR + mPackageFilenames[newestRevision]) )
  {
    delete mostCurrentPackage;
    return NULL;
  }
  mLoadedPackages.push_back(mostCurrentPackage);
  return mostCurrentPackage;
}

/*!
  @param baseLocation - the parent to the package directory

  This sets the directory which will be used as a parent to the directory where the packages are stored.  The package directory will be created if it does not exist.
*/
void PackageIndex::SetBaseLocation(string baseLocation)
{
  mBaseLocation = baseLocation;
  MakeDirectory(mBaseLocation + mPackageName);
}

/*!
  @param package - a package to add to this index
  @return false if 1) package is NULL
                   2) the package is missing its name or revision
                   3) the index contains a package with the same revision
                      as the revision of package
                   4) the package has the wrong name

  This adds a package to the list of packages handled by this index.  If this is the first package, it is used to set the name of the package index.
*/
bool PackageIndex::AddPackage(Package* package)
{
  string packageName;
  Revision packageRevision;
  string packageFilename;

  if ( !package )
    return false;

  packageName = package->GetName();
  packageRevision = package->GetRevision();
  if ( (packageName == "") || (packageRevision.GetString() == "") )
    return false;

  if ( ContainsRevision(packageRevision.GetString()) )
    return false;

  if ( (mPackageName != "") && (mPackageName != packageName) )
    return false;

  if ( mPackageName == "" )
    mPackageName = packageName;

  mLoadedPackages.push_back(package);
  packageFilename = packageName+"-"+packageRevision.GetString()+PACKAGE_FILE_EXTENSION_STR;
  mPackageRevisions.push_back(packageRevision);
  mPackageFilenames.push_back(packageFilename);
  return true;
}

/*!
  @return the name of the package managed by this index
*/
string PackageIndex::GetName() const
{
  return mPackageName;
}

/*!
  @param revision - the revision potentially handled by this index
  @return - true if the specified revision is contained in one of the packages handled by this index
*/
bool PackageIndex::ContainsRevision(string revision) const
{
  int i, numRevisions;

  numRevisions = mPackageRevisions.size();
  for (i=0; i<numRevisions; i++)
    if ( mPackageRevisions[i].GetString() == revision )
      return true;
  return false;
}

// Private---------------------------------------------------------------------
