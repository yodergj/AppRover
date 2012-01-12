///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Gabriel Yoder
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
#include "Fetcher.h"
#include "DirectoryUtils.h"
#include "Interpreter.h"
#include "Labels.h"

// Public----------------------------------------------------------------------

Fetcher::Fetcher()
{
}

Fetcher::~Fetcher()
{
}

/*!
  @param fileLocation - the remote location of the file
  @param storageDirectory - the directory to hold the local copy of the file
  @param checksum - string to check md5sum against, check disabled if ""
  @param force - if true, the file will be fetched even if a local copy exists
  @return false if file cannot be downloaded or the file exists and cannot be forcibly removed
  @return true if successful or fle already exists

  Downloads the file from the specifed location and stores it in the storage directory.  If the file is already in the storage directory, it will only be downloaded if force is true.
*/
bool Fetcher::Fetch(string fileLocation, string storageDirectory, string checksum, bool force) const
{
  string filename;

  // TODO : add support for downloading with things other than wget

  filename = GetFilenamePart(fileLocation);
  if ( FileExists(storageDirectory+filename) )
  {
    if ( checksum == "" )
      return true;
    if ( (checksum == ::GetChecksum(storageDirectory+filename)) && !force )
      return true;
    if ( !gCommandInterpreter->Interpret(REMOVE_STR + storageDirectory + filename) )
      return false;
  }

  if ( !gCommandInterpreter->Interpret("cd " + storageDirectory) )
    return false;
  return gCommandInterpreter->Interpret("wget " + fileLocation);
}

/*!
  @param fileLocation - the remove location of the file
  @param storageDirectory - the directory where the file is locally stored
  @return the checksum for the local copy of the specified file
*/
string Fetcher::GetChecksum(string fileLocation, string storageDirectory) const
{
  string filename;
  filename = GetFilenamePart(fileLocation);
  return ::GetChecksum(storageDirectory+filename);
}

// Private---------------------------------------------------------------------
