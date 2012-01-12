///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004-2009 Gabriel Yoder
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
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "AppRover.h"
#include "DirectoryUtils.h"
#include "Interpreter.h"

enum
{
  COMMAND_DEFAULT,
  COMMAND_FETCH,
  COMMAND_UNPACK,
  COMMAND_CONFIGURE,
  COMMAND_BUILD,
  COMMAND_INSTALL,
  COMMAND_UNINSTALL,
  COMMAND_CREATE,
  COMMAND_EDIT,
  COMMAND_ACTIVATE,
  COMMAND_DEACTIVATE,
  COMMAND_EXCLUDE,
  COMMAND_INCLUDE,
  COMMAND_PURIFY,
  COMMAND_CLEAN,
  COMMAND_SYNC,
  COMMAND_UPDATE
};

AppRover *rover = NULL;
Interpreter* gCommandInterpreter = NULL;
ShowProcessingStageFuncType gShowProcessingStage = NULL;

void ShowProcessingStage(std::string msg)
{
  char* term;

  /* Bail out if TERM isn't some sort of xterm */
  term = getenv("TERM");
  if ( !term || strncmp(term, "xterm", 5) )
    return;

  fprintf(stderr, "%c]0;%s%c", 0x1B, msg.c_str(), 0x07);
  fflush(stderr);
}

bool GetYesNo(string prompt)
{
  string response;
  bool eof;

  while ( 1 )
  {
    printf("%s\n",prompt.c_str());
    response = Readline(stdin,eof);
    if ( response != "" )
    {
      if ( (response[0] == 'y') || (response[0] == 'Y') )
        return true;
      if ((response[0] == 'n') || (response[0] == 'N') )
        return false;
    }
  }

  return false;
}

string GetWord(string prompt)
{
  string response;
  bool eof;

  while ( 1 )
  {
    printf("%s\n", prompt.c_str());
    response = Readline(stdin, eof);
    if ( response.find_first_of(" \t") == string::npos )
      return response;
  }

  return "";
}

bool EditLine(string& line)
{
  FILE* fp;
  char* environmentValue;
  string editor, result;
  bool eof;

  environmentValue = getenv("EDITOR");
  if ( environmentValue )
    editor = environmentValue;
  else
    editor = "vi";

  fp = fopen(".AppRover.tmp", "w");
  if ( !fp )
  {
    printf("Unable to write temporary file\n");
    return false;
  }
  fputs(line.c_str(), fp);
  fclose(fp);

  if ( gCommandInterpreter->Interpret(editor + " .AppRover.tmp") )
  {
    fp = fopen(".AppRover.tmp", "r");
    if ( !fp )
    {
      printf("Couldn't read file - something wacky happened\n<Press a key to continue>\n");
      getchar();
      return false;
    }
    result = Readline(fp, eof);
    Readline(fp, eof);
    fclose(fp);
    if ( !eof )
    {
      printf("Invalid input: Only one line allowed\n<Press a key to continue>\n");
      getchar();
      return false;
    }

    line = result;
    return true;
  }

  return false;
}

void SetSlot(Package* package)
{
  bool eof;
  string input;

  if ( !package )
    return;

  printf("Current slot value is: %s\n",package->GetSlot().c_str());
  printf("Enter slot value [""]:\n");
  input = Readline(stdin,eof);
  if ( input.find_first_of(" \t") == string::npos )
    package->SetSlot(input);
  else
  {
    printf("Invalid slot, using default value\n");
    package->SetSlot("");
  }
}

void SetBinaryRevision(Package* package)
{
  int revision = 1;
  bool readError;

  if ( !package )
    return;

  printf("Current binary compatability revision is: %d\n",
      package->GetBinaryCompatabilityRevision());
  printf("Enter binary compatability revision [1]:\n");
  revision = ReadInt(stdin, readError);
  if ( readError )
  {
    printf("Revision must be an integer, using default value\n");
    revision = 1;
  }
  package->SetBinaryCompatabilityRevision(revision);
}

void DeclareFeatures(Package* package)
{
  bool done = false;
  bool eof, readError;
  string input;
  int i, numFeatures, command;
  FeatureOption* feature;

  if ( !package )
    return;

  while ( !done )
  {
    printf("Current features:\n");
    numFeatures = package->GetNumFeatures();
    for (i=0; i<numFeatures; i++)
    {
      feature = package->GetFeature(i);
      if ( feature->IsEnabled() )
        printf("+");
      else
        printf("-");
      printf("%s\n",feature->GetName().c_str());
    }
    printf("\n1 Add feature\n");
    printf("2 Remove feature\n");
    printf("3 Toggle activation state\n");
    printf("4 Done\n");
    command = ReadInt(stdin,readError);
    if ( !readError )
    {
      if ( command == 1 )
      {
        printf("Enter feature name:\n");
        input = Readline(stdin,eof);
        if ( (input != "") &&
            (input.find_first_of(" \t") == string::npos) )
        {
          feature = new FeatureOption();
          feature->SetName(input);
          package->AddFeature(feature);
        }
        else
          printf("Invalid feature name\n");
      }
      else if ( command == 2 )
      {
        printf("Enter feature name:\n");
        input = Readline(stdin,eof);
        package->RemoveFeature(input);
      }
      else if ( command == 3 )
      {
        printf("Enter feature name:\n");
        input = Readline(stdin,eof);
        if ( (input != "") &&
            (input.find_first_of(" \t") == string::npos) )
        {
          numFeatures = package->GetNumFeatures();
          for (i=0; i<numFeatures; i++)
          {
            feature = package->GetFeature(i);
            if ( feature->GetName() == input )
            {
              if ( feature->IsEnabled() )
                feature->Disable();
              else
                feature->Enable();
              break;
            }
          }
        }
      }
      else if ( command == 4 )
        done = true;
    }
  }
}

void SetDependencies(Package* package)
{
  bool done = false;
  bool eof, readError;
  bool inputOK;
  string input;
  int i, numDependencies, command;
  string conditionStr, name, minRevision, maxRevision, type;
  PackageDependency* dependency;
  BooleanExpression condition;

  if ( !package )
    return;

  while ( !done )
  {
    printf("Current dependencies:\n");
    numDependencies = package->GetNumDependencies();
    for (i=0; i<numDependencies; i++)
      printf("%s [%s]\n",package->GetDependency(i)->GetString().c_str(),package->GetDependency(i)->GetType().c_str());
    printf("\n1 Add dependency\n");
    printf("2 Remove dependency\n");
    printf("3 Done\n");
    command = ReadInt(stdin, readError);
    if ( !readError )
    {
      if ( command == 1 )
      {
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter option condition for this dependency [<none>]:\n");
          conditionStr = Readline(stdin, eof);
          if ( condition.ParseFromString(conditionStr) )
            inputOK = true;
          else
            printf("Invalid condition expression\n");
        }
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter dependency package name:\n");
          name = Readline(stdin, eof);
          if ( (name != "") &&
	       (name.find_first_of(" \t") == string::npos) )
            inputOK = true;
        }
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter dependency package minimum revision:\n");
          minRevision = Readline(stdin, eof);
          if ( (minRevision != "") &&
	       (minRevision.find_first_of(" \t") == string::npos) )
            inputOK = true;
        }
        printf("Enter dependency package maximum revision (optional):\n");
        maxRevision = Readline(stdin, eof);
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter dependency relationship (\"static\", \"dynamic\", or \"install\"):\n");
          type = Readline(stdin, eof);
          if ( (type == "static") || (type == "dynamic") || (type == "install") )
            inputOK = true;
        }
        dependency = new PackageDependency(name, minRevision, maxRevision, type);
        dependency->SetCondition(condition);
        package->AddDependency(dependency);
        dependency = NULL;
      }
      else if ( command == 2 )
      {
        printf("Enter dependency name:\n");
        input = Readline(stdin, eof);
        package->RemoveDependency(input);
      }
      else if ( command == 3 )
        done = true;
    }
  }
}

void SetSources(Package* package)
{
  bool done = false;
  bool eof, readError;
  bool inputOK;
  string input;
  int i, numFiles, command, fileNumber;
  string conditionStr, location, fileContents;
  SourceFile* file;
  BooleanExpression condition;

  if ( !package )
    return;

  while ( !done )
  {
    printf("Current source files:\n");
    numFiles = package->GetNumSourceFiles();
    for (i = 0; i < numFiles; i++)
    {
      printf("%d (%s) %s\n", i+1,
          package->GetSourceFile(i)->GetCondition().GetString().c_str(),
          package->GetSourceFile(i)->GetLocation().c_str());
    }
    printf("\n1 Add remote source file\n");
    printf("2 Remove source file\n");
    printf("3 Change source remote location or embedded filename\n");
    printf("4 Add embedded source file\n");
    printf("5 Change source file conditions\n");
    printf("6 Done\n");
    command = ReadInt(stdin, readError);
    if ( !readError )
    {
      if ( command == 1 )
      {
        file = new SourceFile();
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter option condition for this source file [<none>]:\n");
          conditionStr = Readline(stdin, eof);
          if ( condition.ParseFromString(conditionStr) )
          {
            inputOK = true;
            file->SetCondition(condition);
          }
          else
            printf("Invalid condition expression\n");
        }
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter file location:\n");
          location = Readline(stdin, eof);
          if ( file->SetLocation(location) )
            inputOK = true;
          else
            printf("Invalid file location\n");
        }
        package->AddSourceFile(file);
        file = NULL;
      }
      else if ( command == 2 )
      {
        printf("Enter file number:\n");
        fileNumber = ReadInt(stdin, readError) - 1;
        if ( !readError )
          package->RemoveSourceFile(fileNumber);
      }
      else if ( command == 3 )
      {
        if ( numFiles == 0 )
          printf("No files to edit\n");
        else
        {
          if ( numFiles == 1 )
            fileNumber = 0;
          else
          {
            printf("Enter file number:\n");
            fileNumber = ReadInt(stdin, readError) - 1;
            if ( readError )
              fileNumber = -1;
          }
          if ( fileNumber != -1 )
          {
            file = package->GetSourceFile(fileNumber);
            inputOK = false;
            while ( !inputOK && file )
            {
              if ( file->IsEmbedded() )
              {
                printf("Enter embedded filename:\n");
                location = Readline(stdin, eof);

                if ( file->SetLocation(location) )
                  inputOK = true;
                else
                  printf("Invalid filename\n");
              }
              else
              {
                location = file->GetLocation();
                if ( EditLine(location) && file->SetLocation(location) )
                  inputOK = true;
                else
                  printf("Invalid file location\n");
              }
            }
          }
        }
      }
      else if ( command == 4 )
      {
        file = new SourceFile();
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter option condition for this source file [<none>]:\n");
          conditionStr = Readline(stdin, eof);
          if ( condition.ParseFromString(conditionStr) )
          {
            inputOK = true;
            file->SetCondition(condition);
          }
          else
            printf("Invalid condition expression\n");
        }
        inputOK = false;
        while ( !inputOK )
        {
          printf("Enter local source filename:\n");
          location = Readline(stdin, eof);
          if ( FileExists(location) &&
               file->SetLocation(GetFilenamePart(location)) )
            inputOK = true;
          else
            printf("Invalid file\n");

          fileContents = ReadFile(location.c_str());
          file->SetContents(fileContents);
        }
        package->AddSourceFile(file);
        file = NULL;
      }
      else if ( command == 5 )
      {
        printf("Enter file number:\n");
        fileNumber = ReadInt(stdin, readError) - 1;
        if ( !readError )
        {
	  file = package->GetSourceFile(fileNumber);
          inputOK = false;
          while ( !inputOK && file )
          {
            printf("Enter option condition for this source file [<none>]:\n");
            conditionStr = Readline(stdin,eof);
            if ( condition.ParseFromString(conditionStr) )
            {
              inputOK = true;
              file->SetCondition(condition);
            }
            else
              printf("Invalid condition expression\n");
          }
        }
      }
      else if ( command == 6 )
        done = true;
    }
  }
}

bool SetPackageInstructions(int commandType, Package* package)
{
  FILE* fp;
  bool eof;
  string input;
  char* environmentValue;
  string editor;

  environmentValue = getenv("EDITOR");
  if ( environmentValue )
    editor = environmentValue;
  else
    editor = "vi";

  fp = fopen(".AppRover.tmp","w");
  if ( !fp )
  {
    printf("Unable to write temporary file\n");
    return false;
  }
  switch (commandType)
  {
    case COMMAND_UNPACK:
      package->DumpUnpackInstructionsToFile(fp);
      package->ClearUnpackInstructions();
      break;
    case COMMAND_CONFIGURE:
      package->DumpConfigureInstructionsToFile(fp);
      package->ClearConfigureInstructions();
      break;
    case COMMAND_BUILD:
      package->DumpBuildInstructionsToFile(fp);
      package->ClearBuildInstructions();
      break;
    case COMMAND_UNINSTALL:
      package->DumpUninstallInstructionsToFile(fp);
      package->ClearUninstallInstructions();
      break;
    case COMMAND_INSTALL:
    default:
      package->DumpInstallInstructionsToFile(fp);
      package->ClearInstallInstructions();
      break;
  }
  fclose(fp);
  if ( gCommandInterpreter->Interpret(editor + " .AppRover.tmp") )
  {
    fp = fopen(".AppRover.tmp","r");
    if ( fp )
    {
      input = Readline(fp, eof);
      while ( !eof )
      {
        switch (commandType)
        {
          case COMMAND_UNPACK:
            package->AddUnpackInstruction(input);
            break;
          case COMMAND_CONFIGURE:
            package->AddConfigureInstruction(input);
            break;
          case COMMAND_BUILD:
            package->AddBuildInstruction(input);
            break;
          case COMMAND_UNINSTALL:
            package->AddUninstallInstruction(input);
            break;
          case COMMAND_INSTALL:
          default:
            package->AddInstallInstruction(input);
            break;
        }
        input = Readline(fp,eof);
      }
      gCommandInterpreter->Interpret("rm -f .AppRover.tmp");
      return true;
    }
  }
  return false;
}

bool ModifyPackage(Package* package)
{
  bool done = false;
  bool readError;
  int input;

  if ( !package )
    return false;

  while ( !done )
  {
    printf("1 Set slot\n");
    printf("2 Set binary compatability revision\n");
    printf("3 Declare features\n");
    printf("4 Set dependencies\n");
    printf("5 Set sources\n");
    printf("6 Set unpacking instructions\n");
    printf("7 Set configuration instructions\n");
    printf("8 Set build instructions\n");
    printf("9 Set install instructions\n");
    printf("10 Set uninstall instructions\n");
    printf("11 Done\n");
    input = ReadInt(stdin,readError);
    if ( !readError )
    {
      switch (input)
      {
        case 1:
          SetSlot(package);
          break;
        case 2:
          SetBinaryRevision(package);
          break;
        case 3:
          DeclareFeatures(package);
          break;
	case 4:
	  SetDependencies(package);
	  break;
        case 5:
          SetSources(package);
          break;
        case 6:
          SetPackageInstructions(COMMAND_UNPACK, package);
          break;
        case 7:
          SetPackageInstructions(COMMAND_CONFIGURE, package);
          break;
        case 8:
          SetPackageInstructions(COMMAND_BUILD, package);
          break;
        case 9:
          SetPackageInstructions(COMMAND_INSTALL, package);
          break;
        case 10:
          SetPackageInstructions(COMMAND_UNINSTALL, package);
          break;
        case 11:
          done = true;
          break;
        default:
          break;
      }
    }
  }

  return true;
}

bool CreatePackage(string packageName)
{
  string revision, response, fileLocation, filename;
  Package* package = NULL;

  revision = GetWord("Enter version number for " + packageName);

  package = rover->GetPackage(packageName,revision);
  if ( package )
  {
    printf("Revision %s %s already exists, use edit to modify it.\n",
        packageName.c_str(),revision.c_str());
    return false;
  }

  package = rover->GetPrecedingPackage(packageName,revision);
  if ( package )
  {
    if ( !GetYesNo("Would you like to use revision " + package->GetRevision().GetString() + " as a template?") )
      package = NULL;
  }

  if ( package )
    package = new Package(package,revision);
  else
  {
    package = new Package();
    package->SetName(packageName);
    package->SetRevision(revision);

    if ( GetYesNo("Would you like to use a standard UNIX build template?") )
    {
      SourceFile* file = new SourceFile;
      PackageDependency* dependency;
      int dotPos;
      string dirName, configOptions;
      bool eof;

      fileLocation = GetWord("Enter source file location:");
      file->SetLocation(fileLocation);
      package->AddSourceFile(file);
      filename = GetFilenamePart(fileLocation);

      dependency = new PackageDependency("binutils","2.14","","install");
      package->AddDependency(dependency);
      dependency = new PackageDependency("gcc","3.3.4","","install");
      package->AddDependency(dependency);
      dependency = new PackageDependency("make","3.80","","install");
      package->AddDependency(dependency);
      dependency = new PackageDependency("glibc","2.3.3","","dynamic");
      package->AddDependency(dependency);

      package->AddUnpackInstruction("tar -xvf " + filename);

      dotPos = filename.find(".tar");
      if ( dotPos == (int)string::npos )
        dotPos = filename.find(".tgz");
      if ( dotPos == (int)string::npos )
        dirName = filename;
      else
        dirName = filename.substr(0,dotPos);

      printf("Enter parameters to ./configure [optional]:\n");
      configOptions = Readline(stdin,eof);

      package->AddConfigureInstruction("cd " + dirName);
      package->AddConfigureInstruction("./configure " + configOptions);

      package->AddBuildInstruction("cd " + dirName);
      package->AddBuildInstruction("make");

      package->AddInstallInstruction("cd " + dirName);
      package->AddInstallInstruction("make install || exit 1");
      package->AddInstallInstruction("ldconfig");
    }
    else if ( GetYesNo("Would you like to use a cmake build template?") )
    {
      SourceFile* file = new SourceFile;
      PackageDependency* dependency;
      int dotPos;
      string dirName, configOptions;

      fileLocation = GetWord("Enter source file location:");
      file->SetLocation(fileLocation);
      package->AddSourceFile(file);
      filename = GetFilenamePart(fileLocation);

      dependency = new PackageDependency("binutils","2.14","","install");
      package->AddDependency(dependency);
      dependency = new PackageDependency("gcc","3.3.4","","install");
      package->AddDependency(dependency);
      dependency = new PackageDependency("make","3.80","","install");
      package->AddDependency(dependency);
      dependency = new PackageDependency("cmake","2.0.0","","install");
      package->AddDependency(dependency);
      dependency = new PackageDependency("glibc","2.3.3","","dynamic");
      package->AddDependency(dependency);

      package->AddUnpackInstruction("tar -xvf " + filename);
      package->AddUnpackInstruction("mkdir -p build");

      dotPos = filename.find(".tar");
      if ( dotPos == (int)string::npos )
        dotPos = filename.find(".tgz");
      if ( dotPos == (int)string::npos )
        dirName = filename;
      else
        dirName = filename.substr(0,dotPos);

      package->AddConfigureInstruction("cd build");
      package->AddConfigureInstruction("cmake ../" + dirName + " -DCMAKE_INSTALL_PREFIX:PATH='/usr'");

      package->AddBuildInstruction("cd build");
      package->AddBuildInstruction("make");

      package->AddInstallInstruction("cd build");
      package->AddInstallInstruction("make install || exit 1");
      package->AddInstallInstruction("ldconfig");
    }
  }

  if ( !ModifyPackage(package) )
    return false;
  rover->AddNewPackage(package);
  return true;
}

bool EditPackage(string packageName)
{
  string revision;
  Package* package = NULL;

  revision = GetWord("Enter version number for " + packageName);

  package = rover->GetPackage(packageName,revision);
  if ( !package )
  {
    printf("Unknown package or revision %s %s\n",packageName.c_str(),revision.c_str());
    return false;
  }
  if ( !ModifyPackage(package) )
    return false;
  rover->UpdatePackageChecksums(package);
  return package->Save();
}

void ActivateFeature(string featureName, bool state)
{
  int i, numPackages;
  string response;
  vector<string> packageNames;
  vector<string> packagesToReinstall;
  ActionDescriptionVector actionList;

  packageNames = rover->ActivateFeature(featureName,state);
  numPackages = packageNames.size();
  for (i=0; i<numPackages; i++)
  {
    if ( GetYesNo("Would you like to apply this change to " + packageNames[i] + "?") )
    {
      packagesToReinstall.push_back(packageNames[i]);
      rover->ActivateFeatureForPackage(packageNames[i],featureName,state);
    }
  }
  numPackages = packagesToReinstall.size();
  for (i=0; i<numPackages; i++)
  {
    //! @todo need to check and handle failure
    rover->Install(packagesToReinstall[i],"",false,actionList,STAGE_INSTALL,true);
  }
}

void PrintFeatures(ActionDescription* action)
{
  int i, numFeatures;
  const FeatureOption* feature = NULL;

  numFeatures = action->GetNumFeatures();
  for (i=0; i<numFeatures; i++)
  {
    feature = action->GetFeature(i);
    if ( feature->IsEnabled() )
      printf(" +");
    else
      printf(" -");
    printf("%s",feature->GetName().c_str());
  }
}

void PrintSyntax()
{
  printf("Usage: AppRover [options] [command] <package> [<package> ...]\n");
  printf("Options:\n");
  printf("\t-c <filename> - specify the configuration file (default AppRover.cfg)\n");
  printf("\t-p - print the actions that will be performed without executing\n");
  printf("\t-? - print this message\n");
  printf("Commands:\n");
  printf("\tfetch - downloads all package files\n");
  printf("\tunpack - halt installation after performing unpack stage\n");
  printf("\tconfigure - halt installation after performing configure stage\n");
  printf("\tbuild - halt installation after performing build stage\n");
  printf("\tinstall - (default) perform all steps to install packages\n");
  printf("\tcreate -  create a new package or package revision\n");
  printf("\tedit -  modify an existring package\n");
}

int main(int argc, char *argv[])
{
  int retcode;
  string configFilename = APP_ROVER_DIR;
  int command = COMMAND_DEFAULT;
  bool pretend = false;
  ActionDescriptionVector actionList;
  int i, numActions, pos;
  string resultStr, packageStr, revisionStr;

  configFilename += "AppRover.cfg";
  while ((retcode = getopt(argc,argv, "c:p?")) != -1)
  {
    switch (retcode)
    {
      case 'c':
        configFilename = optarg;
        break;
      case 'p':
        pretend = true;
        break;
      case '?':
      default:
        PrintSyntax();
        exit(1);
    }
  }

  if (optind == argc)
  {
    PrintSyntax();
    exit(1);
  }

  rover = new AppRover(configFilename);
  gCommandInterpreter = new Interpreter();
  gShowProcessingStage = ShowProcessingStage;
  if ( !strcmp(argv[optind],"fetch") )
  {
    command = COMMAND_FETCH;
    optind++;
  }
  else if ( !strcmp(argv[optind],"unpack") )
  {
    command = COMMAND_UNPACK;
    optind++;
  }
  else if ( !strcmp(argv[optind],"configure") )
  {
    command = COMMAND_CONFIGURE;
    optind++;
  }
  else if ( !strcmp(argv[optind],"build") )
  {
    command = COMMAND_BUILD;
    optind++;
  }
  else if ( !strcmp(argv[optind],"install") )
  {
    command = COMMAND_INSTALL;
    optind++;
  }
  else if ( !strcmp(argv[optind],"uninstall") )
  {
    command = COMMAND_UNINSTALL;
    optind++;
  }
  else if ( !strcmp(argv[optind],"create") )
  {
    command = COMMAND_CREATE;
    optind++;
  }
  else if ( !strcmp(argv[optind],"edit") )
  {
    command = COMMAND_EDIT;
    optind++;
  }
  else if ( !strcmp(argv[optind],"activate") )
  {
    command = COMMAND_ACTIVATE;
    optind++;
  }
  else if ( !strcmp(argv[optind],"deactivate") )
  {
    command = COMMAND_DEACTIVATE;
    optind++;
  }
  else if ( !strcmp(argv[optind],"exclude") )
  {
    command = COMMAND_EXCLUDE;
    optind++;
  }
  else if ( !strcmp(argv[optind],"include") )
  {
    command = COMMAND_INCLUDE;
    optind++;
  }
  else if ( !strcmp(argv[optind],"purify") )
  {
    command = COMMAND_PURIFY;
    optind++;
  }
  else if ( !strcmp(argv[optind],"clean") )
  {
    command = COMMAND_CLEAN;
  }
  else if ( !strcmp(argv[optind],"sync") )
  {
    command = COMMAND_SYNC;
  }
  else if ( !strcmp(argv[optind],"update") )
  {
    command = COMMAND_UPDATE;
  }

  if (optind == argc)
  {
    PrintSyntax();
    exit(1);
  }

  while (optind < argc)
  {
    packageStr = argv[optind];
    pos = packageStr.find('=');
    if ( pos != (int)string::npos )
    {
      revisionStr = packageStr.substr(pos + 1);
      packageStr = packageStr.substr(0, pos);
    }
    else
      revisionStr = "";
    switch (command)
    {
      case COMMAND_CREATE:
	if ( pretend )
	  printf("Cannot use pretend with create command\n");
	else
	{
	  if ( !CreatePackage(packageStr) )
	    printf("Failed to create %s\n", packageStr.c_str());
	  else
	    printf("Package created for %s.\n", packageStr.c_str());
	}
	break;
      case COMMAND_EDIT:
	if ( pretend )
	  printf("Cannot use pretend with edit command\n");
	else
	{
	  if ( !EditPackage(packageStr) )
	    printf("Failed to edit %s\n",packageStr.c_str());
	}
	break;
      case COMMAND_SYNC:
	if ( pretend )
	  printf("Cannot use pretend with sync command\n");
	else
	{
          resultStr = rover->SynchronizeRepositories();
	  if ( resultStr == "" )
            printf("Successfully synchronized all repositories\n");
          else
	    printf("While synchronizing:\n%s\n",resultStr.c_str());
	}
	break;
      case COMMAND_ACTIVATE:
	if ( pretend )
	  printf("Cannot use pretend with activate command\n");
	else
          ActivateFeature(packageStr,true);
        break;
      case COMMAND_DEACTIVATE:
        if ( pretend )
          printf("Cannot use pretend with deactivate command\n");
        else
          ActivateFeature(packageStr,false);
        break;
      case COMMAND_EXCLUDE:
	if ( pretend )
	  printf("Cannot use pretend with exclude command\n");
	else
        {
          resultStr += rover->ExcludeFromUpdates(packageStr, true);
          if ( resultStr != "" )
            printf(resultStr.c_str());
        }
        break;
      case COMMAND_INCLUDE:
	if ( pretend )
	  printf("Cannot use pretend with include command\n");
	else
        {
          resultStr = rover->ExcludeFromUpdates(packageStr, false);
          if ( resultStr != "" )
            printf(resultStr.c_str());
        }
        break;
      case COMMAND_PURIFY:
        if ( pretend )
          printf("Cannot use pretend with purify command\n");
        else
        {
          resultStr = rover->Purify(packageStr);
          if ( resultStr != "" )
            printf(resultStr.c_str());
        }
        break;
      case COMMAND_CLEAN:
        if ( !pretend )
          printf("Cleaning out partially installed packages, and packages which are needed no longer.\n");
        resultStr = rover->Clean(pretend,actionList);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully cleaned the system\n");
          else
          {
            printf("While cleaning:\n");
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
      case COMMAND_UPDATE:
        resultStr = rover->Update(pretend,actionList);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully updated the system\n");
          else
          {
            printf("While updating the system:\n");
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
      case COMMAND_FETCH:
        if ( !pretend )
          printf("Fetching %s\n",packageStr.c_str());
        resultStr = rover->Install(packageStr,revisionStr,pretend,actionList,STAGE_FETCH,true);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully fetched %s\n",packageStr.c_str());
          else
          {
            printf("While fetching %s:\n",packageStr.c_str());
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
      case COMMAND_UNPACK:
        if ( !pretend )
          printf("Unpacking %s\n",packageStr.c_str());
        resultStr = rover->Install(packageStr,revisionStr,pretend,actionList,STAGE_UNPACK,true);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully unpacked %s\n",packageStr.c_str());
          else
          {
            printf("While unpacking %s:\n",packageStr.c_str());
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
      case COMMAND_CONFIGURE:
        if ( !pretend )
          printf("Configuring %s\n",packageStr.c_str());
        resultStr = rover->Install(packageStr,revisionStr,pretend,actionList,STAGE_CONFIGURE,true);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully configured %s\n",packageStr.c_str());
          else
          {
            printf("While configuring %s:\n",packageStr.c_str());
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
      case COMMAND_BUILD:
        if ( !pretend )
          printf("Building %s\n",packageStr.c_str());
        resultStr = rover->Install(packageStr,revisionStr,pretend,actionList,STAGE_BUILD,true);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully built %s\n",packageStr.c_str());
          else
          {
            printf("While building %s:\n",packageStr.c_str());
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
      case COMMAND_UNINSTALL:
        if ( !pretend )
          printf("Uninstalling %s\n",packageStr.c_str());
        resultStr = rover->Uninstall(packageStr,pretend,actionList);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully uninstalled %s\n",packageStr.c_str());
          else
          {
            printf("While uninstalling %s:\n",packageStr.c_str());
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
      case COMMAND_INSTALL:
      default:
        if ( !pretend )
          printf("Installing %s\n",packageStr.c_str());
        resultStr = rover->Install(packageStr,revisionStr,pretend,actionList,STAGE_INSTALL,true);
        if ( !pretend )
        {
          if ( resultStr == "" )
            printf("Successfully installed %s\n",packageStr.c_str());
          else
          {
            printf("While installing %s:\n",packageStr.c_str());
            printf("%s\n",resultStr.c_str());
          }
        }
        break;
    }
    optind++;
  }
  if ( command == COMMAND_UNINSTALL )
  {
    if ( !pretend )
      printf("Cleaning out packages which are no longer needed.\n");
    resultStr = rover->Clean(pretend,actionList,true);
    if ( !pretend )
    {
      if ( resultStr == "" )
        printf("Successfully cleaned the system\n");
      else
      {
        printf("While cleaning:\n");
        printf("%s\n",resultStr.c_str());
      }
    }
  }
  if ( pretend )
  {
    numActions = actionList.size();
    for (i=0; i<numActions; i++)
    {
      switch (actionList[i]->GetAction())
      {
        case ACTION_FETCH:
          printf("[ F ] %s %s",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n");
          break;
        case ACTION_UNPACK:
          printf("[ P ] %s %s",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n");
          break;
        case ACTION_CONFIGURE:
          printf("[ C ] %s %s",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n");
          break;
        case ACTION_BUILD:
          printf("[ B ] %s %s",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n");
          break;
        case ACTION_INSTALL:
          printf("[ I ] %s %s",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n");
          break;
        case ACTION_REINSTALL:
          printf("[ R ] %s %s",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n");
          break;
        case ACTION_UPDATE:
          printf("[ U ] %s %s [%s]",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str(),actionList[i]->GetOldRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n");
          break;
        case ACTION_UNINSTALL:
          printf("[ D ] %s %s\n",actionList[i]->GetPackageName().c_str(),actionList[i]->GetOldRevision().c_str());
          break;
        case ACTION_CLEAN:
          printf("[ E ] %s %s\n",actionList[i]->GetPackageName().c_str(),actionList[i]->GetOldRevision().c_str());
          break;
        case ACTION_FAIL:
          printf("[ FAIL ] %s %s",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str());
          PrintFeatures(actionList[i]);
          printf("\n\t%s\n", actionList[i]->GetError().c_str());
          break;
        default:
          printf("[ Unknown ] %s %s [%s]\n",actionList[i]->GetPackageName().c_str(),actionList[i]->GetNewRevision().c_str(),actionList[i]->GetOldRevision().c_str());
      }
      delete actionList[i];
    }
  }

  gShowProcessingStage("");
  delete rover;
  return 0;
}
