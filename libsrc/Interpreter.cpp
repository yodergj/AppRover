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
#include "Interpreter.h"
#include "DirectoryUtils.h"

// Public----------------------------------------------------------------------

Interpreter::~Interpreter()
{
}

/*!
  @param command - the instruction to execute
  @param pleaseWait - if true, indicates that the user should be somehow prompted to wait for execution to finish
  @return false if the instruction fails

  Executes the specified instruction from the present working directory.
*/
bool Interpreter::Interpret(string command, bool pleaseWait)
{
  int retcode, exitcode;
  int cdPos;

  //! @todo If command starts with "@", make in execute silently
  cdPos = command.find("cd ");
  if ( cdPos == 0 )
    return ChangeDirectory(command.substr(3));

  retcode = system(command.c_str());
  if ( retcode == -1 )
    return false;

  if ( WIFSIGNALED(retcode) )
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
