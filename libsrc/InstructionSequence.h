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
#ifndef INSTRUCTION_SEQUENCE_H
#define INSTRUCTION_SEQUENCE_H

#include <string>
#include <vector>
#include <libxml/tree.h>
#include "FeatureOption.h"

class InstallEntry;

using std::string;
using std::vector;

//! Class for managing a series of command line instructions to be executed
class InstructionSequence
{
public:
  InstructionSequence();
  InstructionSequence(const InstructionSequence& refSequence);
  ~InstructionSequence();
  void AddInstruction(string instruction);
  void SetStartingDirectory(string directory);
  string GetStartingDirectory() const;
  void SetShell(string shell);
  bool Execute(InstallEntry* logEntry, int numProcessors, bool logFilesWritten) const;
  bool DumpToFile(FILE* file) const;
  bool Save(xmlNodePtr parentNode, string type) const;
  bool Load(xmlNodePtr instructionNode);
  bool IsEmpty() const;
  void ReplaceString(string oldString, string newString, bool includeStartingDir);
  void Clear();
  InstructionSequence& operator=(const InstructionSequence& src);
private:
  bool Translate(InstallEntry* logEntry, int numProcessors) const;
  bool HandleIfStatement(InstallEntry* logEntry, FILE* outputFile, bool skipLines, int& lineNumber) const;
  bool ConditionFulfilled(const FeatureOptionVector& variables, int lineNumber) const;
  //! The list of instructions in this sequence
  vector<string> mInstructions;
  //! The directory to change to before executing the instructions
  string mBaseDirectory;
  //! The shell used to interpret the instructions
  string mShell;
};

#endif
