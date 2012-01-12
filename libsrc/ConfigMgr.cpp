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
#include "ConfigMgr.h"
#include "XMLUtils.h"
#include "DirectoryUtils.h"
#include "Labels.h"

// Public----------------------------------------------------------------------

ConfigMgr::ConfigMgr()
{
  mInstallLogFilename = "";
  mStorageDirectory = "";
  mWorkDirectory = "";
  mInstallFileDirectory = "";
  mCfgFilename = "";
  mNumProcessors = 0;
}

ConfigMgr::~ConfigMgr()
{
}

/*!
  @param filename - the name of the configuration file
  @return CFG_FILE_NOT_SPECIFIED if filename is ""
  @return CFG_FILE_NOT_WRITABLE if the file cannot be opened for writing
  @return CFG_OK if successful

  Saves the configuration of the ConfigMgr to file.
*/
ConfigMgrStatusType ConfigMgr::Save(string filename)
{
  int i;
  FILE *fp;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  xmlNodePtr repositoryNode;
  xmlNodePtr featureNode;
  int numRepositories;
  string appRoverDir = APP_ROVER_DIR;
  map<string,bool>::iterator feature;

  if ( filename == "" )
    return CFG_FILE_NOT_SPECIFIED;

  fp = fopen(filename.c_str(), "w");
  if ( !fp )
    return CFG_FILE_NOT_WRITABLE;

  mCfgFilename = filename;

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document,NULL,(const xmlChar *)CONFIG_HDR_STR,NULL);
  SetDoubleValue(rootNode, VERSION_STR, 1);
  xmlDocSetRootElement(document,rootNode);

  if ( mInstallLogFilename == "" )
    SetStringValue(rootNode, LOG_STR, appRoverDir+"AppRover.log");
  else
    SetStringValue(rootNode, LOG_STR, mInstallLogFilename);
  if ( mStorageDirectory == "" )
    SetStringValue(rootNode, STORAGE_STR, appRoverDir+"storage/");
  else
    SetStringValue(rootNode, STORAGE_STR, mStorageDirectory);
  if ( mWorkDirectory == "" )
    SetStringValue(rootNode, WORK_STR, appRoverDir + "work/");
  else
    SetStringValue(rootNode, WORK_STR, mWorkDirectory);
  if ( mInstallFileDirectory == "" )
    SetStringValue(rootNode, INSTALL_FILE_STR, appRoverDir + "files/");
  else
    SetStringValue(rootNode, INSTALL_FILE_STR, mInstallFileDirectory);

  if ( mNumProcessors < 1 )
    DetectNumProcessors();
  SetIntValue(rootNode, PROCESSOR_STR, mNumProcessors);

  numRepositories = mRepositoryDirectories.size();
  if ( !numRepositories )
  {
    repositoryNode = xmlNewNode(NULL,(const xmlChar*)REPOSITORY_STR);
    SetStringValue(repositoryNode, DIR_STR, appRoverDir+"repo/");
    xmlAddChild(rootNode,repositoryNode);
  }
  else
  {
    for (i = 0; i < numRepositories; i++)
    {
      repositoryNode = xmlNewNode(NULL,(const xmlChar*)REPOSITORY_STR);
      SetStringValue(repositoryNode, DIR_STR, mRepositoryDirectories[i]);
      xmlAddChild(rootNode,repositoryNode);
    }
  }

  for (feature=mFeatureSettings.begin(); feature!=mFeatureSettings.end(); feature++)
  {
    featureNode = xmlNewNode(NULL,(const xmlChar*)FEATURE_STR);
    SetStringValue(featureNode, FEATURE_NAME_STR, (*feature).first);
    SetBoolValue(featureNode, ENABLED_STR, (*feature).second);
    xmlAddChild(rootNode,featureNode);
  }

  xmlDocFormatDump(fp,document,1);
  fclose(fp);
  xmlFreeDoc(document);
  return CFG_OK;
}

/*!
  @return CFG_FILE_NOT_SPECIFIED if a filename has never been specified
  @return CFG_FILE_NOT_WRITABLE if the file cannot be opened for writing
  @return CFG_OK if successful

  Saves the configuration of the ConfigMgr to file which was last used for either saving or loading.
*/
ConfigMgrStatusType ConfigMgr::Save()
{
  return Save(mCfgFilename);
}

/*!
  @return CFG_FILE_NOT_SPECIFIED if a filename has never been specified
  @return CFG_FILE_MISSING if the file does not exist
  @return CFG_FILE_FORMAT_ERROR if there is a problem with the file data
  @return CFG_OK if successful

  Loads the configuration of the ConfigMgr from file.
*/
ConfigMgrStatusType ConfigMgr::Load(string filename)
{
  xmlDocPtr document;
  xmlNodePtr currentNode;
  string location;
  double version;
  string name;

  if ( filename == "" )
    return CFG_FILE_NOT_SPECIFIED;

  document = xmlParseFile(filename.c_str());

  if ( !document )
    return CFG_FILE_MISSING;

  currentNode = xmlDocGetRootElement(document);
  if ( !currentNode )
  {
    printf("no root node in config\n");
    xmlFreeDoc(document);
    return CFG_FILE_FORMAT_ERROR;
  }

  mRepositoryDirectories.clear();

  version = GetDoubleValue(currentNode, VERSION_STR, 1.0);

  mInstallLogFilename = GetStringValue(currentNode, LOG_STR);
  if ( mInstallLogFilename == "" )
    return CFG_FILE_FORMAT_ERROR;

  mStorageDirectory = GetStringValue(currentNode, STORAGE_STR);
  if ( mStorageDirectory == "" )
    return CFG_FILE_FORMAT_ERROR;

  mWorkDirectory = GetStringValue(currentNode, WORK_STR);
  if ( mWorkDirectory == "" )
    return CFG_FILE_FORMAT_ERROR;

  mInstallFileDirectory = GetStringValue(currentNode, INSTALL_FILE_STR);
  if ( mInstallFileDirectory == "" )
    return CFG_FILE_FORMAT_ERROR;

  mNumProcessors = GetIntValue(currentNode, PROCESSOR_STR, 0);
  if ( mNumProcessors < 1 )
    DetectNumProcessors();

  currentNode = currentNode->children;
  while ( currentNode )
  {
    if ( !strcmp((char *)currentNode->name,REPOSITORY_STR) )
    {
      location = GetStringValue(currentNode, DIR_STR);
      if ( location != "" )
	mRepositoryDirectories.push_back(location);
    }
    else if ( !strcmp((char *)currentNode->name,FEATURE_STR) )
    {
      name = GetStringValue(currentNode, FEATURE_NAME_STR);
      if ( name != "" )
        mFeatureSettings[name] = GetBoolValue(currentNode,ENABLED_STR,false);
    }
    currentNode = currentNode->next;
  }

  mCfgFilename = filename;

  xmlFreeDoc(document);
  return CFG_OK;
}

/*!
  @return the name of the file containing the installation log
*/
string ConfigMgr::GetInstallLogFilename() const
{
  return mInstallLogFilename;
}

/*!
  @return the number of repositories specified in the configuration
*/
int ConfigMgr::GetNumRepositories() const
{
  return mRepositoryDirectories.size();
}

/*!
  @param repositoryNumber - the index for the list of repositories
  @return  the full path of the directory containing the repository ("" if repositoryNumber is invalid)
*/
string ConfigMgr::GetRepositoryDirectory(int repositoryNumber) const
{
  if ( (unsigned int)repositoryNumber < mRepositoryDirectories.size() )
    return mRepositoryDirectories[repositoryNumber];
  return "";
}

/*!
  @param directory - the full path of the directory containing the repository
  @return false if directory is "" or already has been specified

  Adds add repository directory to the configuration.  This only records a location - nothing is actually done to the repository.
*/
bool ConfigMgr::AddRepositoryDirectory(string directory)
{
  int i, numRepositories;

  if ( directory == "" )
    return false;

  numRepositories = mRepositoryDirectories.size();
  for (i=0; i<numRepositories; i++)
    if ( mRepositoryDirectories[i] == directory )
      return false;
  mRepositoryDirectories.push_back(directory);
  return true;
}

/*!
  @return the directory used for storing fetched files
*/
string ConfigMgr::GetStorageDirectory() const
{
  return mStorageDirectory;
}

/*!
  @param directory - the full path of the storage directory

  Changes the location of the storage directory.  The old directory remains untouched, but is no longer used.  If the new directory does not exist, it is created.
*/
void ConfigMgr::SetStorageDirectory(string directory)
{
  mStorageDirectory = directory;
  MakeDirectory(directory);
}

/*!
  @return the full path to the directory which houses individual package working directories
*/
string ConfigMgr::GetWorkDirectory() const
{
  return mWorkDirectory;
}

/*!
  @param directory - the full path of the work directory

  Changes the location of the work directory.  The old directory remains untouched, but is no longer used.  If the new directory does not exist, it is created.
*/
void ConfigMgr::SetWorkDirectory(string directory)
{
  mWorkDirectory = directory;
  MakeDirectory(directory);
}

/*!
  @return the directory where lists of installed files are stored
*/
string ConfigMgr::GetInstallFileDirectory() const
{
  return mInstallFileDirectory;
}

/*!
  @param directory - name of the directory where lists of installed fiels should be stored
*/
void ConfigMgr::SetInstallFileDirectory(string directory)
{
  mInstallFileDirectory = directory;
  MakeDirectory(directory);
}

/*!
  @return number of processors in system (0 if unknown)
*/
int ConfigMgr::GetNumProcessors()
{
  return mNumProcessors;
}

/*!
  @param featureName - name of the optional feature whose activation status is desired
  @return FEATURE_ENABLED if the feature has been activated
  @return FEATURE_DISABLE if the feature has been deactivated
  @return FEATURE_USE_DEFAULT if a system default value has not been set (indicates that the package specific default should be used)
*/
int ConfigMgr::GetFeatureState(string featureName)
{
  map<string,bool>::iterator setting;
  
  setting = mFeatureSettings.find(featureName);
  if ( setting != mFeatureSettings.end() )
  {
    if ( (*setting).second )
      return FEATURE_ENABLED;
    return FEATURE_DISABLED;
  }
  return FEATURE_USE_DEFAULT;
}

/*!
  @param featureName - the name of optional feature whose system default should be set
  @param state - the system default state for the feature
*/
void ConfigMgr::SetFeatureState(string featureName, int state)
{
  if ( featureName == "" )
    return;

  if ( (state != FEATURE_ENABLED) && (state != FEATURE_DISABLED) )
  {
    map<string,bool>::iterator setting;
    setting = mFeatureSettings.find(featureName);
    if ( setting != mFeatureSettings.end() )
      mFeatureSettings.erase(setting);
  }
  else
    mFeatureSettings[featureName] = (state == FEATURE_ENABLED);
}

// Private---------------------------------------------------------------------

//! Determines the number of processors in the system and sets mNumProcessors accordingly.
void ConfigMgr::DetectNumProcessors()
{
  /* Do some voodoo to determine the number of processors under Linux.  It would be nice
     to have some cross-platform way to do this. */
  mNumProcessors = system("exit `grep '^processor' /proc/cpuinfo | wc -l`");
  mNumProcessors = WEXITSTATUS(mNumProcessors);
  if ( mNumProcessors < 1 )
    mNumProcessors = 1;
}
