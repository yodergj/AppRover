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
#ifndef ACTION_DESCRIPTION_H
#define ACTION_DESCRIPTION_H

#include <string>
#include <vector>
#include "FeatureOption.h"

using std::string;
using std::vector;

//! Enum specifying all possible actions to perform on packages
typedef enum
{
  ACTION_NONE,
  ACTION_FETCH,
  ACTION_UNPACK,
  ACTION_CONFIGURE,
  ACTION_BUILD,
  ACTION_INSTALL,
  ACTION_REINSTALL,
  ACTION_UPDATE,
  ACTION_UNINSTALL,
  ACTION_CLEAN,
  ACTION_FAIL
} ActionType;

//! Class reponsible representing an action to be performed on a package
class ActionDescription
{
public:
  ActionDescription();
  ~ActionDescription();
  string GetOldRevision();
  void SetOldRevision(string revision);
  string GetNewRevision();
  void SetNewRevision(string revision);
  string GetPackageName();
  void SetPackageName(string name);
  string GetError();
  void SetError(string errorMsg);
  ActionType GetAction();
  void SetAction(ActionType action);
  int GetNumFeatures() const;
  const FeatureOption* GetFeature(int featureNumber) const;
  void SetFeatures(const FeatureOptionVector& features);
  bool operator==(const ActionDescription& refAction) const;
private:
  //! the already installed revision of the affected application
  string mOldRevision;
  //! the revision to be [partially] installed
  string mNewRevision;
  //! the name of the affected application
  string mPackageName;
  //! the problem with processing the request
  string mErrorMessage;
  //! the action that will be performed on the application
  ActionType mAction;
  //! the list of optional features relevant to the application
  FeatureOptionVector mFeatures;
};

//! Vector class for ActionDescriptions which prevents duplicate entries
class ActionDescriptionVector : public vector<ActionDescription *>
{
public:
  void push_back(ActionDescription* element);
};

#endif
