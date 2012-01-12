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
#include <qlabel.h>
#include <qvbox.h>
#include "kWaitDialog.h"

// Public----------------------------------------------------------------------

/*!
  @param parent - the parent widget of this dialog
  @param name - the name of this dialog
*/
WaitDialog::WaitDialog(QWidget* parent, const char* name): KDialog(parent,name,true)
{
  QVBox* box = new QVBox(this);
  QLabel* label = new QLabel("Please wait",box);
  label->setIndent(20);
  mProgressAnimation = new KAnimWidget("kde",32,box);
  mProgressAnimation->setIcons("rotate\trotate_ccw");
}

/*!
  Makes this dialog visible and starts the animation.
*/
void WaitDialog::show()
{
  KDialog::show();
  mProgressAnimation->start();
}

/*!
  Hides this dialog and stops the animation.
*/
void WaitDialog::hide()
{
  mProgressAnimation->stop();
  KDialog::hide();
}

// Private---------------------------------------------------------------------
