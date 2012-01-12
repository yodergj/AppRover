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
#include <string.h>
#include "InstalledFile.h"
#include "DirectoryUtils.h"
#include "Labels.h"
#include "XMLUtils.h"

// Public----------------------------------------------------------------------

InstalledFile::InstalledFile()
{
  mFilename = "";
  mChecksum = "";
  mIsABackupFile = false;
  mIsAnInheritedFile = false;
}

/*!
  @param filename - the absolute path of the installed file
*/
InstalledFile::InstalledFile(string filename)
{
  mFilename = filename;
  mChecksum = "";
  mIsABackupFile = false;
  mIsAnInheritedFile = false;
}

InstalledFile::~InstalledFile()
{
}

/*!
  @return the absolute path of the installed file
*/
string InstalledFile::GetFilename() const
{
  return mFilename;
}

/*!
  @param filename - the absolute path of the installed file
*/
void InstalledFile::SetFilename(string filename)
{
  mFilename = filename;
  mFilePart = GetFilenamePart(mFilename);
  mDirectory = GetDirectoryPart(mFilename);
}

/*!
  @return the absolute path that would identify a backup of this file
*/
string InstalledFile::GetBackupFilename() const
{
  return mDirectory + "." + mFilePart + ".arbak";
}

/*!
  @return the checksum of this file
*/
string InstalledFile::GetChecksum() const
{
  return mChecksum;
}

/*!
  @param checksum - the checksum of this file
*/
void InstalledFile::SetChecksum(string checksum)
{
  mChecksum = checksum;
}

/*!
  @return the inherited state of this file
*/
bool InstalledFile::IsAnInheritedFile() const
{
  return mIsAnInheritedFile;
}

/*!
  @param inherited - the inherited state of this file
*/
void InstalledFile::SetInherited(bool inherited)
{
  mIsAnInheritedFile = inherited;
}

/*!
  @return true if this file has data that may need protection from overwriting
*/
bool InstalledFile::NeedsBackedUp() const
{
  int len;
  const char* fileStr;

  /* Backups of backups are kind of absurd */
  if ( mIsABackupFile )
    return false;

  if ( !FileExists(mFilename) )
    return false;

  /* Don't back up files that are definitely binaries */
  len = mFilePart.size();
  if ( len >= 2 )
  {
    fileStr = mFilePart.c_str();
    if ( !strcmp(fileStr + len - 2, ".a") )
      return false;
    if ( !strcmp(fileStr + len - 2, ".o") )
      return false;
    if ( len >= 3 )
    {
      if ( !strcmp(fileStr + len - 3, ".so") )
        return false;
      if ( !strcmp(fileStr + len - 3, ".la") )
        return false;
      if ( mFilePart.find(".a.") != string::npos )
        return false;
      if ( mFilePart.find(".la.") != string::npos )
        return false;
      if ( mFilePart.find(".so.") != string::npos )
        return false;
    }
  }

  if ( (mChecksum != "") && (mChecksum != ::GetChecksum(mFilename)) )
    return true;

  return false;
}

/*!
  @return the backup state of this file
*/
bool InstalledFile::IsABackupFile() const
{
  return mIsABackupFile;
}

/*!
  @param backupState - the backup state of this file
*/
void InstalledFile::SetBackup(bool backupState)
{
  mIsABackupFile = backupState;
}

/*!
  @param parentNode - the parent XML DOM node for the file node
  @return false if parentNode is NULL

  Saves the file data as a child node of the specified parent.
*/
bool InstalledFile::Save(xmlNodePtr parentNode)
{
  xmlNodePtr fileNode;

  if ( !parentNode )
    return false;

  fileNode = xmlNewNode(NULL,(const xmlChar*)FILE_STR);

  SetStringValue(fileNode, LOCATION_STR, mFilename);
  SetStringValue(fileNode, CHECKSUM_STR, mChecksum);
  SetBoolValue(fileNode, BACKUP_STR, mIsABackupFile);
  SetBoolValue(fileNode, INHERITED_STR, mIsAnInheritedFile);

  xmlAddChild(parentNode,fileNode);

  return true;
}

/*!
  @param fileNode - the XML DOM node containing the file data
  @return false if fileNode is NULL or there is a format error

  Loads the file data from the specified node.
*/
bool InstalledFile::Load(xmlNodePtr fileNode)
{
  string revision;
  string filename;

  if ( !fileNode )
    return false;

  filename = GetStringValue(fileNode, LOCATION_STR);
  SetFilename(filename);

  mChecksum = GetStringValue(fileNode, CHECKSUM_STR);
  mIsABackupFile = GetBoolValue(fileNode, BACKUP_STR, false);
  mIsAnInheritedFile = GetBoolValue(fileNode, INHERITED_STR, false);

  return true;
}

/*!
  @param refFile - the installed file to compare against
  @return true if filenames are the same
*/
bool InstalledFile::operator==(const InstalledFile& refFile) const
{
  return mFilename == refFile.mFilename;
}

/*!
  @param refFile - the installed file to compare against
  @return true if the filename of this file precedes refFile's filename
*/
bool InstalledFile::operator<(const InstalledFile& refFile) const
{
  if ( mDirectory == refFile.mDirectory )
    return mFilePart < refFile.mFilePart;
  return mDirectory < refFile.mDirectory;
}

// Private---------------------------------------------------------------------
