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
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "DirectoryUtils.h"
#include "Labels.h"
#include "Interpreter.h"

/*!
  @param directoryName - the path (relative or full) to the directory to create
  @return false if directoryName is "", or cannot be created
  @return true if successful or directory already exists

  Creates the specified directory.  If the parent directories do not exist, they will be created as well.
*/
bool MakeDirectory(string directoryName)
{
  string command;
  int retcode;
  int length;

  if ( directoryName == "" )
    return false;
  length = directoryName.length();
  if ( directoryName[length - 1] == '/' )
    directoryName.resize(length - 1);
  retcode = mkdir(directoryName.c_str(),0777);
  if ( retcode == -1 )
  {
    if ( errno == EEXIST )
      return true;
    if ( errno == ENOENT )
    {
      int pos = directoryName.rfind(SLASH_STR);
      directoryName[pos] = 0;
      if ( MakeDirectory(directoryName) )
      {
        directoryName[pos] = '/';
        return MakeDirectory(directoryName);
      }
    }
    return false;
  }
  return true;
}

/*!
  @param directoryName - the path (relative or full) to the current directory of the running process
  @return false if directoryName is "", or the directory does not exist and cannot be created
  @return true if successful

  Changes the present working directory to the new directory.  If the directory does not exist, it will be created.
*/
bool ChangeDirectory(string directoryName)
{
  if ( directoryName == "" )
    return false;

  if ( chdir(directoryName.c_str()) == 0 )
    return true;
  if ( errno == ENOENT )
  {
    if ( MakeDirectory(directoryName) && (chdir(directoryName.c_str())==0) )
      return true;
  }
  return false;
}

/*!
  @return the absolute path of the current directory
*/
string GetCurrentDir()
{
  static char *buf=NULL;
  static size_t size=0;

  if ( !buf )
  {
    size = 256;
    buf = (char *)malloc(size);
  }
  if (!buf)
    exit(1);
  while ( !getcwd(buf,size) )
  {
    if ( errno != ERANGE )
      exit(1);
    size *= 2;
    buf = (char *)realloc(buf,size);
    if ( !buf )
      exit(1);
  }
  return buf;
}

/*!
  @param location - the relative or full path of the file or directory
  @return true if the location is a directory
  @return false otherwise
*/
bool IsADirectory(string location)
{
  DIR* dir;
  dir = opendir(location.c_str());
  if ( !dir )
    return false;
  closedir(dir);
  return true;
}

/*!
  @param filename - the relative or full path of the file
  @return true if the file exists
  @return false otherwise
*/
bool FileExists(string filename)
{
  FILE *fp;

  fp = fopen(filename.c_str(),"r");

  if ( !fp )
    return false;

  fclose(fp);
  return true;
}

/*!
  @param path - the relative path, full path, or URL of a file
  @return the filename portion of the path excluding all directory locations
*/
string GetFilenamePart(string path)
{
  int slashPos;

  slashPos = path.rfind(SLASH_STR);

  if ( slashPos < 0 )
    return path;

  return path.substr(slashPos+1);
}

/*!
  @param path - the relative path, full path, or URL of a file
  @return the path with the filename portion excluded
*/
string GetDirectoryPart(string path)
{
  int slashPos;

  slashPos = path.rfind(SLASH_STR);

  if ( slashPos < 0 )
    return "";

  return path.substr(0,slashPos+1);
}

/*!
  @param path - the absolute path
  @return the absolute path in its shortest form (without ".." and "." entries)
*/
string GetCleanedPath(string path)
{
  int slashPos;
  int numBackups;
  string front, back, result;

  result = "";
  numBackups = 0;
  back = path;
  slashPos = back.rfind('/');
  while (slashPos >= 0)
  {
    front = back.substr(slashPos+1);
    back = back.substr(0,slashPos);
    if ( front == ".." )
      numBackups++;
    else if ( (front != ".") && (front != "") )
    {
      if ( numBackups )
	numBackups--;
      else
      {
	if ( result != "" )
	  result = "/" + result;
	result = front + result;
      }
    }
    slashPos = back.rfind('/');
  }
  result = "/" + result;
  return result;
}

/*!
  @param filename - the relative or absolute path of a file whose checksum is needed
  @return the md5sum string for the file
*/
string GetChecksum(string filename)
{
  string sumFilename = GetDirectoryPart(filename) + ".md5sum";
  char buf[128];
  FILE* sumFile = NULL;
  string checksum = "";

  // Make sure we properly handle filenames where some idiot has inserted
  // whitespace
  if ( !gCommandInterpreter->Interpret("md5sum \"" + filename + "\" > " + sumFilename + " 2> /dev/null", true) )
  {
    gCommandInterpreter->Interpret("rm -f " + sumFilename);
    return "";
  }
  sumFile = fopen(sumFilename.c_str(),"r");
  if ( !sumFile )
    return "";
  if ( fscanf(sumFile,"%127s",buf) == 1 )
  {
    buf[127] = 0;
    checksum = buf;
  }
  fclose(sumFile);
  gCommandInterpreter->Interpret("rm -f " + sumFilename);
  return checksum;
}

#define BUFFER_LENGTH 256

/*!
  @param fp - the file from which a line should be read
  @param eof - receives the state of whether the end of file has been reached
  @return the contents of a single line in the file (without \\r or \\n)
*/
string Readline(FILE* fp, bool& eof)
{
  char buf[BUFFER_LENGTH];
  string line;
  int length;

  eof = false;

  if ( !fp )
    return "";

  line = "";
  while ( fgets(buf, BUFFER_LENGTH, fp) )
  {
    line += buf;
    length = line.length();
    if ( (line[length - 1] == '\n') ||
         (line[length - 1] == '\r') )
    {
      while ( (line[length - 1] == '\n') ||
              (line[length - 1] == '\r') )
      {
	line.resize(length - 1);
	length--;
      }
      return line;
    }
  }
  if ( line == "" )
    eof = true;
  return line;
}

/*!
  @param filename - the name of the file to read
  @return the contents of the file
*/
string ReadFile(const char* filename)
{
  FILE* fp;
  char buf[BUFFER_LENGTH];
  string contents;

  if ( !filename )
    return "";

  fp = fopen(filename, "r");
  
  if ( !fp )
    return "";

  while ( fgets(buf, BUFFER_LENGTH, fp) )
    contents += buf;

  fclose(fp);

  return contents;
}

/*!
  @param fp - the file from which a line should be read
  @param error - receives the state of whether a read or format error was encountered
  @return the integer value found at the current position of the file
*/
int ReadInt(FILE* fp, bool& error)
{
  bool eof;
  string line;
  char *endChar;
  long value = 0;

  line = Readline(fp,eof);
  if ( eof )
  {
    error = true;
    return 0;
  }
  value = strtol(line.c_str(),&endChar,10);
  if ( line.c_str() == endChar )
    error = true;
  else
    error = false;
  return (int)value;
}
