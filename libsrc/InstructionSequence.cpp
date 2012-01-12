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
#include "InstructionSequence.h"
#include "Interpreter.h"
#include "InstallEntry.h"
#include "Labels.h"
#include "XMLUtils.h"
#include "BooleanExpression.h"
#include "DirectoryUtils.h"
#include <string.h>

// Public----------------------------------------------------------------------

InstructionSequence::InstructionSequence()
{
  mBaseDirectory = ".";
  mShell = "/bin/sh";
}

/*!
  @param refSequence - instruction sequence to copy

  Creates an InstructionSequence object which is an exact copy of refSequence.
*/
InstructionSequence::InstructionSequence(const InstructionSequence& refSequence)
{
  int i, numInstructions;

  mBaseDirectory = refSequence.mBaseDirectory;
  mShell = refSequence.mShell;

  numInstructions = refSequence.mInstructions.size();
  for (i=0; i<numInstructions; i++)
    mInstructions.push_back(refSequence.mInstructions[i]);
}

InstructionSequence::~InstructionSequence()
{
}

/*!
  @param instruction - an instruction to process

  Adds an instruction to the end of the list of instructions.
*/
void InstructionSequence::AddInstruction(string instruction)
{
  mInstructions.push_back(instruction);
}

/*!
  @param directory - the directory to change to before executing
*/
void InstructionSequence::SetStartingDirectory(string directory)
{
  mBaseDirectory = directory;
}

/*!
  @return the directory to change to before executing
*/
string InstructionSequence::GetStartingDirectory() const
{
  return mBaseDirectory;
}

/*!
  @param shell - the shell used to interpret the instructions
*/
void InstructionSequence::SetShell(string shell)
{
  mShell = shell;
}

/*!
  @param logEntry - the log entry to receive error messages
  @param numProcessors - number of processors in system (for parallelization)
  @param logFilesWritten - should the names of files written be logged
  @return false if an instruction fails

  Executes the stored sequence of instructions.  If any instruction fails, it immediately returns false.  Eventually error messages will be logged.
*/
bool InstructionSequence::Execute(InstallEntry* logEntry, int numProcessors, bool logFilesWritten) const
{
  string loggingStr = "";

  if ( !Translate(logEntry, numProcessors) )
    return false;

  if ( logFilesWritten )
    loggingStr = "LD_PRELOAD=libwatcher.so APPROVER_WATCHFILE=" + mBaseDirectory + "/.AppRoverFileLog ";

  if ( !gCommandInterpreter->Interpret(loggingStr + mBaseDirectory + "/.AppRoverInstructions.sh 2> " + mBaseDirectory + "/.AppRoverError.log",true) )
  {
    string errorMsg;
    FILE* errorFile = NULL;
    bool eof = false;

    errorMsg = "";
    errorFile = fopen((mBaseDirectory+"/.AppRoverError.log").c_str(),"r");
    if ( errorFile )
    {
      while ( !eof )
        errorMsg += Readline(errorFile,eof) + "\n";
      fclose(errorFile);
    }
    else
      errorMsg = "Unknown error";

    logEntry->SetErrorMessage(errorMsg);

    return false;
  }
  logEntry->SetErrorMessage("");
  return true;
}

/*!
  @param file - the file to receive the contents of these instructions
  @return false if the file is NULL
*/
bool InstructionSequence::DumpToFile(FILE* file) const
{
  int i;

  if ( !file )
    return false;

  for (i=0; i<(int)mInstructions.size(); i++)
    fprintf(file,"%s\n",mInstructions[i].c_str());

  return true;
}

/*!
  @param parentNode - the xml node to serve as the parent of the instruction node
  @param type - string identifying what these instructions are used for
  @return false if parentNode is null

  Saves the instructions as child nodes of the specified parent node.
*/
bool InstructionSequence::Save(xmlNodePtr parentNode, string type) const
{
  int i, numInstructions;
  xmlNodePtr instructionNode;
  xmlNodePtr textNode;

  if ( !parentNode )
    return false;

  instructionNode = xmlNewNode(NULL,(const xmlChar*)INSTRUCTION_STR);
  SetStringValue(instructionNode,SHELL_STR,mShell);
  SetStringValue(instructionNode,INSTRUCTION_TYPE_STR,type);
  numInstructions = mInstructions.size();
  for (i=0; i<numInstructions; i++)
  {
    textNode = xmlNewText((const xmlChar*)(mInstructions[i]+"\n").c_str());
    xmlAddChild(instructionNode,textNode);
  }
  xmlAddChild(parentNode,instructionNode);
  return true;
}

/*!
  @param instructionNode - the xml node which is the parent of the text nodes containing the actual instructions
  @return false if parentNode is NULL

  Loads the instructions stored as child text nodes of the specified instruction node.
*/
bool InstructionSequence::Load(xmlNodePtr instructionNode)
{
  xmlNodePtr currentNode;
  string instructionText;
  string shellText;
  string instruction;
  int eolPos, lastEolPos;

  if ( !instructionNode )
    return false;

  shellText = GetStringValue(instructionNode,SHELL_STR);
  if ( shellText != "" )
    SetShell(shellText);
  currentNode = instructionNode->children;
  while ( currentNode )
  {
    if ( strcmp((const char *)currentNode->name,"text") )
      return false;
    instructionText = (char *)currentNode->content;
    eolPos = instructionText.find('\n');
    if ( eolPos != (int)string::npos )
    {
      instruction = instructionText.substr(0, eolPos);
      mInstructions.push_back(instruction);
      lastEolPos = eolPos;
      eolPos = instructionText.find('\n', lastEolPos + 1);
      while ( eolPos != (int)string::npos )
      {
	instruction = instructionText.substr(lastEolPos + 1, eolPos - lastEolPos - 1);
        mInstructions.push_back(instruction);
        lastEolPos = eolPos;
        eolPos = instructionText.find('\n', lastEolPos + 1);
      }
      instruction = instructionText.substr(lastEolPos + 1);
      if ( instruction != "" )
        mInstructions.push_back(instruction);
    }
    else
      mInstructions.push_back(instructionText);
    currentNode = currentNode->next;
  }
  return true;
}

/*!
  Deletes all of the stored instructions.
*/
void InstructionSequence::ReplaceString(string oldString, string newString, bool includeStartingDir)
{
  int i, numInstructions, strPos;

  numInstructions = mInstructions.size();
  for (i = 0; i < numInstructions; i++)
  {
    strPos = mInstructions[i].find(oldString);
    while ( strPos != (int)string::npos )
    {
      mInstructions[i].replace(strPos, oldString.length(), newString);
      strPos = mInstructions[i].find(oldString, strPos + newString.length());
    }
  }

  if ( includeStartingDir )
  {
    strPos = mBaseDirectory.find(oldString);
    while ( strPos != (int)string::npos )
    {
      mBaseDirectory.replace(strPos, oldString.length(), newString);
      strPos = mBaseDirectory.find(oldString, strPos + newString.length());
    }
  }
}

/*!
  Deletes all of the stored instructions.
*/
void InstructionSequence::Clear()
{
  mInstructions.clear();
}

/*!
  @return true there are no stored instructions
*/
bool InstructionSequence::IsEmpty() const
{
  return ((int)mInstructions.size() == 0);
}

/*!
  @param src - the instruction sequence to be copied

  Copies an InstructionSequence
*/
InstructionSequence& InstructionSequence::operator=(const InstructionSequence& src)
{
  int i, numInstructions;

  mBaseDirectory = src.mBaseDirectory;
  mShell = src.mShell;

  mInstructions.clear();
  numInstructions = src.mInstructions.size();
  for (i=0; i<numInstructions; i++)
    mInstructions.push_back(src.mInstructions[i]);
  return *this;
}

// Private---------------------------------------------------------------------

/*!
  @param logEntry - the log entry to receive any error messages
  @param numProcessors - number of processors in system (for parallelization)
  @return true if these instructions were successfully used to create a script file
*/
bool InstructionSequence::Translate(InstallEntry* logEntry, int numProcessors) const
{
  int i;
  FILE* scriptFile = NULL;
  string filename = mBaseDirectory + "/.AppRoverInstructions.sh";
  int ifPos, startPos;

  scriptFile = fopen(filename.c_str(), "w");
  if ( !scriptFile )
  {
    logEntry->SetErrorMessage("Unable to create script file");
    return false;
  }

  //! @todo Add support for constants for things like package name
  fprintf(scriptFile,"#!%s\n",mShell.c_str());
  fprintf(scriptFile,"cd %s\n",mBaseDirectory.c_str());
  fprintf(scriptFile,"if [ -f \"/etc/profile\" ]\n");
  fprintf(scriptFile,"then\n");
  fprintf(scriptFile,"source /etc/profile\n");
  fprintf(scriptFile,"fi\n");
  fprintf(scriptFile,"if [ -f \"/etc/bashrc\" ]\n");
  fprintf(scriptFile,"then\n");
  fprintf(scriptFile,"source /etc/bashrc\n");
  fprintf(scriptFile,"fi\n");
  for (i = 0; i < (int)mInstructions.size(); i++)
  {
    ifPos = mInstructions[i].find("@if@");
    if ( ifPos != (int)string::npos )
    {
      if ( !HandleIfStatement(logEntry,scriptFile,false,i) )
        return false;
    }
    else
    {
      startPos = mInstructions[i].find_first_not_of(" \t");
      if ( startPos != (int)string::npos )
      {
        /* Look for "make" commands so we can insert the -j option */
        if ( (mInstructions[i].substr(startPos, 4) == "make") &&
             ( (startPos + 4 == (int)mInstructions[i].size()) ||
               (mInstructions[i][startPos + 4] == ' ') ||
               (mInstructions[i][startPos + 4] == '\t') ) )
        {
          fprintf(scriptFile,"make -j%d %s\n", numProcessors, mInstructions[i].c_str() + startPos + 4);
        }
        else
          fprintf(scriptFile,"%s\n",mInstructions[i].c_str());
      }
    }
  }

  fclose(scriptFile);
  if ( !gCommandInterpreter->Interpret("chmod a+x " + filename) )
  {
    logEntry->SetErrorMessage("Unable to make script file executable");
    return false;
  }

  return true;
}

/*!
  @param logEntry - the log entry to receive any error messages
  @param outputFile - the script file to receive the translated instructions
  @param skipLines - if true, indicates that we are processing a section of an if statement which will not be executed due to feature conditions
  @param lineNumber - the current line in the list of instructions
  @return true if successfully parsed
*/
bool InstructionSequence::HandleIfStatement(InstallEntry* logEntry, FILE* outputFile, bool skipLines, int& lineNumber) const
{
  int ifPos, elsePos, endifPos;
  bool skipToElse;

  if ( lineNumber == (int)mInstructions.size() - 1 )
  {
    logEntry->SetErrorMessage("Parse error at end of instructions");
    return false;
  }

  if ( ConditionFulfilled(logEntry->GetFeatures(),lineNumber) )
  {
    lineNumber++;
    while ( lineNumber < (int)mInstructions.size() )
    {
      ifPos = mInstructions[lineNumber].find("@if@");
      if ( ifPos != (int)string::npos )
      {
        if ( !HandleIfStatement(logEntry,outputFile,skipLines,lineNumber) )
          return false;
      }
      else
      {
        endifPos = mInstructions[lineNumber].find("@endif@");
        if ( endifPos != (int)string::npos )
          return true;
        elsePos = mInstructions[lineNumber].find("@else@");
        if ( elsePos != (int)string::npos )
          skipLines = true;
        else
        {
          if ( !skipLines )
            fprintf(outputFile,"%s\n",mInstructions[lineNumber].c_str());
        }
      }
      lineNumber++;
    }
  }
  else
  {
    lineNumber++;
    skipToElse = true;
    while ( lineNumber < (int)mInstructions.size() )
    {
      ifPos = mInstructions[lineNumber].find("@if@");
      if ( ifPos != (int)string::npos )
      {
        if ( !HandleIfStatement(logEntry,outputFile,skipLines||skipToElse,lineNumber) )
          return false;
      }
      else
      {
        endifPos = mInstructions[lineNumber].find("@endif@");
        if ( endifPos != (int)string::npos )
          return true;
        elsePos = mInstructions[lineNumber].find("@else@");
        if ( elsePos != (int)string::npos )
          skipToElse = false;
        else
        {
          if ( !skipLines && !skipToElse )
            fprintf(outputFile,"%s\n",mInstructions[lineNumber].c_str());
        }
      }
      lineNumber++;
    }
  }

  if ( lineNumber < (int)mInstructions.size() )
    logEntry->SetErrorMessage("Parse error at '" + mInstructions[lineNumber] + "'");
  else
    logEntry->SetErrorMessage("Parse error at end of file - unterminated @if@ statement");
  return false;
}

/*!
  @param variables - list of optional features and their activation states
  @param lineNumber - the current instruction line to evaluate
  @return true if a condition string was successfully parsed and evaluated to true
*/
bool InstructionSequence::ConditionFulfilled(const FeatureOptionVector& variables, int lineNumber) const
{
  int ifPos;
  BooleanExpression condition;
  string conditionStr;

  ifPos = mInstructions[lineNumber].find("@if@");
  if ( ifPos == (int)string::npos )
    conditionStr = mInstructions[lineNumber];
  else
    conditionStr = mInstructions[lineNumber].substr(ifPos + 4);
  //! @todo decide how to handle this case
  if ( !condition.ParseFromString(conditionStr) )
    return false;

  return condition.EvaluatesToTrue(variables);
}
