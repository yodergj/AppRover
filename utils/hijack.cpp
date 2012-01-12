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
#include <stdlib.h>
#include "core.h"

using std::string;
using std::vector;

int main()
{
  char* environmentValue;
  string utilDirectory;

  environmentValue = getenv("APPROVER_UTIL_DIR");
  if ( !environmentValue )
  {
    fprintf(stderr,"Override directory not specified\n");
    return 1;
  }

  if ( FileExists("/bin/.AppRover.cp") ||
       FileExists("/bin/.AppRover.mv") ||
       FileExists("/bin/.AppRover.ln") ||
       FileExists("/bin/.AppRover.mkdir") ||
       FileExists("/bin/.AppRover.install") ||
       FileExists("/bin/.AppRover.install-info") )
  {
    fprintf(stderr,"System has already been hijacked\n");
    return 1;
  }

  utilDirectory = environmentValue;

  Execute("/bin/mv /bin/cp /bin/.AppRover.cp");
  Execute("/bin/mv /bin/install /bin/.AppRover.install");
  Execute("/bin/mv /bin/ln /bin/.AppRover.ln");
  Execute("/bin/mv /bin/mkdir /bin/.AppRover.mkdir");
  Execute("/bin/mv /usr/bin/install-info /usr/bin/.AppRover.install-info");
  Execute("/bin/mv /usr/bin/pod2man /usr/bin/.AppRover.pod2man");
  Execute("/bin/mv /bin/mv /bin/.AppRover.mv");

  Execute("/bin/.AppRover.ln -s " + utilDirectory + "/cp /bin/cp");
  Execute("/bin/.AppRover.ln -s " + utilDirectory + "/ln /bin/ln");
  Execute("/bin/.AppRover.ln -s " + utilDirectory + "/mv /bin/mv");
  Execute("/bin/.AppRover.ln -s " + utilDirectory + "/install /bin/install");
  Execute("/bin/.AppRover.ln -s " + utilDirectory + "/mkdir /bin/mkdir");
  Execute("/bin/.AppRover.ln -s " + utilDirectory + "/install-info /usr/bin/install-info");
  Execute("/bin/.AppRover.ln -s " + utilDirectory + "/pod2man /usr/bin/pod2man");

  return 0;
}
