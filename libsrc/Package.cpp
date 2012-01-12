///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004-2012 Gabriel Yoder
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
#include "Package.h"
#include "Fetcher.h"
#include "XMLUtils.h"
#include "DirectoryUtils.h"
#include "Interpreter.h"
#include "Labels.h"

// Public----------------------------------------------------------------------

Package::Package()
{
  mPackageName = "";
  mPackageSlot = "";
  mBinaryCompatabilityRevision = 0;
  mFilename = "";
}

/*!
  @param refPackage - package used as a base
  @param revision - the revision for this package

  Creates a new package which is an updated copy of refPackage.
  The new package is the same as refPackage except that all
  references to refPackage's revision is changed to the new
  packages revision.
*/
Package::Package(const Package* refPackage, string revision)
{
  int i;
  int revisionPos, numDeps, numFeatures, numSourceFiles;
  string refRevision, fileLocation;
  SourceFile* sourceFile;
  PackageDependency* refDep;
  string minRevision, maxRevision;

  if ( !refPackage || (refPackage->mPackageRevision == revision) )
  {
    mPackageName = "";
    mPackageSlot = "";
    mBinaryCompatabilityRevision = 0;
    mFilename = "";
    return;
  }
  refRevision = refPackage->mPackageRevision.GetString();

  mFilename = refPackage->mFilename;
  revisionPos = mFilename.find(refRevision);
  if ( revisionPos < 0 )
    mFilename += ".clone";
  else
    mFilename.replace(revisionPos,refRevision.length(),revision);

  numDeps = refPackage->mDependencies.size();
  for (i = 0; i < numDeps; i++)
  {
    refDep = refPackage->mDependencies[i];
    minRevision = refDep->GetMinRevision();
    maxRevision = refDep->GetMaxRevision();
    if ( refRevision == minRevision )
      minRevision = revision;
    if ( refRevision == maxRevision )
      maxRevision = revision;
    mDependencies.push_back(new PackageDependency(*refDep, minRevision, maxRevision));
  }

  numFeatures = refPackage->mFeatures.size();
  for (i = 0; i < numFeatures; i++)
    mFeatures.push_back(new FeatureOption(*(refPackage->mFeatures[i])));

  mPackageName = refPackage->mPackageName;
  mPackageRevision.Set(revision);
  mPackageSlot = refPackage->mPackageSlot;
  mBinaryCompatabilityRevision = refPackage->mBinaryCompatabilityRevision;

  numSourceFiles = refPackage->mSourceFiles.size();
  for (i = 0; i < numSourceFiles; i++)
  {
    if ( refPackage->mSourceFiles[i]->IsEmbedded() )
      sourceFile = new SourceFile(refPackage->mSourceFiles[i]);
    else
    {
      fileLocation = refPackage->mSourceFiles[i]->GetLocation();
      revisionPos = fileLocation.find(refRevision);
      while ( revisionPos != (int)string::npos )
      {
        fileLocation.replace(revisionPos,refRevision.length(),revision);
        revisionPos = fileLocation.find(refRevision,revisionPos+revision.length());
      }
      sourceFile = new SourceFile(fileLocation);
      sourceFile->SetCondition(refPackage->mSourceFiles[i]->GetCondition());
    }
    mSourceFiles.push_back(sourceFile);
  }

  /* skipping the current unused mInstalledDirectores, and mInstalledFiles */

  mUnpackInstructions = refPackage->mUnpackInstructions;
  mUnpackInstructions.ReplaceString(refRevision,revision,true);
  mConfigureInstructions = refPackage->mConfigureInstructions;
  mConfigureInstructions.ReplaceString(refRevision,revision,true);
  mBuildInstructions = refPackage->mBuildInstructions;
  mBuildInstructions.ReplaceString(refRevision,revision,true);
  mInstallInstructions = refPackage->mInstallInstructions;
  mInstallInstructions.ReplaceString(refRevision,revision,true);
  mUninstallInstructions = refPackage->mUninstallInstructions;
  mUninstallInstructions.ReplaceString(refRevision,revision,true);
}

Package::~Package()
{
  int i,numDependencies,numFeatures,numFiles;

  numFiles = mSourceFiles.size();
  for (i=0; i<numFiles; i++)
    delete mSourceFiles[i];
  numDependencies = mDependencies.size();
  for (i=0; i<numDependencies; i++)
    delete mDependencies[i];
  numFeatures = mFeatures.size();
  for (i=0; i<numFeatures; i++)
    delete mFeatures[i];
}

/*!
  @param filename - the file to receive the package data
  @return false if filename is "" or is not writable

  Saves the package data to the specified file.
*/
bool Package::Save(string filename)
{
  int i;
  FILE *fp;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  xmlNodePtr fileNode;
  int numFiles, numDirectories, numDependencies, numFeatures;

  if ( filename == "" )
    return false;

  mFilename = filename;

  fp = fopen(filename.c_str(), "w");
  if ( !fp )
    return false;

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document,NULL,(const xmlChar *)PACKAGE_HDR_STR,NULL);
  SetDoubleValue(rootNode, VERSION_STR, 1);
  xmlDocSetRootElement(document,rootNode);

  SetStringValue(rootNode, PACKAGE_NAME_STR, mPackageName);
  SetStringValue(rootNode, REVISION_STR, mPackageRevision.GetString());
  SetStringValue(rootNode, SLOT_STR, mPackageSlot);
  SetIntValue(rootNode, BINARY_COMPATABILITY_REVISION_STR, mBinaryCompatabilityRevision);

  numDependencies = mDependencies.size();
  for (i=0; i<numDependencies; i++)
    mDependencies[i]->Save(rootNode);

  numFeatures = mFeatures.size();
  for (i=0; i<numFeatures; i++)
    mFeatures[i]->Save(rootNode);

  numFiles = mSourceFiles.size();
  for (i=0; i<numFiles; i++)
    mSourceFiles[i]->Save(rootNode);

  numFiles = mInstalledFiles.size();
  for (i=0; i<numFiles; i++)
  {
    fileNode = xmlNewNode(NULL,(const xmlChar*)FILE_STR);
    SetStringValue(fileNode, LOCATION_STR, mInstalledFiles[i]);
    xmlAddChild(rootNode,fileNode);
  }

  numDirectories = mInstalledDirectories.size();
  for (i=0; i<numDirectories; i++)
  {
    fileNode = xmlNewNode(NULL,(const xmlChar*)DIR_STR);
    SetStringValue(fileNode, LOCATION_STR, mInstalledDirectories[i]);
    xmlAddChild(rootNode,fileNode);
  }

  mUnpackInstructions.Save(rootNode,UNPACK_STR);
  mConfigureInstructions.Save(rootNode,CONFIGURE_STR);
  mBuildInstructions.Save(rootNode,BUILD_STR);
  mInstallInstructions.Save(rootNode,INSTALL_STR);
  mUninstallInstructions.Save(rootNode,UNINSTALL_STR);

  xmlDocFormatDump(fp,document,1);
  fclose(fp);
  xmlFreeDoc(document);
  return true;
}

/*!
  @return true if this package was successfully saved to the file specified by mFilename
*/
bool Package::Save()
{
  return Save(mFilename);
}

/*!
  @param filename - the file containing the package data
  @return false if filename is "", the file doesn't exists, or there is a format error in the file

  Loads the package data from the specified file.
*/
bool Package::Load(string filename)
{
  xmlDocPtr document;
  xmlNodePtr currentNode;
  double version;
  PackageDependency *dependency;
  FeatureOption *feature;
  SourceFile* file;
  string location;
  string revision;
  string instructionType;

  if ( filename == "" )
    return false;

  mFilename = filename;
  document = xmlParseFile(filename.c_str());

  if ( !document )
    return false;

  currentNode = xmlDocGetRootElement(document);
  if ( !currentNode )
  {
    xmlFreeDoc(document);
    return false;
  }

  version = GetDoubleValue(currentNode, VERSION_STR, 1.0);

  mPackageName = GetStringValue(currentNode, PACKAGE_NAME_STR);
  if ( mPackageName == "" )
    return false;

  revision = GetStringValue(currentNode, REVISION_STR);
  if ( revision == "" )
    return false;
  mPackageRevision.Set(revision);

  mPackageSlot = GetStringValue(currentNode, SLOT_STR);
  mBinaryCompatabilityRevision = GetIntValue(currentNode, BINARY_COMPATABILITY_REVISION_STR, 0);

  currentNode = currentNode->children;
  while ( currentNode )
  {
    if ( !strcmp((char *)currentNode->name,DEPENDENCY_STR) )
    {
      dependency = new PackageDependency();
      if ( !dependency->Load(currentNode) )
      {
        delete dependency;
        return false;
      }
      mDependencies.push_back(dependency);
    }
    else if ( !strcmp((char *)currentNode->name,FEATURE_STR) )
    {
      feature = new FeatureOption();
      if ( !feature->Load(currentNode) )
      {
        delete feature;
        return false;
      }
      mFeatures.push_back(feature);
    }
    else if ( !strcmp((char *)currentNode->name,SOURCE_FILE_STR) )
    {
      file = new SourceFile();
      if ( !file->Load(currentNode) )
      {
        delete file;
        return false;
      }
      mSourceFiles.push_back(file);
    }
    else if ( !strcmp((char *)currentNode->name,FILE_STR) )
    {
      location = GetStringValue(currentNode, LOCATION_STR);
      if ( location == "" )
        return false;
      mInstalledFiles.push_back(location);
    }
    else if ( !strcmp((char *)currentNode->name,DIR_STR) )
    {
      location = GetStringValue(currentNode, LOCATION_STR);
      if ( location == "" )
        return false;
      mInstalledDirectories.push_back(location);
    }
    else if ( !strcmp((char *)currentNode->name,INSTRUCTION_STR) )
    {
      instructionType = GetStringValue(currentNode,INSTRUCTION_TYPE_STR);
      if ( !strcmp(instructionType.c_str(),UNPACK_STR) )
      {
        if ( !mUnpackInstructions.Load(currentNode) )
          return false;
      }
      else if ( !strcmp(instructionType.c_str(),CONFIGURE_STR) )
      {
        if ( !mConfigureInstructions.Load(currentNode) )
          return false;
      }
      else if ( !strcmp(instructionType.c_str(),BUILD_STR) )
      {
        if ( !mBuildInstructions.Load(currentNode) )
          return false;
      }
      else if ( !strcmp(instructionType.c_str(),INSTALL_STR) )
      {
        if ( !mInstallInstructions.Load(currentNode) )
          return false;
      }
      else if ( !strcmp(instructionType.c_str(),UNINSTALL_STR) )
      {
        if ( !mUninstallInstructions.Load(currentNode) )
          return false;
      }
    }
    currentNode = currentNode->next;
  }

  xmlFreeDoc(document);
  return true;
}

/*!
  @param refPackage - the package to compare against
  @return true if this package has a higher revision number than refPackage

  Compares the revision numbers of this package with refPackage and returns true if this package has a higher revision number.
*/
bool Package::IsNewerThan(const Package* refPackage) const
{
  if ( !refPackage )
    return true;

  if ( mPackageRevision > refPackage->mPackageRevision )
    return true;

  return false;
}

/*!
  @return the name of this package
*/
string Package::GetName() const
{
  return mPackageName;
}

/*!
  @param packageName - the new name of this package

  Sets the name of this package to the specified string.
*/
void Package::SetName(string packageName)
{
  mPackageName = packageName;
}

/*!
  @return the revision of this package
*/
const Revision& Package::GetRevision() const
{
  return mPackageRevision;
}

/*!
  @param revision - the new revision of this package

  Sets the revision of this package to the specified value.
*/
void Package::SetRevision(string revision)
{
  mPackageRevision.Set(revision);
}

/*!
  @return the slot this package occupies
*/
string Package::GetSlot() const
{
  return mPackageSlot;
}

/*!
  @param slot - the slot this package occupies
*/
void Package::SetSlot(string slot)
{
  mPackageSlot = slot;
}

/*!
  @return the value used to indicate if 2 packages have binary compatable interfaces
*/
int Package::GetBinaryCompatabilityRevision() const
{
  return mBinaryCompatabilityRevision;
}

/*!
  @param revision - the value used to indicate if 2 packages have binary compatable interfaces
*/
void Package::SetBinaryCompatabilityRevision(int revision)
{
  mBinaryCompatabilityRevision = revision;
}

/*!
  @param instruction - the new instruction

  Adds an instruction to the end of the list of unpacking instructions.
*/
void Package::AddUnpackInstruction(string instruction)
{
  mUnpackInstructions.AddInstruction(instruction);
}

/*!
  @param file - the file to receive the unpacking instructions
  @return true if the unpacking instructions were successfully saved to the file
*/
bool Package::DumpUnpackInstructionsToFile(FILE* file) const
{
  return mUnpackInstructions.DumpToFile(file);
}

/*!
  Deletes all of the unpacking instructions.
*/
void Package::ClearUnpackInstructions()
{
  mUnpackInstructions.Clear();
}

/*!
  @param instruction - the new instruction

  Adds an instruction to the end of the list of configuration instructions.
*/
void Package::AddConfigureInstruction(string instruction)
{
  mConfigureInstructions.AddInstruction(instruction);
}

/*!
  @param file - the file to receive the configuration instructions
  @return true if the configuration instructions were successfully saved to the file
*/
bool Package::DumpConfigureInstructionsToFile(FILE* file) const
{
  return mConfigureInstructions.DumpToFile(file);
}

/*!
  Deletes all of the configuration instructions.
*/
void Package::ClearConfigureInstructions()
{
  mConfigureInstructions.Clear();
}

/*!
  @param instruction - the new instruction

  Adds an instruction to the end of the list of build instructions.
*/
void Package::AddBuildInstruction(string instruction)
{
  mBuildInstructions.AddInstruction(instruction);
}

/*!
  @param file - the file to receive the build instructions
  @return true if the build instructions were successfully saved to the file
*/
bool Package::DumpBuildInstructionsToFile(FILE* file) const
{
  return mBuildInstructions.DumpToFile(file);
}

/*!
  Deletes all of the build instructions.
*/
void Package::ClearBuildInstructions()
{
  mBuildInstructions.Clear();
}

/*!
  @param instruction - the new instruction

  Adds an instruction to the end of the list of install instructions.
*/
void Package::AddInstallInstruction(string instruction)
{
  mInstallInstructions.AddInstruction(instruction);
}

/*!
  @param file - the file to receive the install instructions
  @return true if the install instructions were successfully saved to the file
*/
bool Package::DumpInstallInstructionsToFile(FILE* file) const
{
  return mInstallInstructions.DumpToFile(file);
}

/*!
  Deletes all of the install instructions.
*/
void Package::ClearInstallInstructions()
{
  mInstallInstructions.Clear();
}

/*!
  instruction - the new instruction

  Adds an instruction to the end of the list of uninstall instructions.
*/
void Package::AddUninstallInstruction(string instruction)
{
  mUninstallInstructions.AddInstruction(instruction);
}

/*!
  @param file - the file to receive the uninstall instructions
  @return true if the uninstall instructions were successfully saved to the file
*/
bool Package::DumpUninstallInstructionsToFile(FILE* file) const
{
  return mUninstallInstructions.DumpToFile(file);
}

/*!
  Deletes all of the uninstall instructions.
*/
void Package::ClearUninstallInstructions()
{
  mUninstallInstructions.Clear();
}

/*!
  @param file - the source file to add
  @return false if file is NULL

  Adds file to the end of the list of source files.  "Source" indicates a file to be fetched and does not necessarily indicate that it contains source code.
*/
bool Package::AddSourceFile(SourceFile* file)
{
  if ( !file )
    return false;

  mSourceFiles.push_back(file);
  return true;
}

/*!
  @return the number of files required to install this package
*/
int Package::GetNumSourceFiles() const
{
  return mSourceFiles.size();
}

/*!
  @param fileNumber - index into the list of source files
  @return the source file at the specifed position in the list
  @return NULL if fileNumber is invalid
*/
SourceFile* Package::GetSourceFile(int fileNumber)
{
  if ( (fileNumber >= (int)mSourceFiles.size()) || (fileNumber < 0) )
    return NULL;
  return mSourceFiles[fileNumber];
}

/*!
  @param fileNumber - index into the list of source files
  @return false if fileNumber is invalid
*/
bool Package::RemoveSourceFile(int fileNumber)
{
  if ( (fileNumber >= (int)mSourceFiles.size()) || (fileNumber < 0) )
    return false;

  delete mSourceFiles[fileNumber];
  mSourceFiles.erase(mSourceFiles.begin()+fileNumber);
  return true;
}

/*!
  Empties the list of source files.
*/
void Package::ClearSourceFiles()
{
  int i, numFiles;

  numFiles = mSourceFiles.size();
  for (i=0; i<numFiles; i++)
    delete mSourceFiles[i];
  mSourceFiles.clear();
}

/*!
  @param storageDirectory - location where local copies of source files are stored
  @return true if checksums were successfully calculated for all source files
*/
bool Package::RecordSourceChecksums(string storageDirectory)
{
  Fetcher fileFetcher;
  int i, numFiles;
  string checksum;

  numFiles = mSourceFiles.size();
  for (i = 0; i < numFiles; i++)
  {
    if ( mSourceFiles[i]->IsEmbedded() )
      continue;

    if ( !fileFetcher.Fetch(mSourceFiles[i]->GetLocation(),storageDirectory) )
      return false;
    checksum = fileFetcher.GetChecksum(mSourceFiles[i]->GetLocation(),storageDirectory);
    if ( checksum == "" )
      return false;
    mSourceFiles[i]->SetCheckSum(checksum);
  }
  return true;
}

/*!
  @param dependency - the dependency to add

  Adds a dependency to the list of dependencies.
*/
void Package::AddDependency(PackageDependency* dependency)
{
  if ( dependency )
    mDependencies.push_back(dependency);
}

/*!
  @param packageName - the name of package to be removed from the list of dependencies
  @return false if packageName is "" or is not a dependency of this package
*/
bool Package::RemoveDependency(string packageName)
{
  int i, numDependencies;

  if ( packageName == "" )
    return false;

  numDependencies = mDependencies.size();
  for (i=0; i<numDependencies; i++)
  {
    if ( packageName == mDependencies[i]->GetName() )
    {
      delete mDependencies[i];
      mDependencies.erase(mDependencies.begin()+i);
      return true;
    }
  }
  return false;
}

/*!
  Empties the list of dependencies files.
*/
void Package::ClearDependencies()
{
  int i;
  for (i=0; i<(int)mDependencies.size(); i++)
    delete mDependencies[i];
  mDependencies.clear();
}

/*!
  @return the number of prerequisite packages that must be satisfied before this package can be installed
*/
int Package::GetNumDependencies() const
{
  return mDependencies.size();
}

/*!
  @param dependencyNumber - the index to the dependency list
  @return NULL if the index is invalid
  @return Otherwise, a pointer to the dependency object
*/
const PackageDependency* Package::GetDependency(int dependencyNumber) const
{
  if ( (unsigned int)dependencyNumber > mDependencies.size() )
    return NULL;
  return mDependencies[dependencyNumber];
}

/*!
  @param feature - optional feature to be added to the list
*/
void Package::AddFeature(FeatureOption* feature)
{
  if ( feature )
    mFeatures.push_back(feature);
}

/*!
  @param featureName - name of the optional feature to be removed from the list
  @return false if featureName is "" or does not match any features in the list
*/
bool Package::RemoveFeature(string featureName)
{
  int i, numFeatures;

  if ( featureName == "" )
    return false;

  numFeatures = mFeatures.size();
  for (i=0; i<numFeatures; i++)
  {
    if ( mFeatures[i]->GetName() == featureName )
    {
      delete mFeatures[i];
      mFeatures.erase(mFeatures.begin()+i);
      return true;
    }
  }
  return false;
}

/*!
  Removes all entries from the list of optional features
*/
void Package::ClearFeatures()
{
  int i;
  for (i=0; i<(int)mFeatures.size(); i++)
    delete mFeatures[i];
  mFeatures.clear();
}

/*!
  @return the number of optional features recognized by this package
*/
int Package::GetNumFeatures() const
{
  return mFeatures.size();
}

/*!
  @param featureNumber - the index into the list of optional features
  @return the feature at the specified location in the list
  @return NULL if featureNumber is invalid
*/
FeatureOption* Package::GetFeature(int featureNumber) const
{
  if ( (featureNumber < 0) || (featureNumber >= (int)mFeatures.size()) )
    return NULL;
  return mFeatures[featureNumber];
}

/*!
  @param logEntry - the install log entry corresponding to this package
  @param storageDirectory - the directory where source files are stored
  @param force (=false) - if true, fetch all files even if local copies exist
  @return false if logEntry is NULL or a file cannot be retrieved

  This retrieves all files needed in order to install the package.
*/
bool Package::Fetch(InstallEntry* logEntry, string storageDirectory, bool force) const
{
  Fetcher fileFetcher;
  int i, numFiles;

  if ( !logEntry )
    return false;

  numFiles = mSourceFiles.size();
  if ( numFiles > 0 )
    gShowProcessingStage("Fetching " + mPackageName + " " + mPackageRevision.GetString());

  for (i = 0; i < numFiles; i++)
  {
    if ( !mSourceFiles[i]->IsActive(logEntry->GetFeatures()) ||
         mSourceFiles[i]->IsEmbedded() )
      continue;
    if ( !fileFetcher.Fetch(mSourceFiles[i]->GetLocation(), storageDirectory, mSourceFiles[i]->GetCheckSum(), force) )
    {
      logEntry->SetErrorMessage("Failed to fetch " + mSourceFiles[i]->GetLocation());
      return false;
    }
  }
  logEntry->SetErrorMessage("");
  return true;
}

/*!
  @param logEntry - the install log entry corresponding to this package
  @param storageDirectory - the directory where source files are stored
  @param workDirectory - the parent directory of all package working directories
  @param numProcessors - number of processors in system (for parallelization)
  @return false if logEntry is NULL or an error occurs while unpacking

  Unpacks the package by creating a working directory and copying all source files to the working directory.  If any unpacking instructions have been specified, they will then be executed from the working directory.
*/
bool Package::Unpack(InstallEntry* logEntry, string storageDirectory, string workDirectory, int numProcessors)
{
  string unpackDirectory;
  int i, numFiles;

  if ( !logEntry )
    return false;

  gShowProcessingStage("Unpacking " + mPackageName + " " + mPackageRevision.GetString());
  unpackDirectory = workDirectory + mPackageName + "-" + mPackageRevision.GetString() + SLASH_STR;
  if ( !MakeDirectory(unpackDirectory) )
  {
    logEntry->SetErrorMessage("Failed to create working directory " + unpackDirectory);
    return false;
  }
  if ( !gCommandInterpreter->Interpret("cd " + unpackDirectory) )
  {
    logEntry->SetErrorMessage("Failed to enter working directory " + unpackDirectory);
    return false;
  }
  numFiles = mSourceFiles.size();
  for (i = 0; i < numFiles; i++)
  {
    if ( !mSourceFiles[i]->IsActive(logEntry->GetFeatures()) )
      continue;
    if ( mSourceFiles[i]->IsEmbedded() )
    {
      if ( !mSourceFiles[i]->ExtractContents(unpackDirectory) )
      {
        logEntry->SetErrorMessage("Failed to extract " + GetFilenamePart(mSourceFiles[i]->GetLocation()));
        return false;
      }
    }
    else
    {
      if ( !gCommandInterpreter->Interpret("cp " + storageDirectory + GetFilenamePart(mSourceFiles[i]->GetLocation()) + " " + unpackDirectory) )
      {
        logEntry->SetErrorMessage("Failed to copy " + GetFilenamePart(mSourceFiles[i]->GetLocation()));
        return false;
      }
    }
  }
  mUnpackInstructions.SetStartingDirectory(unpackDirectory);
  if ( !mUnpackInstructions.Execute(logEntry, numProcessors, false) )
    return false;
  logEntry->SetUnpacked();
  logEntry->SetErrorMessage("");
  return true;
}

/*!
  @param logEntry - the install log entry corresponding to this package
  @param workDirectory - the parent directory of all package working directories
  @param numProcessors - number of processors in system (for parallelization)
  @return false if logEntry is NULL or an error occurs while configuring

  Configures the package by executing all configure instructions from the working directory.
*/
bool Package::Configure(InstallEntry* logEntry, string workDirectory, int numProcessors)
{
  string unpackDirectory;

  if ( !logEntry )
    return false;

  gShowProcessingStage("Configuring " + mPackageName + " " + mPackageRevision.GetString());
  unpackDirectory = workDirectory + mPackageName + "-" + mPackageRevision.GetString() + SLASH_STR;
  if ( !gCommandInterpreter->Interpret("cd " + unpackDirectory) )
  {
    logEntry->SetErrorMessage("Failed to enter working directory " + unpackDirectory);
    return false;
  }
  mConfigureInstructions.SetStartingDirectory(unpackDirectory);
  if ( !mConfigureInstructions.Execute(logEntry, numProcessors, false) )
    return false;
  logEntry->SetConfigured();
  logEntry->SetErrorMessage("");
  return true;
}

/*!
  @param logEntry - the install log entry corresponding to this package
  @param workDirectory - the parent directory of all package working directories
  @param numProcessors - number of processors in system (for parallelization)
  @return false if logEntry is NULL or an error occurs while building

  Builds the package by executing all build instructions from the working directory.
*/
bool Package::Build(InstallEntry* logEntry, string workDirectory, int numProcessors)
{
  string unpackDirectory;

  if ( !logEntry )
    return false;

  gShowProcessingStage("Building " + mPackageName + " " + mPackageRevision.GetString());
  unpackDirectory = workDirectory + mPackageName + "-" + mPackageRevision.GetString() + SLASH_STR;
  if ( !gCommandInterpreter->Interpret("cd " + unpackDirectory) )
  {
    logEntry->SetErrorMessage("Failed to enter working directory " + unpackDirectory);
    return false;
  }
  mBuildInstructions.SetStartingDirectory(unpackDirectory);
  if ( !mBuildInstructions.Execute(logEntry, numProcessors, false) )
    return false;
  logEntry->SetBuilt();
  logEntry->SetErrorMessage("");
  return true;
}

/*!
  @param logEntry - the install log entry corresponding to this package
  @param workDirectory - the parent directory of all package working directories
  @param numProcessors - number of processors in system (for parallelization)
  @return false if logEntry is NULL or an error occurs while installing

  Installs the package by executing all install instructions from the working directory.
*/
bool Package::Install(InstallEntry* logEntry, string workDirectory, int numProcessors)
{
  string unpackDirectory, logFilename;
  string appRoverDir = APP_ROVER_DIR;
  FILE* logFile;
  string line, command, name;
  bool eof;
  int tokenPos;

  if ( !logEntry )
    return false;

  gShowProcessingStage("Installing " + mPackageName + " " + mPackageRevision.GetString());
  unpackDirectory = workDirectory + mPackageName + "-" + mPackageRevision.GetString() + SLASH_STR;
  if ( !gCommandInterpreter->Interpret("cd " + unpackDirectory) )
  {
    logEntry->SetErrorMessage("Failed to enter working directory " + unpackDirectory);
    return false;
  }
  mInstallInstructions.SetStartingDirectory(unpackDirectory);
  if ( !mInstallInstructions.Execute(logEntry, numProcessors, true) )
    return false;
  logEntry->SetUninstallInstructions(mUninstallInstructions);

  logFilename = mInstallInstructions.GetStartingDirectory()+"/.AppRoverFileLog";
  logFile = fopen(logFilename.c_str(), "r");
  if ( logFile )
  {
    line = Readline(logFile, eof);
    while ( !eof )
    {
      tokenPos = line.find(' ');
      command = line.substr(0, tokenPos);
      name = line.substr(tokenPos + 1);

      if ( command == "DIR" )
      {
        name = GetCleanedPath(name);
        if ( name.find(workDirectory) != 0 )
          logEntry->AddInstalledDirectory(name);
      }
      else if ( command == "FILE" )
      {
        name = GetCleanedPath(name);
        if ( (name.find("/dev") != 0) && (name.find(workDirectory) != 0) &&
             (name.find("/etc/ld.so") != 0) )
          logEntry->AddInstalledFile(name);
      }
      else if ( (command == "LINK") || (command == "RENAME") )
      {
        name = GetCleanedPath(name);
        if ( name.find(workDirectory) != 0 )
        {
          if ( IsADirectory(name) )
            logEntry->AddInstalledDirectory(name);
          else if ( (name.find("/dev") != 0) && (name.find(workDirectory) != 0) &&
                    (name.find("/etc/ld.so") != 0) )
            logEntry->AddInstalledFile(name);
        }
      }
      else if ( command == "SYMLINK" )
      {
        name = GetCleanedPath(name);
        if ( name.find(workDirectory) != 0 )
          logEntry->AddInstalledSymLink(name);
      }

      line = Readline(logFile,eof);
    }
    fclose(logFile);
  }
  logEntry->ConsolidateFiles();

  logEntry->SetInstalled();
  logEntry->SetErrorMessage("");
  gCommandInterpreter->Interpret("cd " + workDirectory);
  gCommandInterpreter->Interpret("rm -r " + unpackDirectory);
  return true;
}

// Private---------------------------------------------------------------------
