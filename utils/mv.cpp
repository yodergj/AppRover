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
#include <stdio.h>
#include "core.h"

using std::string;
using std::vector;

int main(int argc, char *argv[])
{
  int i;
  vector<string> options;
  vector<string> arguments;
  vector<string> destinationFiles;
  vector<string> destinationDirectories;
  string command;
  FILE* logfile=NULL;
  string logFilename;
  char* environmentValue;

  environmentValue = getenv("APPROVER_FILELOG");
  if ( environmentValue )
    logFilename = environmentValue;
  else
    logFilename = ".AppRoverLogFile";

  for (i=1; i<argc; i++)
  {
    if ( argv[i][0] == '-' )
      options.push_back(argv[i]);
    else
      arguments.push_back(argv[i]);
  }

  GetDestinationFiles(arguments,destinationFiles,destinationDirectories,true,"");

  logfile = fopen(logFilename.c_str(),"a");
  if ( logfile )
  {
    fprintf(logfile, "#Files from mv\n");
    for (i=0; i<(int)destinationFiles.size(); i++)
      fprintf(logfile,"%s\n",destinationFiles[i].c_str());
    fprintf(logfile, "#Directories from mv\n");
    for (i=0; i<(int)destinationDirectories.size(); i++)
      fprintf(logfile,"%s\n",destinationDirectories[i].c_str());
    fclose(logfile);
  }

  command = "/bin/.AppRover.mv ";
  for (i=0; i<(int)options.size(); i++)
    command += options[i] + " ";
  for (i=0; i<(int)arguments.size(); i++)
    command += "\"" + arguments[i] + "\" ";
  if ( Execute(command) )
    return 0;
  return 1;
}
