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
#include <libxml/tree.h>
#include <string.h>
#include "Repository.h"
#include "XMLUtils.h"
#include "DirectoryUtils.h"
#include "Labels.h"
#include "Interpreter.h"
#include "Fetcher.h"

// Public----------------------------------------------------------------------

Repository::Repository()
{
  mDirectory = ".";
  mSyncProtocol = "";
  mSyncLocation = "";
}

Repository::~Repository()
{
  int i, numPackages;

  numPackages = mPackages.size();
  for (i=0; i<numPackages; i++)
    delete mPackages[i];
}

/*!
  @return REPO_FILE_NOT_WRITABLE if the repository file cannot be written
  @return REPO_OK otherwise

  Saves the repository data to the repository config file in the previously specified directory.
*/
RepositoryStatusType Repository::Save() const
{
  int i;
  FILE *fp;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  int numPackages;
  string filename;

  filename = mDirectory + REPOSITORY_CONFIG_FILE_STR;
  fp = fopen(filename.c_str(), "w");
  if ( !fp )
    return REPO_FILE_NOT_WRITABLE;

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document,NULL,(const xmlChar *)REPOSITORY_HDR_STR,NULL);
  SetDoubleValue(rootNode, VERSION_STR, 1);
  SetStringValue(rootNode, SYNC_PROTOCOL_STR, mSyncProtocol);
  SetStringValue(rootNode, SYNC_LOCATION_STR, mSyncLocation);
  xmlDocSetRootElement(document,rootNode);

  numPackages = mPackages.size();
  for (i=0; i<numPackages; i++)
    mPackages[i]->Save(rootNode);

  xmlDocFormatDump(fp,document,1);
  fclose(fp);
  xmlFreeDoc(document);
  return REPO_OK;
}

/*!
  @param directory - the directory containing the repository config file
  @return REPO_DIR_NOT_SPECIFIED if directory is ""
  @return REPO_FILE_MISSING if the config file could not be read
  @return REPO_FILE_FORMAT_ERROR if there is a format error in the file
  @return REPO_OK otherwise

  Loads the repository data from the repository config file in the specified directory.
*/
RepositoryStatusType Repository::Load(string directory)
{
  xmlDocPtr document;
  xmlNodePtr currentNode;
  double version;
  string configFilename;
  PackageIndex* packageIndex;

  if ( directory == "" )
    return REPO_DIR_NOT_SPECIFIED;

  mDirectory = directory;

  configFilename = directory + REPOSITORY_CONFIG_FILE_STR;
  document = xmlParseFile(configFilename.c_str());

  if ( !document )
    return REPO_FILE_MISSING;

  currentNode = xmlDocGetRootElement(document);
  if ( !currentNode )
  {
    xmlFreeDoc(document);
    return REPO_FILE_FORMAT_ERROR;
  }

  version = GetDoubleValue(currentNode, VERSION_STR, 1.0);
  mSyncProtocol = GetStringValue(currentNode, SYNC_PROTOCOL_STR);
  mSyncLocation = GetStringValue(currentNode, SYNC_LOCATION_STR);

  currentNode = currentNode->children;
  while ( currentNode )
  {
    if ( !strcmp((char *)currentNode->name,PACKAGE_INDEX_STR) )
    {
      packageIndex = new PackageIndex();
      if ( !packageIndex->Load(currentNode) )
      {
        delete packageIndex;
        return REPO_FILE_FORMAT_ERROR;
      }
      mPackages.push_back(packageIndex);
      packageIndex->SetBaseLocation(mDirectory);
    }
    currentNode = currentNode->next;
  }

  xmlFreeDoc(document);
  return REPO_OK;
}

/*!
  @param packageName - the name of the package to retrieve
  @param revision - the revision of the package to retrieve
  @return package with the specified name and revision
  @return NULL if no package with the specified name and revision could be found
*/
Package* Repository::GetPackage(string packageName, string revision)
{
  int i, numPackages;

  numPackages = mPackages.size();
  for (i=0; i<numPackages; i++)
  {
    if ( (mPackages[i]->GetName() == packageName) )
      return mPackages[i]->GetPackage(revision);
  }
  return NULL;
}

/*!
  @param packageName - the name of the package to retrieve
  @param revision - the revision which comes after the desired revision
  @return package with the specified name with revision preceding the specified one
  @return NULL if no package with the specified name and revision constraint could be found
*/
Package* Repository::GetPrecedingPackage(string packageName, string revision)
{
  int i, numPackages;

  numPackages = mPackages.size();
  for (i=0; i<numPackages; i++)
  {
    if ( (mPackages[i]->GetName() == packageName) )
      return mPackages[i]->GetPrecedingPackage(revision);
  }
  return NULL;
}

/*!
  @param packageName - the name of the package to find
  @param dependency - the dependency requirements
  @return pointer to the package with the specified name and latest revision
  @return NULL if the packageName is not recognized or no revision is available.

  Finds the latest revision of the package with the specified name.
  If dependency is non-null, it locates the most current revision which
  statisfies the dependency.
*/
Package* Repository::FindMostCurrentPackage(string packageName,
                                            const PackageDependency *dependency)
{
  int i, numPackages;

  numPackages = mPackages.size();
  for (i=0; i<numPackages; i++)
  {
    if ( mPackages[i]->GetName() == packageName )
      return mPackages[i]->GetMostCurrentPackage(dependency);
  }
  return NULL;
}

/*!
  @return the directory containing the repository config file
*/
string Repository::GetDirectory() const
{
  return mDirectory;
}

/*!
  @param directory - the directory to hold the repository

  Sets the config directory for the repository.  If the directory does not exist, it will be created.
*/
void Repository::SetDirectory(string directory)
{
  mDirectory = directory;
  MakeDirectory(directory);
}

/*!
  @param package - the package to add to the repository
  @return false if package is NULL or package duplicates another package

  Adds the specified package to the repository.  If no package index exists for this package, one will be created.
*/
bool Repository::AddPackage(Package* package)
{
  string name, revision;
  string command;
  bool packageInserted = false;
  int i, numPackages;;

  if ( !package )
    return false;

  name = package->GetName();
  revision = package->GetRevision().GetString();
  numPackages = mPackages.size();
  for (i=0; i<numPackages; i++)
  {
    if ( name == mPackages[i]->GetName() )
    {
      if ( mPackages[i]->ContainsRevision(revision) )
        return false;
      mPackages[i]->AddPackage(package);
      packageInserted = true;
      break;
    }
  }
  if ( !packageInserted )
  {
    PackageIndex *pkgIndex = new PackageIndex();
    pkgIndex->AddPackage(package);
    pkgIndex->SetBaseLocation(mDirectory);
    mPackages.push_back(pkgIndex);
  }
  Save();
  return true;
}

/*!
  @return the number of distinct package names in the repository
*/
int Repository::NumPackages() const
{
  return mPackages.size();
}

/*!
  @param packageIndex the index to the list of package names

  @return the name of the package at the specified index
  @return "" if the index is invalid
*/
string Repository::GetPackageName(int packageIndex) const
{
  if ( packageIndex >= (int)mPackages.size() )
    return "";
  return mPackages[packageIndex]->GetName();
}

/*!
  @return string containing an error message (if any)

  Uses the protocol specified in mSyncProtocol to update this repository to match the contents of the location specified in mSyncLocation.  This does nothing if the repository is local, or either mSyncProtocol or mSyncLocation is unspecified.  Cvs and package protocols are supported.  Rsync support is currently not implemented.
*/
string Repository::Synchronize()
{
  if ( (mSyncProtocol == "") || (mSyncLocation == "") )
    return "";

  if ( mSyncProtocol == "cvs" )
  {
    if ( !gCommandInterpreter->Interpret("cd " + mDirectory) )
      return "Could not cd to " + mDirectory;
    if ( !gCommandInterpreter->Interpret("cvs " + mSyncLocation) )
      return "Failed on cvs " + mSyncLocation;
    return "";
  }
  if ( mSyncProtocol == "rsync" )
  {
    return "rsync support has not been implemented";
  //! @todo Do actual rsync synchronization
  }
  if ( mSyncProtocol == "package" )
  {
    Fetcher fetcher;
    string filename;
    if ( !fetcher.Fetch(mSyncLocation,mDirectory) )
      return "Failed to fetch " + mSyncLocation;
    if ( !gCommandInterpreter->Interpret("cd " + mDirectory) )
      return "Could not cd to " + mDirectory;
    filename = GetFilenamePart(mSyncLocation);
    if ( filename.find(".tgz") != string::npos )
    {
      if ( !gCommandInterpreter->Interpret("gzip -cd " + filename + " | tar xvf -") )
        return "Failed to unzip " + filename + " with gzip";
    }
    else
    {
      if ( !gCommandInterpreter->Interpret("bzip2 -cd " + filename + " | tar xvf -") )
        return "Failed to unzip " + filename + " with bzip2";
    }
    return "";
  }
  return "Unhandled synchronization of type " + mSyncProtocol;
}

/*!
  @param protocol - the protocol to use for syncronization to a remote repository
  @return true if the protocol is supported ("cvs", "rsync", or "package")
*/
bool Repository::SetSyncProtocol(string protocol)
{
  if ( (protocol != "cvs") && (protocol != "rsync") &&(protocol != "package") )
    return false;
  mSyncProtocol = protocol;
  return true;
}

/*!
  @param location - the remote location used to synchronize this repository ("" for local repository)
  @return true;
*/
bool Repository::SetSyncLocation(string location)
{
  //! @todo Start checking if location format matches protocol
  mSyncLocation = location;
  return true;
}

/*!
  @return true if this repository does not have a synchronization location
*/
bool Repository::IsLocal() const
{
  return (mSyncLocation == "");
}

// Private---------------------------------------------------------------------

