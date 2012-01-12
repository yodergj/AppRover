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
  if ( FileExists("/bin/.AppRover.mv") )
    Execute("/bin/.AppRover.mv /bin/.AppRover.mv /bin/mv");
  if ( FileExists("/bin/.AppRover.cp") )
    Execute("/bin/mv /bin/.AppRover.cp /bin/cp");
  if ( FileExists("/bin/.AppRover.ln") )
    Execute("/bin/mv /bin/.AppRover.ln /bin/ln");
  if ( FileExists("/bin/.AppRover.mkdir") )
    Execute("/bin/mv /bin/.AppRover.mkdir /bin/mkdir");
  if ( FileExists("/bin/.AppRover.install") )
    Execute("/bin/mv /bin/.AppRover.install /bin/install");
  if ( FileExists("/usr/bin/.AppRover.install-info") )
    Execute("/bin/mv /usr/bin/.AppRover.install-info /usr/bin/install-info");
  if ( FileExists("/usr/bin/.AppRover.pod2man") )
    Execute("/bin/mv /usr/bin/.AppRover.pod2man /usr/bin/pod2man");

  return 0;
}
