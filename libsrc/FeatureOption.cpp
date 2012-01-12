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
#include "FeatureOption.h"
#include "Labels.h"
#include "XMLUtils.h"

// Public----------------------------------------------------------------------

FeatureOption::FeatureOption()
{
  mName = "";
  mEnabled = false;
}

/*!
  @param clone - the optional feature to copy
*/
FeatureOption::FeatureOption(const FeatureOption* clone)
{
  mName = clone->mName;
  mEnabled = clone->mEnabled;
}

FeatureOption::~FeatureOption()
{
}

/*!
  @param parentNode - the XML DOM node which will be the parent to the node for this feature
  @return false if parentNode is NULL
*/
bool FeatureOption::Save(xmlNodePtr parentNode) const
{
  xmlNodePtr featureNode;

  if ( !parentNode )
    return false;

  featureNode = xmlNewNode(NULL,(const xmlChar*)FEATURE_STR);
  SetStringValue(featureNode, FEATURE_NAME_STR, mName);
  SetBoolValue(featureNode, ENABLED_STR, mEnabled);

  xmlAddChild(parentNode, featureNode);

  return true;
}

/*!
  @param featureNode - the XML DOM node containing date for this feature
  @return false if featureNode is null or does not contain a feature name
*/
bool FeatureOption::Load(xmlNodePtr featureNode)
{
  if ( !featureNode )
    return false;

  mName = GetStringValue(featureNode, FEATURE_NAME_STR);
  if ( mName == "" )
    return false;

  mEnabled = GetBoolValue(featureNode, ENABLED_STR, false);
  return true;
}

/*!
  @return the name of this optional feature
*/
string FeatureOption::GetName() const
{
  return mName;
}

/*!
  @param name - the new name for this optional feature
*/
void FeatureOption::SetName(string name)
{
  mName = name;
}

/*!
  @return true if this feature is enabled
*/
bool FeatureOption::IsEnabled() const
{
  return mEnabled;
}

/*!
  Sets the state of this feature to enabled
*/
void FeatureOption::Enable()
{
  mEnabled = true;
}

/*!
  Sets the state of this feature to disabled
*/
void FeatureOption::Disable()
{
  mEnabled = false;
}

// Private---------------------------------------------------------------------
