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
#ifndef PACKAGE_DEPENDENCY_H
#define PACKAGE_DEPENDENCY_H

#include <string>
#include <vector>
#include <libxml/tree.h>
#include "Revision.h"
#include "FeatureOption.h"
#include "BooleanExpression.h"

class Package;
class InstallEntry;

using std::string;
using std::vector;

//! Class responsible for representing a dependency between two packages
class PackageDependency
{
public:
  PackageDependency();
  PackageDependency(const PackageDependency& clone);
  PackageDependency(const PackageDependency& clone, string minRevision, string maxRevision);
  PackageDependency(string name, string minRevision, string maxRevision, string type);
  ~PackageDependency();
  bool IsSatisfiedBy(const Package * package) const;
  bool IsSatisfiedBy(const InstallEntry * package) const;
  bool IsSatisfiedBy(const string& packageName, const Revision& revision) const;
  string GetName() const;
  string GetString() const;
  string GetType() const;
  string GetMinRevision() const;
  string GetMaxRevision() const;
  bool SetType(string type);
  void SetCondition(const BooleanExpression& condition);
  string GetDependentString() const;
  string GetDependentName() const;
  void SetDependentName(string name);
  string GetDependentRevision() const;
  void SetDependentRevision(string revision);
  bool Save(xmlNodePtr parentNode);
  bool Load(xmlNodePtr dependencyNode);
  bool IsActive(const FeatureOptionVector& features) const;
private:
  //! The name of the package this dependency refers to
  string mPackageName;
  //! The minimum revision of the package which satisfies this dependency
  Revision mMinRevision;
  //! The maximum revision of the package which satisfies this dependency
  Revision mMaxRevision;
  //! The expression of feature conditions which activate this dependency
  BooleanExpression mCondition;
  //! The type of dependency (i.e. install tool, static lib, dynamic lib, etc)
  string mType;
  //! The name of the package which relies on this dependency (empty unless this is a reverse dependency)
  string mDependentName;
  //! The revision of the package which relies on this dependency (meaningless unless this is a reverse dependency)
  Revision mDependentRevision;
};

typedef vector<PackageDependency *> PackageDependencyVector;

#endif
