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
#include "SourceFile.h"
#include "Labels.h"
#include "DirectoryUtils.h"
#include "XMLUtils.h"
#include <string.h>

// Public----------------------------------------------------------------------

SourceFile::SourceFile()
{
  mLocation = "";
  mCheckSum = "";
}

/*!
  @param location - the remote location of the source file
*/
SourceFile::SourceFile(string location)
{
  mLocation = location;
}

/*!
  @param refFile - SourceFile to clone
*/
SourceFile::SourceFile(const SourceFile* refFile)
{
  mLocation = refFile->mLocation;
  mCheckSum = refFile->mCheckSum;
  mCondition = refFile->mCondition;
  mContents = refFile->mContents;
}

SourceFile::~SourceFile()
{
}

/*!
  @return the remote location of the source file
*/
string SourceFile::GetLocation() const
{
  return mLocation;
}

/*!
  @param location - the remote location of the source file
  @return true if the location is properly formatted (currently only checks for a lack of whitespace)
*/
bool SourceFile::SetLocation(string location)
{
  if ( location.find_first_not_of(" \t\n\r") == string::npos  )
    return false;

  mLocation = location;
  return true;
}

/*!
  @return the checksum of this file
*/
string SourceFile::GetCheckSum() const
{
  return mCheckSum;
}

/*!
  @param checksum - the checksum of this file
*/
void SourceFile::SetCheckSum(string checksum)
{
  mCheckSum = checksum;
}

/*!
  @return the expression indicating whether this file is required
*/
const BooleanExpression& SourceFile::GetCondition() const
{
  return mCondition;
}

/*!
  @param condition - the expression indicating whether this file is required
*/
void SourceFile::SetCondition(const BooleanExpression& condition)
{
  mCondition = condition;
}

/*!
  @param parentNode - the parent XML DOM node for the file node
  @return false if parentNode is NULL

  Saves the file data as a child node of the specified parent.
*/
bool SourceFile::Save(xmlNodePtr parentNode)
{
  xmlNodePtr fileNode;
  xmlNodePtr contentsNode;

  if ( !parentNode )
    return false;

  fileNode = xmlNewNode(NULL, (const xmlChar*)SOURCE_FILE_STR);

  SetStringValue(fileNode, LOCATION_STR, mLocation);
  SetStringValue(fileNode, CHECKSUM_STR, mCheckSum);
  SetStringValue(fileNode, CONDITION_STR, mCondition.GetString());

  xmlAddChild(parentNode, fileNode);

  if ( mContents != "" )
  {
    contentsNode = xmlNewText((const xmlChar*)mContents.c_str());
    xmlAddChild(fileNode, contentsNode);    
  }

  return true;
}

/*!
  @param fileNode - the XML DOM node containing the file data
  @return false if fileNode is NULL or there is a format error

  Loads the file data from the specified node.
*/
bool SourceFile::Load(xmlNodePtr fileNode)
{
  string revision;
  xmlNodePtr currentNode;  

  if ( !fileNode )
    return false;

  mLocation = GetStringValue(fileNode, LOCATION_STR);
  if ( mLocation == "" )
    return false;

  mCheckSum = GetStringValue(fileNode, CHECKSUM_STR);
  if ( !mCondition.ParseFromString(GetStringValue(fileNode, CONDITION_STR)) )
    return false;

  currentNode = fileNode->children;
  while ( currentNode )
  {
    if ( strcmp((const char *)currentNode->name, "text") )
      return false;

    mContents = (char *)currentNode->content;

    currentNode = currentNode->next;
  }

  return true;
}

/*!
  @param features - the features used to evaluate whether this file is required
  @return the result of evaluating the condition with respect to the value of the features
  @return true if the condition is empty
*/
bool SourceFile::IsActive(const FeatureOptionVector& features) const
{
  if ( mCondition.IsEmpty() )
    return true;

  return mCondition.EvaluatesToTrue(features);
}

/*!
  @return true if the file contents are embedded in this object, false otherwise
*/
bool SourceFile::IsEmbedded() const
{
  return mContents != "";
}

/*! Sets the embedded file contents to the specified string.  If the string is empty, the contents are cleared and this source file will no longer be embedded.
  @param contents - the contents of the embedded file
*/
void SourceFile::SetContents(string contents)
{
  mContents = contents;
}

/*! Saves the embedded file contents to the specified directory with the filename determined by the contents of the location field.
  @param contents - the contents of the embedded file
  @return true on success
  @return false if file is not embedded or other error
*/
bool SourceFile::ExtractContents(string directory)
{
  string filename;
  FILE* file;
  bool retCode = true;

  if ( (directory == "") || (mContents == "") )
    return false;

  if ( !MakeDirectory(directory) )
    return false;

  filename = directory + GetFilenamePart(mLocation);
  file = fopen(filename.c_str(), "w");
  if ( !file )
    return false;

  if ( fputs(mContents.c_str(), file) == EOF )
    retCode = false;

  fclose(file);

  return true;
}

// Private---------------------------------------------------------------------
