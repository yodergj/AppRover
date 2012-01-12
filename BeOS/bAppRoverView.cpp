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
#include <ListView.h>
#include <ScrollView.h>
#include "bAppRoverView.h"

/*!
  @param frame - the rectangle boundary of the view
  @param name - the name associated with this view
*/
AppRoverView::AppRoverView(BRect frame, char *name)
              :BView(frame, name, 0, B_WILL_DRAW)
{
  mPackageList = new BListView(frame,"packages",B_MULTIPLE_SELECTION_LIST);
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));
  mPackageList->AddItem(new BStringItem("temp stuff"));

  AddChild(new BScrollView("scroll_packages",mPackageList,
	B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS,
	false,true));
  /*
  AddChild(new BScrollView("scroll_packages",mPackageList,
	B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS,
	false,true));
	*/
}
