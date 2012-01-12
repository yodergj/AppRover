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
#ifndef CORE_H
#define CORE_H

#include <string>
#include <vector>

using std::string;
using std::vector;

string GetAbsolutePath(string path);
void GetDestinationFiles(vector<string>& arguments,vector<string>& destinationFiles,vector<string>& destinationDirectories,bool recursive,string destDir);
void GetLinkDestinationFiles(vector<string>& arguments,vector<string>& destinationFiles,string destDir);
void GetDirectories(vector<string>& arguments,vector<string>& destinationDirectories);
void GetDirectories(string path,vector<string>& destinationDirectories);
bool Execute(string command);
bool FileExists(string location);
string GetDirectoryPart(string path);

#endif
