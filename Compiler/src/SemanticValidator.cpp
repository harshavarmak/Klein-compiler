#include <iostream>
#include "header/Parser.h"
#include "header/Scanner.h"
#include "header/SemanticChecker.h"

int main(int argv, char* argc[])
{
	if (argv==1){
		cout << "Please insert a KLEIN file to scan for tokens." << endl;
		}

	try
	{
		Scanner Scanner("programs/fractionAddition.kln");
		Parser Parser(Scanner);

		Parser.parseProgram();
		ASTNode AST = Parser.getFinalASTNode();
		SemanticChecker SCheck = SemanticChecker(AST);
	}
	catch (const std::exception& e)
	{
		//Print Error Message
		cout << "IS VALID PROGRAM:  FALSE" << endl;
		cout << endl << e.what() << endl;
		return 1;
	}
	return 0;
}