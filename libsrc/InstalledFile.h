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
#ifndef INSTALLED_FILE_H
#define INSTALLED_FILE_H

#include <string>
#include <vector>
#include <libxml/tree.h>

using std::string;
using std::vector;

/*! @brief A class describing a file installed by a package
*/
class InstalledFile
{
public:
  InstalledFile();
  InstalledFile(string filename);
  ~InstalledFile();
  string GetFilename() const;
  void SetFilename(string filename);
  string GetBackupFilename() const;
  string GetChecksum() const;
  void SetChecksum(string checksum);
  bool IsAnInheritedFile() const;
  void SetInherited(bool inherited);
  bool NeedsBackedUp() const;
  bool IsABackupFile() const;
  void SetBackup(bool backupState);
  bool Save(xmlNodePtr parentNode);
  bool Load(xmlNodePtr fileNode);
  bool operator==(const InstalledFile& refFile) const;
  bool operator<(const InstalledFile& refFile) const;
private:
  //! The full path of this file
  string mFilename;
  //! The filename portion without the path
  string mFilePart;
  //! The directory where this file is located
  string mDirectory;
  //! The checksum for this file
  string mChecksum;
  //! Is this file a backup copy of another installed file
  bool mIsABackupFile;
  //! Is this a file whose ownership was transferred
  bool mIsAnInheritedFile;
};

//! Vector class for InstalledFile which prevents duplicate entries
class InstalledFileVector : public vector<InstalledFile *>
{
public:
  void push_back(InstalledFile* element);
};

#endif
