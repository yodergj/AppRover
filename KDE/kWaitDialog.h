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
#ifndef K_WAIT_DIALOG_H
#define K_WAIT_DIALOG_H

#include <kanimwidget.h>
#include <kdialog.h>

//! Dialog used by KDE AppRover GUI to notify the user to wait for an action to complete
class WaitDialog : public KDialog
{
  Q_OBJECT
public:
  WaitDialog(QWidget* parent=0, const char* name=0);
public slots:
  virtual void show();
  virtual void hide();
private:
  //! Animation widget to show that the application has not frozen during the wait
  KAnimWidget* mProgressAnimation;
};

#endif
