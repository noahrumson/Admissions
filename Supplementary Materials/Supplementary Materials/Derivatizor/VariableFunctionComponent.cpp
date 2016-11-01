// Noah Rubin

#include "FunctionComponent.h"


FunctionComponentPtr VariableFunctionComponent::Differentiate() const
{
	return std::make_shared<ConstantFunctionComponent>(1.0);
}


int VariableFunctionComponent::AdditionOrder() const
{
	return 10;
}

int VariableFunctionComponent::MultiplicationOrder() const
{
	return ORDER_VARIABLE;
}


std::string VariableFunctionComponent::Print(bool parentheses) const
{
	return "x";
}