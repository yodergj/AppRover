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
#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <vector>
#include "FeatureOption.h"
#include "PackageDependency.h"
#include "InstructionSequence.h"
#include "InstallEntry.h"
#include "Revision.h"
#include "SourceFile.h"

using std::string;
using std::vector;

//! Class for handling all relevant data pertaining to a particular revision of an application
class Package
{
public:
  Package();
  Package(const Package* refPackage, string revision);
  ~Package();
  bool Save();
  bool Save(string filename);
  bool Load(string filename);
  bool IsNewerThan(const Package* refPackage) const;
  string GetName() const;
  void SetName(string packageName);
  const Revision& GetRevision() const;
  void SetRevision(string revision);
  string GetSlot() const;
  void SetSlot(string slot);
  int GetBinaryCompatabilityRevision() const;
  void SetBinaryCompatabilityRevision(int revision);
  int GetNumSourceFiles() const;
  SourceFile* GetSourceFile(int fileNumber);
  bool AddSourceFile(SourceFile* file);
  bool RemoveSourceFile(int fileNumber);
  void ClearSourceFiles();
  bool RecordSourceChecksums(string storageDirectory);
  void AddUnpackInstruction(string instruction);
  bool DumpUnpackInstructionsToFile(FILE* file) const;
  void ClearUnpackInstructions();
  void AddConfigureInstruction(string instruction);
  bool DumpConfigureInstructionsToFile(FILE* file) const;
  void ClearConfigureInstructions();
  void AddBuildInstruction(string instruction);
  bool DumpBuildInstructionsToFile(FILE* file) const;
  void ClearBuildInstructions();
  void AddInstallInstruction(string instruction);
  bool DumpInstallInstructionsToFile(FILE* file) const;
  void ClearInstallInstructions();
  void AddUninstallInstruction(string instruction);
  bool DumpUninstallInstructionsToFile(FILE* file) const;
  void ClearUninstallInstructions();
  int GetNumDependencies() const;
  const PackageDependency* GetDependency(int dependencyNumber) const;
  void AddDependency(PackageDependency* dependency);
  bool RemoveDependency(string packageName);
  void ClearDependencies();
  int GetNumFeatures() const;
  FeatureOption* GetFeature(int featureNumber) const;
  void AddFeature(FeatureOption* feature);
  bool RemoveFeature(string featureName);
  void ClearFeatures();
  bool Fetch(InstallEntry* logEntry, string storageDirectory, bool force=false) const;
  bool Unpack(InstallEntry* logEntry, string storageDirectory, string workDirectory, int numProcessors);
  bool Configure(InstallEntry* logEntry, string workDirectory, int numProcessors);
  bool Build(InstallEntry* logEntry, string workDirectory, int numProcessors);
  bool Install(InstallEntry* logEntry, string workDirectory, int numProcessors);
private:
  //! The of the file which contains this package data
  string mFilename;
  //! List of required dependencies for intalling this package
  PackageDependencyVector mDependencies;
  //! List of optional features and their default states which can be used by this package
  FeatureOptionVector mFeatures;
  //! The name of the application
  string mPackageName;
  //! The revision of the application and this package
  Revision mPackageRevision;
  //! The slot which is occupies by this package
  string mPackageSlot;
  //! Value used to indiciate if two packages have binary compatable interfaces
  int mBinaryCompatabilityRevision;
  //! List of files required to install this package
  SourceFileVector mSourceFiles;
  //! List of installed files to be used for detecting if this package has been installed
  vector<string> mInstalledFiles;
  //! List of installed directories to be used for detecting if this package has been installed
  vector<string> mInstalledDirectories;
  //! The instructions to unpack this package
  InstructionSequence mUnpackInstructions;
  //! The instructions to configure this package
  InstructionSequence mConfigureInstructions;
  //! The instructions to build this package
  InstructionSequence mBuildInstructions;
  //! The instructions to install this package
  InstructionSequence mInstallInstructions;
  //! The instructions to uninstall this package
  InstructionSequence mUninstallInstructions;
};

typedef vector<Package *> PackageVector;

#endif
