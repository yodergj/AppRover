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
#ifndef K_APP_ROVER_H
#define K_APP_ROVER_H

#include <kmainwindow.h>
#include <klistview.h>
#include "AppRover.h"

//! Window class of the KDE AppRover GUI
class AppRoverKWindow : public KMainWindow
{
  Q_OBJECT
public:
  AppRoverKWindow(const char* title);
  ~AppRoverKWindow();
public slots:
  void Install();
  void Uninstall();
  void CreateNewPackage();
private:
  void FillOutPackageList();
  //! the view which lists the available packages
  KListView* mPackageList;
  //! the AppRover work horse object
  AppRover* mAppRover;
};

#endif
