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
#include "ActionDescription.h"

// Public----------------------------------------------------------------------

ActionDescription::ActionDescription()
{
  mAction = ACTION_NONE;
  mOldRevision = "";
  mNewRevision = "";
  mPackageName = "";
}

ActionDescription::~ActionDescription()
{
  int i, numFeatures;

  numFeatures = mFeatures.size();
  for (i=0; i<numFeatures; i++)
    delete mFeatures[i];
}

/*!
  @return string containing the currently installed revision of the affected application
*/
string ActionDescription::GetOldRevision()
{
  return mOldRevision;
}

/*!
  @param revision - the currently installed revision of the affected application
*/
void ActionDescription::SetOldRevision(string revision)
{
  mOldRevision = revision;
}

/*!
  @return string containing the revision to be [partially] installed for the affected application
*/
string ActionDescription::GetNewRevision()
{
  return mNewRevision;
}

/*!
  @param revision - the revision to be [partially] installed for the affected application
*/
void ActionDescription::SetNewRevision(string revision)
{
  mNewRevision = revision;
}

/*!
  @return the name of the affected application
*/
string ActionDescription::GetPackageName()
{
  return mPackageName;
}

/*!
  @param name - the name of the affected application
*/
void ActionDescription::SetPackageName(string name)
{
  mPackageName = name;
}

/*!
  @return a string describing the error with processing
*/
string ActionDescription::GetError()
{
  return mErrorMessage;
}

/*!
  @param name - a string describing the error with processing
*/
void ActionDescription::SetError(string errorMsg)
{
  mErrorMessage = errorMsg;
  mAction = ACTION_FAIL;
}

/*!
  @return the action to be performed on the affected application
*/
ActionType ActionDescription::GetAction()
{
  return mAction;
}

/*!
  @param action - the action to be performed on the affected application
*/
void ActionDescription::SetAction(ActionType action)
{
  mAction = action;
}

/*!
  @return the number of optional features relevant to the affected application
*/
int ActionDescription::GetNumFeatures() const
{
  return mFeatures.size();
}

/*!
  @param featureNumber - the index into the list of optional feature relevant to the affected application
  @return the feature option at the desired location
  @return NULL if the featureNumber is invalid
*/
const FeatureOption* ActionDescription::GetFeature(int featureNumber) const
{
  if ( (featureNumber < 0) || (featureNumber >= (int)mFeatures.size()) )
    return NULL;
  return mFeatures[featureNumber];
}

/*!
  @param features - the list of optional feature associated with the affected application
*/
void ActionDescription::SetFeatures(const FeatureOptionVector& features)
{
  int i, numFeatures;

  numFeatures = features.size();
  for (i=0; i<numFeatures; i++)
    mFeatures.push_back(new FeatureOption(features[i]));
}

/*!
  @param refAction - the action description to compare against
  @return true if package name, revisions, and action are the same
*/
bool ActionDescription::operator==(const ActionDescription& refAction) const
{
  if ( mPackageName != refAction.mPackageName )
    return false;
  if ( mOldRevision != refAction.mOldRevision )
    return false;
  if ( mNewRevision != refAction.mNewRevision )
    return false;
  return mAction == refAction.mAction;
}

/*!
  @param element - the action description to add to the vector

  Adds the element to the end of the vector.  If the element matches an element that is already in the vector, the new element is simply deleted.
*/
void ActionDescriptionVector::push_back(ActionDescription* element)
{
  int i, numElements;

  numElements = size();
  for (i=0; i<numElements; i++)
  {
    if ( *(*this)[i] == *element )
    {
      delete element;
      return;
    }
  }
  vector<ActionDescription *>::push_back(element);
}

// Private---------------------------------------------------------------------
