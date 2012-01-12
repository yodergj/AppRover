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
#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include "kAppRover.h"
#include "kInterpreter.h"

Interpreter* gCommandInterpreter = NULL;
ShowProcessingStageFuncType gShowProcessingStage = NULL;

void ShowProcessingStage(std::string msg)
{
}

int main(int argc, char **argv)
{
  KAboutData *aboutData;

  aboutData = new KAboutData("kAppRover", I18N_NOOP("AppRover for KDE"),
      "1.0", I18N_NOOP("A package management utility"), KAboutData::License_BSD,
      "(C) 2003 Gabriel Yoder", "", "");
  aboutData->addAuthor("Gabriel Yoder", I18N_NOOP("Developer"),
      "gyoder@stny.rr.com", "");

  KCmdLineArgs::init(argc, argv, aboutData);
  KApplication app;

  gCommandInterpreter = new kInterpreter();
  gShowProcessingStage = ShowProcessingStage;  

  AppRoverKWindow *window = new AppRoverKWindow("AppRover for KDE");
  window->resize(320,200);
  app.setMainWidget(window);
  window->show();

  return app.exec();
}
