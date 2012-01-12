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
#ifndef SOURCE_FILE_H
#define SOURCE_FILE_H

#include <string>
#include <vector>
#include <libxml/tree.h>
#include "FeatureOption.h"
#include "BooleanExpression.h"

using std::string;
using std::vector;

/*! @brief A class describing a package source file
*/
class SourceFile
{
public:
  SourceFile();
  SourceFile(string location);
  SourceFile(const SourceFile* refFile);
  ~SourceFile();
  string GetLocation() const;
  bool SetLocation(string location);
  string GetCheckSum() const;
  void SetCheckSum(string checksum);
  const BooleanExpression& GetCondition() const;
  void SetCondition(const BooleanExpression& condition);
  bool Save(xmlNodePtr parentNode);
  bool Load(xmlNodePtr fileNode);
  bool IsActive(const FeatureOptionVector& features) const;
  bool IsEmbedded() const;
  void SetContents(string contents);
  bool ExtractContents(string directory);
private:
  //! The remote location of this file, or if embedded file, just the filename
  string mLocation;
  //! The checksum for this file
  string mCheckSum;
  //! The expression which indicates whether this file is required
  BooleanExpression mCondition;
  //! Contents of an embedded file
  string mContents;
};

typedef vector<SourceFile *> SourceFileVector;

#endif
