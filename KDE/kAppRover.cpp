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
#include <kapp.h>
#include <kaction.h>
#include <kconfig.h>
#include <kpushbutton.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qptrlist.h>
#include <string>
#include "kAppRover.h"
#include "kPackageWizard.h"
#include "Interpreter.h"

using std::string;

// Public----------------------------------------------------------------------

/*!
  Initializes the window and creates the AppRover object to handle the bulk of the processing.
*/
AppRoverKWindow::AppRoverKWindow(const char* title) : KMainWindow (0L, title)
{
  string appRoverDir = APP_ROVER_DIR;
  mAppRover = NULL;

  KStdAction::quit(this, SLOT(close()), actionCollection());
  new KAction("&Create new package", 0, this,
              SLOT(CreateNewPackage()), actionCollection(),
              "create_new_package");
  createGUI((appRoverDir+"KDE/kAppRoverui.rc").c_str());

  QVBox* vertLayout = new QVBox(this);
  setCentralWidget(vertLayout);
  mPackageList = new KListView(vertLayout);
  mPackageList->addColumn("Package");
  mPackageList->addColumn("Revision Installed");
  QHBox* horzLayout = new QHBox(vertLayout);
  KPushButton* installButton = new KPushButton(horzLayout);
  KPushButton* uninstallButton = new KPushButton(horzLayout);
  installButton->setText("Install");
  uninstallButton->setText("Uninstall");
  connect(installButton,SIGNAL(clicked()),this,SLOT(Install()));
  connect(uninstallButton,SIGNAL(clicked()),this,SLOT(Uninstall()));

  KConfig *config = kapp->config();
  config->setGroup("Settings");

  if ( !config->hasKey("AppRoverConfigDir") )
    config->writeEntry("AppRoverConfigDir",appRoverDir.c_str());
  if ( !config->hasKey("AppRoverConfigFile") )
    config->writeEntry("AppRoverConfigFile","AppRover.cfg");

  string cmd = "cd ";
  cmd += config->readEntry("AppRoverConfigDir",appRoverDir.c_str()).latin1();
  if ( gCommandInterpreter->Interpret(cmd) )
    mAppRover = new AppRover(config->readEntry("AppRoverConfigFile","AppRover.cfg").latin1());

  FillOutPackageList();
}

AppRoverKWindow::~AppRoverKWindow()
{
  if ( mAppRover )
    delete mAppRover;
}

/*!
  Installs the packages currently selected in the package list widget.
*/
void AppRoverKWindow::Install()
{
  int i, numItems;
  QPtrList<QListViewItem> items = mPackageList->selectedItems();
  QListViewItem* currentItem = NULL;
  ActionDescriptionVector actionList;

  numItems = items.count();
  for (i=0; i<numItems; i++)
  {
    currentItem = items.at(i);
    mAppRover->Install(currentItem->text(0).latin1(),"",false,actionList);
  }
  FillOutPackageList();
}

/*!
  Uninstalls the packages currently selected in the package list widget.
*/
void AppRoverKWindow::Uninstall()
{
  int i, numItems;
  QPtrList<QListViewItem> items = mPackageList->selectedItems();
  QListViewItem* currentItem = NULL;
  ActionDescriptionVector actionList;

  numItems = items.count();
  for (i=0; i<numItems; i++)
  {
    currentItem = items.at(i);
    mAppRover->Uninstall(currentItem->text(0).latin1(),false,actionList);
  }
  FillOutPackageList();
}

/*!
  Creates a new package by invoking a PackageWizard.
*/
void AppRoverKWindow::CreateNewPackage()
{
  Package* package = new Package();
  PackageWizard* wizard = new PackageWizard(package,this,"New Package Wizard");
  if ( wizard->exec() == QDialog::Accepted )
  {
    mAppRover->AddNewPackage(package,0);
    FillOutPackageList();
  }
  else
    delete wizard;
}

// Private---------------------------------------------------------------------

/*!
  Obtains a list of packages from the AppRover and uses it to populate the package list widget.
*/
void AppRoverKWindow::FillOutPackageList()
{
  //! @todo Clean this up so it doesn't do so many allocations / deallocations
  int i;
  int numPackages;

  mPackageList->clear();
  numPackages = mAppRover->NumPackages();
  for (i=0; i<numPackages; i++)
  {
    new QListViewItem(mPackageList,mAppRover->GetPackageName(i).c_str(),
                mAppRover->GetInstalledRevisionNumber(i).c_str());
  }
}
