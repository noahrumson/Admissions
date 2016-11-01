// Noah Rubin

#include <iostream>

#include "FunctionComponent.h"
#include "FunctionCallDerivative.h"
#include "Parser.h"
#include "simplify.h"

#define COLLEGE_BUILD 1	// See the file client.cpp

#if !COLLEGE_BUILD
#	include "client.h"
#endif

int main()
{
#if !COLLEGE_BUILD
	bool connected = Connect();
	if (!connected) {
		return 0;
	}
#endif
	std::cout << "Derivatizor by Noah Rubin\n";
	std::cout << "Enter a function of x to differentiate\n";
	std::string input;
	while (true) {
		std::getline(std::cin, input);
		FunctionComponentPtr func = ParseExpression(input);
		FunctionComponentPtr temp = func->SimplifyConstants();
		func = (temp) ? temp : func;
		if (!func) {
			std::cout << "Expression could not be parsed\n";
		}
		else {
			FunctionComponentPtr derivative = func->Differentiate();
			FunctionComponentPtr simp = derivative->Simplify(!FunctionComponent::DISTRIBUTE);
			simp = (simp) ? simp : derivative;
			simp->Sort();
			std::cout << "Derivative of f(x) = " + func->Print() + " is:\nf'(x) = " + simp->Print() << std::endl;
		}
	}
	return 0;
}