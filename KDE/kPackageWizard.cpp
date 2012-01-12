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
#include <kpushbutton.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include "kPackageWizard.h"

// Public----------------------------------------------------------------------

/*!
  Creates a package wizard to set the values of the specified package.
*/
PackageWizard::PackageWizard(Package* package, QWidget *parent,
                             const char *name): KWizard(parent,name)
{
  mPackage = package;

  // Page 1
  mNamePage = new QVBox(this);
  QHBox* nameHBox = new QHBox(mNamePage);
  QLabel* nameLabel = new QLabel(nameHBox);
  nameLabel->setText("Package Name");
  mNameEdit = new KLineEdit(nameHBox);
  connect(mNameEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(NameChanged(const QString &)));
  QHBox* revisionHBox = new QHBox(mNamePage);
  QLabel* revisionLabel = new QLabel(revisionHBox);
  revisionLabel->setText("Version");
  KLineEdit* revisionEdit = new KLineEdit(revisionHBox);
  connect(revisionEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(RevisionChanged(const QString &)));
  addPage(mNamePage,"Package Name");
  setHelpEnabled(mNamePage,false);

  // Page 2
  mDependencyPage = new QVBox(this);
  QHBox* dependencyHBox = new QHBox(mDependencyPage);
  QVBox* dependencyVBox = new QVBox(dependencyHBox);

  QHBox* dependencyNameHBox = new QHBox(dependencyVBox);
  QLabel* dependencyNameLabel = new QLabel(dependencyNameHBox);
  dependencyNameLabel->setText("Package");
  mDependencyNameEdit = new KLineEdit(dependencyNameHBox);

  QHBox* dependencyRevisionHBox = new QHBox(dependencyVBox);
  QLabel* dependencyRevisionLabel = new QLabel(dependencyRevisionHBox);
  dependencyRevisionLabel->setText("Revision");
  mDependencyRevisionEdit = new KLineEdit(dependencyRevisionHBox);

  KPushButton *addDependencyButton = new KPushButton(dependencyHBox);
  connect(addDependencyButton, SIGNAL(clicked()), this, SLOT(AddDependency()));
  addDependencyButton->setText("Add");
  QHBox* depListHBox = new QHBox(mDependencyPage);
  mDependencyList = new KListView(depListHBox);
  mDependencyList->addColumn("Package");
  mDependencyList->addColumn("Revision");
  KPushButton *removeDependencyButton = new KPushButton(depListHBox);
  connect(removeDependencyButton, SIGNAL(clicked()), this, SLOT(RemoveDependency()));
  removeDependencyButton->setText("Remove");
  addPage(mDependencyPage,"Dependencies (optional)");
  setHelpEnabled(mDependencyPage,false);

  // Page 3
  mFilePage = new QVBox(this);
  QHBox* fileHBox = new QHBox(mFilePage);
  QLabel* fileLabel = new QLabel(fileHBox);
  fileLabel->setText("File to fetch");
  mFileEdit = new KLineEdit(fileHBox);
  KPushButton *addFileButton = new KPushButton(fileHBox);
  connect(addFileButton, SIGNAL(clicked()), this, SLOT(AddFile()));
  addFileButton->setText("Add");
  QHBox* listHBox = new QHBox(mFilePage);
  mFileList = new KListView(listHBox);
  mFileList->addColumn("Files");
  KPushButton *removeFileButton = new KPushButton(listHBox);
  connect(removeFileButton, SIGNAL(clicked()), this, SLOT(RemoveFile()));
  removeFileButton->setText("Remove");
  addPage(mFilePage,"Required Files");
  setHelpEnabled(mFilePage,false);

  // Page 4
  mUnpackPage = new QVBox(this);
  mUnpackInstructions = new QTextEdit(mUnpackPage);
  addPage(mUnpackPage,"Unpacking Instructions (optional)");
  setHelpEnabled(mUnpackPage,false);

  // Page 5
  mConfigurePage = new QVBox(this);
  mConfigureInstructions = new QTextEdit(mConfigurePage);
  addPage(mConfigurePage,"Configuration Instructions (optional)");
  setHelpEnabled(mConfigurePage,false);

  // Page 6
  mBuildPage = new QVBox(this);
  mBuildInstructions = new QTextEdit(mBuildPage);
  addPage(mBuildPage,"Building Instructions (optional)");
  setHelpEnabled(mBuildPage,false);

  // Page 7
  mInstallPage = new QVBox(this);
  mInstallInstructions = new QTextEdit(mInstallPage);
  connect(mInstallInstructions, SIGNAL(textChanged()),
          this, SLOT(InstallChanged()));
  addPage(mInstallPage,"Installation Instructions");
  setHelpEnabled(mInstallPage,false);

  // Page 8
  mUninstallPage = new QVBox(this);
  mUninstallInstructions = new QTextEdit(mUninstallPage);
  connect(mUninstallInstructions, SIGNAL(textChanged()),
          this, SLOT(UninstallChanged()));
  addPage(mUninstallPage,"Uninstallation Instructions");
  setHelpEnabled(mUninstallPage,false);
}

/*!
  @param page - the page to show

  Overrides QWizard::showPage in order to properly intialize focus and disable the "next" button when applicable.
*/
void PackageWizard::showPage(QWidget* page)
{
  QWizard::showPage(page);

  if ( page == mNamePage )
  {
    mNameEdit->setFocus();
    if ( (mPackage->GetName() == "") || (mPackage->GetRevision().GetString() == "") )
      nextButton()->setEnabled(false);
  }
  else if ( page == mDependencyPage )
  {
    mDependencyNameEdit->setFocus();
  }
  else if ( page == mFilePage )
  {
    mFileEdit->setFocus();
    if ( !mFileList->firstChild() )
      nextButton()->setEnabled(false);
  }
  else if ( page == mUnpackPage )
  {
    mUnpackInstructions->setFocus();
  }
  else if ( page == mConfigurePage )
  {
    mConfigureInstructions->setFocus();
  }
  else if ( page == mBuildPage )
  {
    mBuildInstructions->setFocus();
  }
  else if ( page == mInstallPage )
  {
    mInstallInstructions->setFocus();
    if ( mInstallInstructions->text() == "" )
      nextButton()->setEnabled(false);
  }
  else if ( page == mUninstallPage )
  {
    mUninstallInstructions->setFocus();
    if ( mUninstallInstructions->text() == "" )
      nextButton()->setEnabled(false);
  }
}

/*!
  @param newName - the value of the package name

  Sets the name of the package and enables/disables the "next" button when applicable.
*/
void PackageWizard::NameChanged(const QString& newName)
{
  mPackage->SetName(newName.latin1());
  if ( (newName != "") && (mPackage->GetRevision().GetString() != "") )
    nextButton()->setEnabled(true);
  else
    nextButton()->setEnabled(false);
}

/*!
  @param newRevision - the value of the package revision

  Sets the revision of the package and enables/disables the "next" button when applicable.
*/
void PackageWizard::RevisionChanged(const QString& newRevision)
{
  mPackage->SetRevision(newRevision.latin1());
  if ( (mPackage->GetName() != "") && (newRevision != "") )
    nextButton()->setEnabled(true);
  else
    nextButton()->setEnabled(false);
}

/*!
  Uses the data from the dependency edit box to add a new dependency to the dependency list widget.
*/
void PackageWizard::AddDependency()
{
  if ( (mDependencyNameEdit->text() != "") &&
       (mDependencyRevisionEdit->text() != "") )
  {
    new QListViewItem(mDependencyList,mDependencyNameEdit->text(),
                      mDependencyRevisionEdit->text());
    mDependencyNameEdit->clear();
    mDependencyRevisionEdit->clear();
  }
}

/*!
  Removes the selected dependency from the dependency list widget.
*/
void PackageWizard::RemoveDependency()
{
  QListViewItem* item = mDependencyList->selectedItem();
  if ( item )
    delete item;
}

/*!
  Uses the data from the filename edit box to add a new filename to the file list widget.  After adding the file, the "next" button is enabled.
*/
void PackageWizard::AddFile()
{
  new QListViewItem(mFileList,mFileEdit->text());
  mFileEdit->clear();
  nextButton()->setEnabled(true);
}

/*!
  Removes the selected file from the file list widget and disables the "next" button if the list becomes empty.
*/
void PackageWizard::RemoveFile()
{
  QListViewItem* item = mFileList->selectedItem();
  if ( item )
  {
    delete item;
    if ( !mFileList->firstChild() )
      nextButton()->setEnabled(false);
  }
}

/*!
  Called when the install instructions are changed in order to enable or disable the "next" button.
*/
void PackageWizard::InstallChanged()
{
  if ( mInstallInstructions->text() == "" )
    nextButton()->setEnabled(false);
  else
    nextButton()->setEnabled(true);
}

/*!
  Called when the uninstall instructions are changed in order to enable or disable the "finish" button.
*/
void PackageWizard::UninstallChanged()
{
  if ( mUninstallInstructions->text() == "" )
    finishButton()->setEnabled(false);
  else
    finishButton()->setEnabled(true);
}

// Protected-------------------------------------------------------------------

/*!
  Overrides KWizard::next in order to save the data from each page to the package.
*/
void PackageWizard::next()
{
  QWidget* page = currentPage();

  //! @todo : use paragraph option for text to separately add instructions

  if ( page == mDependencyPage )
  {
    mPackage->ClearDependencies();
    QListViewItem* currentDep = mDependencyList->firstChild();
    while (currentDep)
    {
      //! @todo update package wizard for dependency types
      mPackage->AddDependency(
          new PackageDependency(currentDep->text(0),currentDep->text(1),"","dynamic"));
      currentDep = currentDep->nextSibling();
    }
  }
  else if ( page == mFilePage )
  {
    mPackage->ClearSourceFiles();
    QListViewItem* currentFile = mFileList->firstChild();
    while (currentFile)
    {
      mPackage->AddSourceFile(new SourceFile(currentFile->text(0).latin1()));
      currentFile = currentFile->nextSibling();
    }
  }
  else if ( page == mUnpackPage )
  {
    mPackage->ClearUnpackInstructions();
    mPackage->AddUnpackInstruction(mUnpackInstructions->text().latin1());
  }
  else if ( page == mConfigurePage )
  {
    mPackage->ClearConfigureInstructions();
    mPackage->AddConfigureInstruction(mConfigureInstructions->text().latin1());
    if ( mConfigureInstructions->text() == "" )
      setAppropriate(mBuildPage,false);
    else
      setAppropriate(mBuildPage,true);
  }
  else if ( page == mBuildPage )
  {
    mPackage->ClearBuildInstructions();
    mPackage->AddBuildInstruction(mBuildInstructions->text().latin1());
  }
  else if ( page == mInstallPage )
  {
    mPackage->ClearInstallInstructions();
    mPackage->AddInstallInstruction(mInstallInstructions->text().latin1());
  }

  KWizard::next();
}

/*!
  Overrides KWizard::back in order to clear the data from each page and the package.
*/
void PackageWizard::back()
{
  QWidget* page = currentPage();

  if ( page == mDependencyPage )
  {
    mPackage->ClearDependencies();
    mDependencyNameEdit->clear();
    mDependencyRevisionEdit->clear();
    mDependencyList->clear();
  }
  else if ( page == mFilePage )
  {
    mPackage->ClearSourceFiles();
    mFileEdit->clear();
    mFileList->clear();
  }
  else if ( page == mUnpackPage )
  {
    mPackage->ClearUnpackInstructions();
    mUnpackInstructions->setText("");
  }
  else if ( page == mConfigurePage )
  {
    mPackage->ClearConfigureInstructions();
    mConfigureInstructions->setText("");
  }
  else if ( page == mBuildPage )
  {
    mPackage->ClearBuildInstructions();
    mBuildInstructions->setText("");
  }
  else if ( page == mInstallPage )
  {
    mPackage->ClearInstallInstructions();
    mInstallInstructions->setText("");
  }
  else if ( page == mUninstallPage )
  {
    mPackage->ClearUninstallInstructions();
    mUninstallInstructions->setText("");
  }

  KWizard::back();
}

/*!
  Overrides KWizard::accept in order to save the data from the last page to the package.
*/
void PackageWizard::accept()
{
  mPackage->ClearUninstallInstructions();
  mPackage->AddUninstallInstruction(mUninstallInstructions->text().latin1());
  QDialog::accept();
}

// Private---------------------------------------------------------------------
