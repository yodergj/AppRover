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
#include <sys/wait.h>
#include <signal.h>
#include <kapplication.h>
#include "kInterpreter.h"
#include "DirectoryUtils.h"

// Public----------------------------------------------------------------------

kInterpreter::kInterpreter()
{
  mWaitDialog = NULL;
}

/*!
  @param command - the instruction to execute
  @param pleaseWait - if true, indicates that the user should be prompted to wair for execution to finish
  @return false if the instruction fails

  Executes the specified instruction from the present working directory.
*/
bool kInterpreter::Interpret(string command, bool pleaseWait)
{
  int retcode, exitcode;
  int cdPos;
  string currentDir;
  FILE* scriptFile = NULL;
  bool result;

  //! @todo If command starts with "@", make it execute silently
  cdPos = command.find("cd ");
  if ( cdPos == 0 )
    return ChangeDirectory(command.substr(3));

  currentDir = GetCurrentDir();
  scriptFile = fopen(".AppRoverInterpreter.sh","w");
  if ( !scriptFile )
    return false;
  fprintf(scriptFile,"#!/bin/sh\n");
  fprintf(scriptFile,"%s || touch %s/.AppRoverInterpreterError\n",command.c_str(),currentDir.c_str());
  fprintf(scriptFile,"touch %s/.AppRoverInterpreterDone\n",currentDir.c_str());
  fclose(scriptFile);
  system("chmod a+x .AppRoverInterpreter.sh");
  system("rm -f .AppRoverInterpreterError .AppRoverInterpreterDone");

  retcode = system("./.AppRoverInterpreter.sh &");
  if ( retcode == -1 )
    return false;

  if ( WIFSIGNALED(retcode) )
  {
    // In the long term, this should probably be changed.
    // This exists so that behave appears the same between platforms.
    // When system is executing, this process is supposed to ignore
    // SIGINT (it is sent to the subprocess).  BeOS does not follow the
    // posix standard in this regard.  By raising SIGINT, we can make the
    // posix conformant platforms mimic the behavior under BeOS.
    if ( WTERMSIG(retcode) == SIGINT )
      raise(SIGINT);
  }
  else if ( WIFEXITED(retcode) )
  {
    exitcode = WEXITSTATUS(retcode);
    if ( exitcode )
      return false;
  }

  if ( pleaseWait )
  {
    if ( !mWaitDialog )
      mWaitDialog = new WaitDialog(KApplication::kApplication()->mainWidget());
    mWaitDialog->show();
  }

  while ( !FileExists(".AppRoverInterpreterDone") )
    KApplication::kApplication()->processEvents();

  if ( pleaseWait )
    mWaitDialog->hide();

  result = !FileExists(".AppRoverInterpreterError");
  system("rm -f .AppRoverInterpreter.sh .AppRoverInterpreterError .AppRoverInterpreterDone");

  return result;
}
