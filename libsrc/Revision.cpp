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
#include <stdio.h>
#include "Revision.h"

#define NOT_SET -1
#define MIN_VAL 0

// Public----------------------------------------------------------------------

Revision::Revision()
{
  mRevisionStr = "";
  SetValues();
}

/*!
  @param revision - the string representation of this revision
*/
Revision::Revision(string revision)
{
  mRevisionStr = revision;
  SetValues();
}

Revision::~Revision()
{
}

/*!
  @return false if this revision is empty (unspecified)
*/
bool Revision::IsSet() const
{
  return mRevisionStr != "";
}

/*!
  @param src - the revision to copy
  @return reference to this revision
*/
Revision& Revision::operator=(const Revision& src)
{
  mRevisionStr = src.mRevisionStr;
  return *this;
}

/*!
  @param refRevision - the revision that this revision is compared against
  @return true if this revision is earlier than the reference revision

  To determine if a revision comes earlier, first the corresponding numeric parts of the application version are compared.  If all of the corresponding parts match, the revision with fewer numeric components is considered earlier.  Next, the alpha numbers are compared, followed by the beta and rc numbers.  Finally, the number of times the package has been revised should be compared (currently unimplemented).  All other comparison operators determine their values by using the results of this operator and checking for equality.
*/
bool Revision::operator<(const Revision& refRevision) const
{
  int i;
  int minSize;

  if ( *this == refRevision )
    return false;

  if ( mVersionParts.size() < refRevision.mVersionParts.size() )
    minSize = mVersionParts.size();
  else
    minSize = refRevision.mVersionParts.size();

  for (i=0; i<minSize; i++)
  {
    if ( mVersionParts[i] < refRevision.mVersionParts[i] )
      return true;
    if ( mVersionParts[i] > refRevision.mVersionParts[i] )
      return false;
  }
  if ( mVersionParts.size() < refRevision.mVersionParts.size() )
    return true;
  if ( mVersionParts.size() > refRevision.mVersionParts.size() )
    return false;
  if ( mAlphaNumber != refRevision.mAlphaNumber )
  {
    if ( mAlphaNumber == NOT_SET )
      return false;
    if ( refRevision.mAlphaNumber == NOT_SET )
      return true;
    return (mAlphaNumber < refRevision.mAlphaNumber);
  }
  if ( mBetaNumber != refRevision.mBetaNumber )
  {
    if ( mBetaNumber == NOT_SET )
      return false;
    if ( refRevision.mBetaNumber == NOT_SET )
      return true;
    return (mBetaNumber < refRevision.mBetaNumber);
  }
  if ( mRCNumber != refRevision.mRCNumber )
  {
    if ( mRCNumber == NOT_SET )
      return false;
    if ( refRevision.mRCNumber == NOT_SET )
      return true;
    return (mRCNumber < refRevision.mRCNumber);
  }
  //! @todo Add support for checking the revision number

  if ( mRemnant != refRevision.mRemnant )
  {
    if ( mRemnant == "" )
      return true;
    if ( refRevision.mRemnant == "" )
      return false;
    return (mRemnant < refRevision.mRemnant);
  }

  return false;
}

/*!
  @param refRevision - the revision that this revision is compared against
  @return true if this revision is earlier than or matches the reference revision
*/
bool Revision::operator<=(const Revision& refRevision) const
{
  return (*this < refRevision) || (*this == refRevision);
}

/*!
  @param refRevision - the revision that this revision is compared against
  @return true if this revision matches the reference revision
*/
bool Revision::operator==(const Revision& refRevision) const
{
  return mRevisionStr == refRevision.mRevisionStr;
}

/*!
  @param refRevision - the revision that this revision is compared against
  @return true if this revision is later than or matches the reference revision
*/
bool Revision::operator>=(const Revision& refRevision) const
{
  return !(*this < refRevision);
}

/*!
  @param refRevision - the revision that this revision is compared against
  @return true if this revision is later than the reference revision
*/
bool Revision::operator>(const Revision& refRevision) const
{
  return !(*this <= refRevision);
}

/*!
  @return the string representation of this revision
*/
string Revision::GetString() const
{
  return mRevisionStr;
}

/*!
  @return revision - the string representation of this revision
*/
void Revision::Set(string revision)
{
  mRevisionStr = revision;
  SetValues();
}

// Private---------------------------------------------------------------------

/*!
  Parses the revision string and loads the relevant date into the version parts, and sets the alpha, beta, and release candidate numbers.
*/
void Revision::SetValues()
{
  int pos, value, length;
  int alphaPos, betaPos, rcPos, prePos, stopPos;
  bool valueSet;

  mAlphaNumber = NOT_SET;
  mBetaNumber = NOT_SET;
  mRCNumber = NOT_SET;
  mVersionParts.clear();
  mRemnant = "";

  length = mRevisionStr.length();
  alphaPos = mRevisionStr.find("alpha");
  betaPos = mRevisionStr.find("beta");
  rcPos = mRevisionStr.find("rc");
  prePos = mRevisionStr.find("pre");
  if ( alphaPos != (int)string::npos )
  {
    stopPos = alphaPos;
    if ( alphaPos + 5 < length )
      sscanf(&(mRevisionStr.c_str()[alphaPos + 5]), "%d", &mAlphaNumber);
    else
      mAlphaNumber = MIN_VAL;
  }
  else if ( betaPos != (int)string::npos )
  {
    stopPos = betaPos;
    if ( betaPos + 4 < length )
      sscanf(&(mRevisionStr.c_str()[betaPos + 4]), "%d", &mBetaNumber);
    else
      mBetaNumber = MIN_VAL;
  }
  else if ( rcPos != (int)string::npos )
  {
    stopPos = rcPos;
    if ( rcPos + 2 < length )
      sscanf(&(mRevisionStr.c_str()[rcPos + 2]), "%d", &mRCNumber);
    else
      mRCNumber = MIN_VAL;
  }
  else if ( prePos != (int)string::npos )
  {
    stopPos = prePos;
    if ( prePos + 3 < length )
    {
      /* "pre" is considered synonymous with "rc", so use mRCNumber */
      sscanf(&(mRevisionStr.c_str()[prePos + 3]), "%d", &mRCNumber);
    }
    else
      mRCNumber = MIN_VAL;
  }
  else
    stopPos = length;

  pos = 0;
  while ( pos < stopPos )
  {
    value = 0;
    valueSet = false;
    while ( (pos < stopPos) && (mRevisionStr[pos] >= '0') && (mRevisionStr[pos] <= '9') )
    {
      valueSet = true;
      value = value * 10 + mRevisionStr[pos] - '0';
      pos++;
    }
    if ( valueSet )
      mVersionParts.push_back(value);
    if ( (pos < stopPos) && (mRevisionStr[pos] != '.') &&
         (mRevisionStr[pos] != '-') )
    {
      mRemnant = mRevisionStr.substr(pos, stopPos - pos);
      break;
    }
    pos++;
  }
}
