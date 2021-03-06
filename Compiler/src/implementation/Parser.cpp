#pragma once
#include "../header/Parser.h"
#include <algorithm>


Parser::Parser(Scanner& InScanner): ScannerVar(InScanner) {
	return;
}

void Parser::parseProgram()
{
	Stack.push(END_OF_STREAM);
	Stack.push(PROGRAM);
	StackValues StackTop = Stack.top();
	Token PeekedToken = ScannerVar.peek();
	StackValues PeekedTokenValue = mapFromScannerTokenToStackValue(PeekedToken);
	string StringDataForSemanticAction = "NULL";
	string IntDataForSemanticAction = "NULL";

	while (StackTop != END_OF_STREAM) {
		if (isSemanticAction(StackTop))
		{
			// If semantic action, call the visitor method which encapsulates the knowledge of how to modify
			// The stack.
			BuilderVisitor.accept(StackTop, SemanticStackVar, StringDataForSemanticAction, IntDataForSemanticAction);
			Stack.pop();
			StackTop = Stack.top();
			continue;
		}

		if (isTerminalValue(StackTop)) {
			if (StackTop == PeekedTokenValue)
			{
				// Update data to be stored in AST nodes.
				if (StackTop == INTEGER_LITERAL)
				{
					IntDataForSemanticAction = PeekedToken.getValue();
				}
				else {
					StringDataForSemanticAction = PeekedToken.getValue();
				}

				Stack.pop();
				ScannerVar.next();
			}
			else {
				string StackTopString = StackValuesPrintMap.find(StackTop)->second;
				string PeekedTokenSting = StackValuesPrintMap.find(PeekedTokenValue)->second;
				throw runtime_error("ERROR: Expected - " + PeekedTokenSting + ", Saw  - " + StackTopString);
			}
		}
		else {
			if (ParseTable.find(make_pair(StackTop, PeekedTokenValue)) != ParseTable.end()) {
				list<StackValues> Rule = ParseTable.find(make_pair(StackTop, PeekedTokenValue))->second;
				Stack.pop();
				addRuleToStack(Rule);
			}
			else {
				string StackTopString = StackValuesPrintMap.find(StackTop)->second;
				string PeekedTokenSting = StackValuesPrintMap.find(PeekedTokenValue)->second;
				throw runtime_error("ERROR: No Rule exists in the Parse Table for (" + StackTopString + ", " + PeekedTokenSting + ")");
			}
		}

		PeekedToken = ScannerVar.peek();
		PeekedTokenValue = mapFromScannerTokenToStackValue(PeekedToken);
		StackTop = Stack.top();
	}

	checkValidEndState(PeekedTokenValue);
}

bool Parser::isProgramValid()
{
	// This method is only for unit testing. It provides a public method to 
	// the Parser API so that we can run test accordingly.
	try
	{
		parseProgram();

		// valid if no exception thrown
		return true;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

ASTNode Parser::getFinalASTNode()
{
	return FinalASTNode;
}

bool Parser::isTerminalValue(StackValues value)
{
	list<StackValues>::iterator foundElement = find(TerminalValues.begin(), TerminalValues.end(), value);
	return foundElement != TerminalValues.end();
}

bool Parser::isSemanticAction(StackValues value)
{
	list<StackValues>::iterator foundElement = find(SemanticActions.begin(), SemanticActions.end(), value);
	return foundElement != SemanticActions.end();
}

StackValues Parser::mapFromScannerTokenToStackValue(Token InToken)
{
	switch (InToken.getTokenType())
	{
	case INTEGER:
		return INTEGER_LITERAL;
	case DATA_TYPE:
		return InToken.getValue() == "integer" ? INTEGER_DATA_TYPE : BOOLEAN_DATA_TYPE;
	case BOOLEAN:
		return BOOLEAN_LITERAL;
	case ARITHMETIC_OPERATOR:
		return mapArithmeticOperatorTokenToStackValue(InToken);
	case PRIMITIVE_KEYWORD:
		return FUNCTION;
	case PARENTHESIS:
		return InToken.getValue() == "(" ? LEFT_PAREN : RIGHT_PAREN;
	case COMMA:
		return COMMA_LITERAL;
	case COLON:
		return COLON_LITERAL;
	case IDENTIFIER:
		return InToken.getValue() == "print" ? PRINT : IDENTIFIER_LITERAL;
	case COMPARATOR:
		return InToken.getValue() == "<" ? LESS_THAN_OPERATOR : EQUAL_SIGN;
	case LOGICIAL_OPERATOR:
		return mapLogicalOperatorTokenToStackValue(InToken);
	case CONDITIONAL:
		return mapConditionalTokenToStackValue(InToken);
	case END_OF_FILE:
		return END_OF_STREAM;
	}

	throw runtime_error("ERROR: Parser got in a bad state attempting to map TokenType to StackValue. Token value - " + InToken.getValue());
}

StackValues Parser::mapArithmeticOperatorTokenToStackValue(Token InToken)
{
	if (InToken.getValue() == "+")
	{
		return PLUS_OPERATOR;
	}
	else if (InToken.getValue() == "-")
	{
		return MINUS_OPERATOR;
	}
	else if (InToken.getValue() == "/")
	{
		return DIVIDES_OPERATOR;
	}
	else
	{
		return MULTIPLY_OPERATOR;
	};
}

StackValues Parser::mapLogicalOperatorTokenToStackValue(Token InToken)
{
	if (InToken.getValue() == "and")
	{
		return AND;
	}
	else if (InToken.getValue() == "not")
	{
		return NOT;
	}
	else
	{
		return OR;
	};
}

StackValues Parser::mapConditionalTokenToStackValue(Token InToken)
{
	if (InToken.getValue() == "if")
	{
		return IF;
	}
	else if (InToken.getValue() == "then")
	{
		return THEN;
	}
	else
	{
		return ELSE;
	};
}

void Parser::addRuleToStack(list<StackValues> Rule)
{
	// Adds values to stack in reverse order
	while (!Rule.empty())
	{
		Stack.push(Rule.back());
		Rule.pop_back();
	}
}

void Parser::checkValidEndState(StackValues PeekedTokenValue)
{
	if (Stack.top() == END_OF_STREAM && PeekedTokenValue == END_OF_STREAM)
	{
		Stack.pop();
	}

	if (!Stack.isEmpty())
	{
		string StackTopString = StackValuesPrintMap.find(Stack.top())->second;
		throw runtime_error("ERROR: There are still values on the stack, but input stream has ended. Top of stack - " + StackTopString);
	}

	FinalASTNode = SemanticStackVar.pop();
	if (FinalASTNode.getAstNodeType() != PROGRAM_NODE_TYPE)
	{
		throw runtime_error("ERROR: The top of the semantic stack isn't a Program Node.");
	}
	if (!SemanticStackVar.isEmpty())
	{
		throw runtime_error("ERROR: There are still nodes left on the Semantic Stack!");
	}
}