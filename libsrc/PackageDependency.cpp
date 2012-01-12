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
#include "PackageDependency.h"
#include "Package.h"
#include "Labels.h"
#include "XMLUtils.h"

// Public----------------------------------------------------------------------

PackageDependency::PackageDependency()
{
  mPackageName = "";
  mType = "dynamic";
  mDependentName = "";
}

/*!
  @param clone - the package dependency to copy
*/
PackageDependency::PackageDependency(const PackageDependency& clone)
{
  mPackageName = clone.mPackageName;
  mMinRevision = clone.mMinRevision;
  mMaxRevision = clone.mMaxRevision;
  mCondition = clone.mCondition;
  mType = clone.mType;
  mDependentName = clone.mDependentName;
  mDependentRevision = clone.mDependentRevision;
}

/*!
  @param clone - the package dependency to copy
  @param minRevision - the dependency application's minimum allowed revision
  @param maxRevision - the dependency application's maximum allowed revision
*/
PackageDependency::PackageDependency(const PackageDependency& clone, string minRevision, string maxRevision)
{
  mPackageName = clone.mPackageName;
  mMinRevision.Set(minRevision);
  mMaxRevision.Set(maxRevision);
  mCondition = clone.mCondition;
  mType = clone.mType;
  mDependentName = clone.mDependentName;
  mDependentRevision = clone.mDependentRevision;
}

/*!
  @param name - the dependency application's name
  @param minRevision - the dependency application's minimum allowed revision
  @param maxRevision - the dependency application's maximum allowed revision
  @param type - the type of dependency ("static", "dynamic", or "install")
*/
PackageDependency::PackageDependency(string name, string minRevision, string maxRevision, string type)
{
  mPackageName = name;
  mMinRevision.Set(minRevision);
  if ( maxRevision != "" )
    mMaxRevision.Set(maxRevision);
  mType = type;
  mDependentName = "";
}

PackageDependency::~PackageDependency()
{
}

/*!
  @param package - a package which potentially satisfies the dependency
  @return false if package is NULL or does not satisfy the dependency

  Determines whether the specified package satisfies this dependency.
*/
bool PackageDependency::IsSatisfiedBy(const Package* package) const
{
  if ( !package )
    return false;
  //! @todo Add support for blocking dependencies
  //! @todo Add support for "virtual" dependencies
  if ( package->GetName() != mPackageName )
    return false;
  if ( mMaxRevision.IsSet() && (package->GetRevision() > mMaxRevision) )
    return false;
  if ( package->GetRevision() >= mMinRevision )
    return true;
  return false;
}

/*!
  @param package - an installed package which potentially satisfies the dependency
  @return false if package is NULL or does not satisfy the dependency

  Determines whether the specified package satisfies this dependency.
*/
bool PackageDependency::IsSatisfiedBy(const InstallEntry * package) const
{
  if ( !package )
    return false;
  //! @todo Add support for blocking dependencies
  //! @todo Add support for "virtual" dependencies
  if ( package->GetName() != mPackageName )
    return false;
  if ( mMaxRevision.IsSet() && (package->GetRevision() > mMaxRevision) )
    return false;
  if ( package->GetRevision() >= mMinRevision )
    return true;
  return false;
}

/*!
  @param packageName - name of a package which potentially satisfies the dependency
  @param revision - revision of a package which potentially satisfies the dependency
  @return false if package name and revision does not satisfy the dependency

  Determines whether the specified package satisfies this dependency.
*/
bool PackageDependency::IsSatisfiedBy(const string& packageName, const Revision& revision) const
{
  //! @todo Add support for blocking dependencies
  //! @todo Add support for "virtual" dependencies
  if ( packageName != mPackageName )
    return false;
  if ( mMaxRevision.IsSet() && (revision > mMaxRevision) )
    return false;
  if ( revision >= mMinRevision )
    return true;
  return false;
}

/*!
  @return the name of the package required by this dependency
*/
string PackageDependency::GetName() const
{
  return mPackageName;
}

/*!
  @return human readable string describing this dependency
*/
string PackageDependency::GetString() const
{
  if ( !mMaxRevision.IsSet() )
    return "[ " + mCondition.GetString() + " ] " + mPackageName + " >= " + mMinRevision.GetString();
  if ( mMaxRevision == mMinRevision )
    return "[ " + mCondition.GetString() + " ] " + mPackageName + " == " + mMinRevision.GetString();
  return "[ " + mCondition.GetString() + " ] " + mPackageName + " " + mMinRevision.GetString() + " - " + mMaxRevision.GetString();
}

/*!
  @return string describing the dependency relationship ("static", "dynamic", or "install")
*/
string PackageDependency::GetType() const
{
  return mType;
}

/*!
  @return string identifying the minimum required revision
*/
string PackageDependency::GetMinRevision() const
{
  return mMinRevision.GetString();
}

/*!
  @return string identifying the maximum required revision
*/
string PackageDependency::GetMaxRevision() const
{
  return mMaxRevision.GetString();
}

/*!
  @param type - the dependency relationship ("static", "dynamic", or "install")
  @return true if type is valid
*/
bool PackageDependency::SetType(string type)
{
  if ( (type != DEP_TYPE_STATIC) && (type != DEP_TYPE_DYNAMIC) &&
       (type != DEP_TYPE_INSTALL) )
    return false;

  mType = type;
  return true;
}

/*!
  @param condition - the boolean expresssion which activates this dependency
*/
void PackageDependency::SetCondition(const BooleanExpression& condition)
{
  mCondition = condition;
}

/*!
  @return human readable string describing the package that relies on this dependency
*/
string PackageDependency::GetDependentString() const
{
  return mDependentName + " >= " + mDependentRevision.GetString() + " [ " + mCondition.GetString() + " ]";
}

/*!
  @return the name of the package that relies on this dependency
*/
string PackageDependency::GetDependentName() const
{
  return mDependentName;
}

/*!
  @param name - the name of the package that relies on this dependency
*/
void PackageDependency::SetDependentName(string name)
{
  mDependentName = name;
}

/*!
  @return the revision of the package that relies on this dependency
*/
string PackageDependency::GetDependentRevision() const
{
  return mDependentRevision.GetString();
}

/*!
  @param revision - the revision of the package that relies on this dependency
*/
void PackageDependency::SetDependentRevision(string revision)
{
  mDependentRevision.Set(revision);
}

/*!
  @param parentNode - the parent XML DOM node for the dependency node
  @return false if parentNode is NULL

  Saves the dependency data as a child node of the specified parent.
*/
bool PackageDependency::Save(xmlNodePtr parentNode)
{
  xmlNodePtr dependencyNode;

  if ( !parentNode )
    return false;

  if ( mDependentName == "" )
    dependencyNode = xmlNewNode(NULL,(const xmlChar*)DEPENDENCY_STR);
  else
    dependencyNode = xmlNewNode(NULL,(const xmlChar*)REVERSE_DEPENDENCY_STR);

  SetStringValue(dependencyNode, PACKAGE_NAME_STR, mPackageName);
  SetStringValue(dependencyNode, REVISION_STR, mMinRevision.GetString());
  if ( mMaxRevision.IsSet() )
    SetStringValue(dependencyNode, MAX_REVISION_STR, mMaxRevision.GetString());
  SetStringValue(dependencyNode, CONDITION_STR, mCondition.GetString());
  SetStringValue(dependencyNode, TYPE_STR, mType);
  SetStringValue(dependencyNode, DEPENDENT_NAME_STR, mDependentName);
  SetStringValue(dependencyNode, DEPENDENT_REVISION_STR, mDependentRevision.GetString());

  xmlAddChild(parentNode,dependencyNode);

  return true;
}

/*!
  @param dependencyNode - the XML DOM node containing the dependency data
  @return false if dependencyNode is NULL or there is a format error

  Loads the dependency data from the specified node.
*/
bool PackageDependency::Load(xmlNodePtr dependencyNode)
{
  string revision;
  string conditionStr;

  if ( !dependencyNode )
    return false;

  mPackageName = GetStringValue(dependencyNode, PACKAGE_NAME_STR);
  if ( mPackageName == "" )
    return false;

  revision = GetStringValue(dependencyNode, REVISION_STR);
  if ( revision == "" )
    return false;
  mMinRevision.Set(revision);

  revision = GetStringValue(dependencyNode, MAX_REVISION_STR);
  if ( revision != "" )
    mMaxRevision.Set(revision);

  conditionStr = GetStringValue(dependencyNode, CONDITION_STR);
  if ( !mCondition.ParseFromString(conditionStr) )
    return false;

  mType = GetStringValue(dependencyNode, TYPE_STR);
  if ( mType == "" )
    mType = "dynamic";

  mDependentName = GetStringValue(dependencyNode, DEPENDENT_NAME_STR);
  revision = GetStringValue(dependencyNode, DEPENDENT_REVISION_STR);
  if ( revision == "" )
  {
    if ( mDependentName != "" )
      return false;
  }
  else
    mDependentRevision.Set(revision);

  return true;
}

/*!
  @param features - the set of features used to evaluate the dependency condition
  @return true if the dependency condition is empty or evaluates to true (which indicates that this dependency cannot be ignored)
*/
bool PackageDependency::IsActive(const FeatureOptionVector& features) const
{
  if ( mCondition.IsEmpty() )
    return true;

  return mCondition.EvaluatesToTrue(features);
}

// Private---------------------------------------------------------------------
