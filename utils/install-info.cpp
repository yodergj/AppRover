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
  string command;
  FILE* logfile=NULL;
  string logFilename;
  char* environmentValue;
  string dirFilename = "";
  bool versionMode = false;

  environmentValue = getenv("APPROVER_FILELOG");
  if ( environmentValue )
    logFilename = environmentValue;
  else
    logFilename = ".AppRoverLogFile";

  for (i=1; i<argc; i++)
  {
    if ( argv[i][0] == '-' )
    {
      options.push_back(argv[i]);
      if ( !strncmp(argv[i],"--dir-file",10) )
        dirFilename = GetAbsolutePath(argv[i]+11);
      else if ( !strncmp(argv[i],"--info-dir",10) )
      {
        dirFilename = GetAbsolutePath(argv[i]+11);
        dirFilename += "/dir";
      }
      else if ( !strcmp(argv[i],"--version") )
        versionMode = true;
    }
    else
      arguments.push_back(argv[i]);
  }

  if ( !versionMode )
  {
    if ( dirFilename == "" )
    {
      if ( arguments.size() < 2 )
        return 1;
      dirFilename = GetAbsolutePath(arguments[1]);
    }

    logfile = fopen(logFilename.c_str(),"a");
    if ( logfile )
    {
      fprintf(logfile, "#Files from install-info\n");
      fprintf(logfile,"%s\n",dirFilename.c_str());
      fclose(logfile);
    }
  }

  command = "/usr/bin/.AppRover.install-info ";
  for (i=0; i<(int)options.size(); i++)
    command += options[i] + " ";
  for (i=0; i<(int)arguments.size(); i++)
    command += "\"" + arguments[i] + "\" ";
  if ( Execute(command) )
    return 0;
  return 1;
}
