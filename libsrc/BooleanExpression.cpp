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
#include <deque>
#include "BooleanExpression.h"

using std::deque;

// Public----------------------------------------------------------------------

BooleanExpression::BooleanExpression()
{
  mDisplayString = "";
}

BooleanExpression::~BooleanExpression()
{
}

/*!
  @return string containing the expression in "normal" notation
*/
string BooleanExpression::GetString() const
{
  return mDisplayString;
}

/*!
  @param expression - a new boolean expression in "normal" notation
  @return true if the expression could be properly parsed

  Clears the current contents of the expression.  If the new expression can be parsed, it will be used, otherwise the expression is left empty.
*/
bool BooleanExpression::ParseFromString(string expression)
{
  mDisplayString = expression;
  mArgumentsInReversePolishNotation.clear();

  if ( expression == "" )
    return true;
  return ParseString(expression,true);
}

/*!
  @return true if the expression does not contain any elements
*/
bool BooleanExpression::IsEmpty() const
{
  return (mDisplayString == "");
}

/*!
  @param features - the list of optional feature states used to evaluate the value of the expression
  @return true if the expression evaluates to true
  @return false if the expression evaluates to false or is empty
*/
bool BooleanExpression::EvaluatesToTrue(const FeatureOptionVector& features) const
{
  int i, numArguments;
  deque<bool> stack;
  bool operand1, operand2;

  numArguments = mArgumentsInReversePolishNotation.size();

  if ( numArguments == 0 )
    return false;

  for (i=0; i<numArguments; i++)
  {
    if ( mArgumentsInReversePolishNotation[i] == "!" )
    {
      operand1 = stack[0];
      stack.pop_front();
      stack.push_front(!operand1);
    }
    else if ( mArgumentsInReversePolishNotation[i] == "&&" )
    {
      operand1 = stack[0];
      operand2 = stack[1];
      stack.pop_front();
      stack.pop_front();
      stack.push_front(operand1 && operand2);
    }
    else if ( mArgumentsInReversePolishNotation[i] == "||" )
    {
      operand1 = stack[0];
      operand2 = stack[1];
      stack.pop_front();
      stack.pop_front();
      stack.push_front(operand1 || operand2);
    }
    else
      stack.push_front(GetFeatureValue(features,mArgumentsInReversePolishNotation[i]));
  }
  return stack[0];
}

/*!
  @param src - the boolean expression to copy
  @return reference to this boolean expression
*/
BooleanExpression& BooleanExpression::operator=(const BooleanExpression& src)
{
  mDisplayString = src.mDisplayString;
  mArgumentsInReversePolishNotation.clear();
  mArgumentsInReversePolishNotation.insert(mArgumentsInReversePolishNotation.begin(),
      src.mArgumentsInReversePolishNotation.begin(),
      src.mArgumentsInReversePolishNotation.end());
  return *this;
}

// Private---------------------------------------------------------------------

/*!
  @param features - the list of optional feature states used to evaluate this expression
  @param feature - the name of an optional feature whose state is desired
  @return true if the feature is found in the list and is enabled
*/
bool BooleanExpression::GetFeatureValue(const FeatureOptionVector& features, string feature) const
{
  int i, numFeatures;

  numFeatures = features.size();
  for (i=0; i<numFeatures; i++)
  {
    if ( feature == features[i]->GetName() )
      return features[i]->IsEnabled();
  }
  //! @todo Decide how to handle this case.  Perhaps I will exit
  return false;
}

#define BINARY_OPERATION 0
#define UNARY_OPERATION 1
#define EXPRESSION 2
#define EMPTY 3

/*!
  @param expression - the remaining portion of an expression to be parsed
  @param isTopLevel - if true, indiciates that we are working with the entire boolean expression
  @return true if the relevant portion of the expression could be successfully parsed

  This function recursively parses an expression.  The top level call fails if any characters are left unparsed.
*/
bool BooleanExpression::ParseString(string expression, bool isTopLevel)
{
  string argument;
  int lastArgument = EMPTY;
  string lastBinaryOperation;
  string lastUnaryOperation;

  if ( expression == "" )
    return false;

  argument = PopArgument(expression);
  if ( (argument == "") || (argument == ")") )
    return false;

  while ( (argument != "") && (argument != ")") )
  {
    if ( argument == "(" )
    {
      if ( lastArgument == EXPRESSION )
        return false;
      if ( !ParseString(expression,false) )
        return false;
      lastArgument = EXPRESSION;
      if ( lastUnaryOperation != "" )
      {
        mArgumentsInReversePolishNotation.push_back(lastUnaryOperation);
        lastUnaryOperation = "";
      }
      if ( lastBinaryOperation != "" )
      {
        mArgumentsInReversePolishNotation.push_back(lastBinaryOperation);
        lastBinaryOperation = "";
      }
    }
    else if ( argument == "!" )
    {
      if ( (lastArgument == EXPRESSION) || ( lastUnaryOperation != "") )
        return false;
      lastUnaryOperation = argument;
      lastArgument = UNARY_OPERATION;
    }
    else if ( (argument == "||") || (argument == "&&") )
    {
      if ( (lastArgument != EXPRESSION) || ( lastBinaryOperation != "") )
        return false;
      lastBinaryOperation = argument;
      lastArgument = BINARY_OPERATION;
    }
    else
    {
      if ( lastArgument == EXPRESSION )
        return false;
      lastArgument = EXPRESSION;
      mArgumentsInReversePolishNotation.push_back(argument);
      if ( lastUnaryOperation != "" )
      {
        mArgumentsInReversePolishNotation.push_back(lastUnaryOperation);
        lastUnaryOperation = "";
      }
      if ( lastBinaryOperation != "" )
      {
        mArgumentsInReversePolishNotation.push_back(lastBinaryOperation);
        lastBinaryOperation = "";
      }
    }
    argument = PopArgument(expression);
  }
  return  (!isTopLevel || (PopArgument(expression) == ""));
}

/*!
  @param arguments - string of tokens separated by whitespace
  @return string containing the next token ("" if there are no token remaining)

  Modifies the input arguments to remove the leading token from the string and returns the token.
*/
string BooleanExpression::PopArgument(string& arguments) const
{
  int startPos, endPos;
  string result;

  if ( arguments == "" )
    return "";

  startPos = arguments.find_first_not_of(" \t\n\r");
  if ( startPos == (int)string::npos )
  {
    arguments = "";
    return "";
  }
  arguments = arguments.substr(startPos);
  if ( arguments[0] == '(' )
  {
    result = "(";
  }
  else if ( arguments[0] == ')' )
  {
    result = ")";
  }
  else if ( arguments[0] == '!' )
  {
    result = "!";
  }
  else if ( arguments[0] == '|' )
  {
    if ( (arguments.length() == 1) || (arguments[1] != '|') )
      result = "";
    else
      result = "||";
  }
  else if ( arguments[0] == '&' )
  {
    if ( (arguments.length() == 1) || (arguments[1] != '&') )
      result = "";
    else
      result = "&&";
  }
  else
  {
    endPos = arguments.find_first_of(" \t\n\r");
    if ( endPos == (int)string::npos )
      result = arguments;
    else
      result = arguments.substr(0, endPos);
  }
  arguments = arguments.substr(result.length());
  return result;
}
