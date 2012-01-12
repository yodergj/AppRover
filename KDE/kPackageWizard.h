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
#ifndef K_PACKAGE_WIZARD_H
#define K_PACKAGE_WIZARD_H

#include <kwizard.h>
#include <klineedit.h>
#include <klistview.h>
#include <qvbox.h>
#include <qtextedit.h>
#include "Package.h"

//! Wizard class used to create and edit packages within the KDE AppRover GUI
class PackageWizard : public KWizard
{
  Q_OBJECT
public:
  PackageWizard(Package* package, QWidget *parent=NULL, const char *name=NULL);
  void showPage(QWidget* page);
public slots:
  void NameChanged(const QString& newName);
  void RevisionChanged(const QString& newName);
  void AddDependency();
  void RemoveDependency();
  void AddFile();
  void RemoveFile();
  void InstallChanged();
  void UninstallChanged();
protected slots:
  virtual void next();
  virtual void back();
  virtual void accept();
private:
  //! Wizard page for editing things like name and revision
  QVBox *mNamePage;
  //! Wizard page for editing package dependencies
  QVBox *mDependencyPage;
  //! Wizard page for editing source files
  QVBox *mFilePage;
  //! Wizard page for setting unpacking instructions
  QVBox *mUnpackPage;
  //! Wizard page for setting configuration instructions
  QVBox *mConfigurePage;
  //! Wizard page for setting build instructions
  QVBox *mBuildPage;
  //! Wizard page for setting install instructions
  QVBox *mInstallPage;
  //! Wizard page for setting uninstall instructions
  QVBox *mUninstallPage;
  //! Edit box for package name
  KLineEdit *mNameEdit;
  //! Edit box for entering a new source file
  KLineEdit *mFileEdit;
  //! List view of all source files
  KListView *mFileList;
  //! Edit box for entering dependency name
  KLineEdit *mDependencyNameEdit;
  //! Edit box for entering dependency revision
  KLineEdit *mDependencyRevisionEdit;
  //! List view of all package dependencies
  KListView *mDependencyList;
  //! Edit box for unpacking instructions
  QTextEdit *mUnpackInstructions;
  //! Edit box for configuration instructions
  QTextEdit *mConfigureInstructions;
  //! Edit box for build instructions
  QTextEdit *mBuildInstructions;
  //! Edit box for install instructions
  QTextEdit *mInstallInstructions;
  //! Edit box for uninstall instructions
  QTextEdit *mUninstallInstructions;
  //! The package to create or edit
  Package *mPackage;
};

#endif
