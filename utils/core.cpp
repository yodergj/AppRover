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
#include <string>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include "core.h"

using std::string;
using std::vector;

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

string GetAbsolutePath(string path)
{
  int slashPos;
  int numBackups;
  string front, back, result;

  result = "";
  numBackups = 0;
  if ( path[0] != '/' )
    path = GetCurrentDir()+'/'+path;
  back = path;
  slashPos = back.rfind('/');
  while (slashPos >= 0)
  {
    front = back.substr(slashPos+1);
    back = back.substr(0,slashPos);
    if ( front == ".." )
      numBackups++;
    else if ( front != "." )
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

string GetPathPart(string path, int depth)
{
  int i = 0;
  int slashPos;
  int lastSlashPos = 0;
  string result = "";

  if ( (depth < 1) || (path == "") )
    return "";

  slashPos = path.find('/');
  if ( slashPos < 0 )
  {
    if ( depth == 1 )
      return path;
    return "";
  }
  if ( slashPos > 0 )
  {
    result = path.substr(0,slashPos);
    lastSlashPos = slashPos;
    i++;
  }
  for (; i<depth; i++)
  {
    if ( lastSlashPos+1 == (int)path.size() )
      return "";
    slashPos = path.find('/',lastSlashPos+1);
    if ( slashPos == (int)path.size()-1 )
      return "";
    if ( slashPos == -1 )
    {
      if ( i < depth-1 )
        return "";
      result = path;
    }
    else
    {
      result += path.substr(lastSlashPos,slashPos-lastSlashPos);
      lastSlashPos = slashPos;
    }
  }
  return result;
}

string GetFilename(string path)
{
  int slashPos;

  slashPos = path.rfind('/');
  if ( slashPos >= 0 )
    return path.substr(slashPos+1);
  return path;
}

string GetDirectoryPart(string path)
{
  int slashPos;

  slashPos = path.rfind('/');

  if ( slashPos < 0 )
    return "";

  return path.substr(0,slashPos+1);
}

bool IsADirectory(string location)
{
  DIR* dir;
  dir = opendir(location.c_str());
  if ( !dir )
    return false;
  closedir(dir);
  return true;
}

bool FileExists(string location)
{
  FILE* file;
  file = fopen(location.c_str(),"r");
  if ( file )
  {
    fclose(file);
    return true;
  }
  return false;
}

void GetDirectoryContents(string srcDirectory, string destinationBase, vector<string>& destinationDirectories, vector<string>& destinationFiles)
{
  DIR* dir;
  struct dirent* dirEntry;
  string entryName;

  dir = opendir(srcDirectory.c_str());
  if ( !dir )
    return;

  dirEntry = readdir(dir);
  while (dirEntry)
  {
    entryName = dirEntry->d_name;
    if ( (entryName != ".") && (entryName != "..") )
    {
      if ( IsADirectory(srcDirectory+"/"+entryName) )
      {
	destinationDirectories.push_back(GetAbsolutePath(destinationBase+"/"+entryName));
	GetDirectoryContents(srcDirectory+"/"+entryName,destinationDirectories[destinationDirectories.size()-1],destinationDirectories,destinationFiles);
      }
      else
	destinationFiles.push_back(destinationBase+"/"+entryName);
    }
    dirEntry = readdir(dir);
  }
  closedir(dir);
}

void GetDirectories(string path,vector<string>& destinationDirectories)
{
  int depth;
  string absoluteDirectory;
  string pathPart;
  string currentDir = GetCurrentDir();

  absoluteDirectory = GetAbsolutePath(path);
  depth = 1;
  pathPart = GetPathPart(absoluteDirectory,depth);
  while ( pathPart != "" )
  {
    if ( pathPart != GetPathPart(currentDir,depth) )
      destinationDirectories.push_back(pathPart);
    depth++;
    pathPart = GetPathPart(absoluteDirectory,depth);
  }
}

void GetDirectories(vector<string>& arguments,vector<string>& destinationDirectories)
{
  int i;
  int numInputs;

  numInputs = arguments.size();
  for (i=0; i<numInputs; i++)
    GetDirectories(arguments[i],destinationDirectories);
}

void GetDestinationFiles(vector<string>& arguments,vector<string>& destinationFiles, vector<string>& destinationDirectories, bool recursive,string destDir)
{
  vector<string> directories;
  int numArguments = arguments.size();
  int numSources;
  string currentDir;
  int i;
  string filename;

  if ( !numArguments )
    return;

  if ( destDir == "" )
  {
    numSources = numArguments-1;
    if ( IsADirectory(arguments[numArguments-1]) )
    {
      destDir = arguments[numArguments-1];
    }
    else
    {
      if ( numArguments != 2 )
	return;
      if ( IsADirectory(arguments[0]) )
	return;
      destinationFiles.push_back(GetAbsolutePath(arguments[numArguments-1]));
      return;
    }
  }
  else
  {
    if ( !IsADirectory(destDir) )
      return;
    numSources = numArguments;
  }
  destDir = GetAbsolutePath(destDir);
  currentDir = GetCurrentDir();
  for (i=0; i<numSources; i++)
  {
    if ( IsADirectory(arguments[i]) )
    {
      if ( recursive )
      {
	destinationDirectories.push_back(destDir+"/"+GetFilename(arguments[i]));
	GetDirectoryContents(arguments[i],destinationDirectories[destinationDirectories.size()-1],
                             destinationDirectories,destinationFiles);
      }
    }
    else
      destinationFiles.push_back(destDir+"/"+GetFilename(arguments[i]));
  }
}

void GetLinkDestinationFiles(vector<string>& arguments,vector<string>& destinationFiles, string destDir)
{
  int numArguments = arguments.size();
  int numSources;
  int i;

  if ( !numArguments )
    return;

  if ( destDir == "" )
  {
    if ( numArguments == 1 )
    {
      destinationFiles.push_back(GetAbsolutePath(GetFilename(arguments[0])));
      return;
    }
    if ( !IsADirectory(arguments[numArguments-1]) )
    {
      if ( numArguments != 2 )
	return;
      destinationFiles.push_back(GetAbsolutePath(arguments[1]));
      return;
    }
    destDir = arguments[numArguments-1];
    numSources = numArguments-1;
  }
  else
  {
    if ( !IsADirectory(destDir) )
      return;
    numSources = numArguments;
  }
  destDir = GetAbsolutePath(destDir);
  for (i=0; i<numSources; i++)
    destinationFiles.push_back(destDir+"/"+GetFilename(arguments[i]));
}

bool Execute(string command)
{
  int retcode, exitcode;
  int cdPos;

  cdPos = command.find("cd ");
  if ( cdPos == 0 )
  {
    if ( !chdir(command.substr(3).c_str()) )
      return true;
    return false;
  }

  retcode = system(command.c_str());
  if ( retcode == -1 )
    return false;

  if ( WIFEXITED(retcode) )
  {
    exitcode = WEXITSTATUS(retcode);
    if ( exitcode )
      return false;
    return true;
  }

  return false;
}
