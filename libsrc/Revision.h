///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004-2007 Gabriel Yoder
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
#ifndef REVISION_H
#define REVISION_H

#include <string>
#include <vector>

using std::string;
using std::vector;

/*!
  @brief A simple class for handling package revision strings

  A revision consists of the version of an application combined with and indicator of how many times a package for this application has been redone (such as "1.00beta1", "1.00", "1.00-r1", "1.00-r2", "1.02")
*/
class Revision
{
public:
  Revision();
  Revision(string revision);
  ~Revision();
  bool IsSet() const;
  Revision& operator=(const Revision& src);
  bool operator<(const Revision& refRevision) const;
  bool operator<=(const Revision& refRevision) const;
  bool operator==(const Revision& refRevision) const;
  bool operator>=(const Revision& refRevision) const;
  bool operator>(const Revision& refRevision) const;
  string GetString() const;
  void Set(string revision);
private:
  void SetValues();
  //! The contents of this revision in a readable string
  string mRevisionStr;
  //! The alpha number of this revision (i.e. alpha 1, alpha 2, etc.)
  int mAlphaNumber;
  //! The beta number of this revision (i.e. beta 1, beta 2, etc.)
  int mBetaNumber;
  //! The release candidate number of this revision (i.e. RC 1, RC 2, etc.)
  int mRCNumber;
  //! List of the leading numeric components in the application's version
  vector<int> mVersionParts;
  //! Non-numeric revision suffix aside from alpha, beta, and rc
  string mRemnant;
};

#endif
