// Noah Rubin

#include <algorithm>

#include "FunctionComponent.h"
#include "MathHelper.h"
#include "simplify.h"


FunctionComponentPtr AdditionFunctionComponent::Differentiate() const
{
	/*
		(f + g)' = f' + g'
		(f - g)' = f' - g'
	*/
	AdditionFunctionComponentPtr tree = std::make_shared<AdditionFunctionComponent>();
	for (const FunctionComponentPtr child : GetChildren()) {
		tree->AddChild(child->Differentiate());
	}
	return tree;
}

void AdditionFunctionComponent::AddInverseChild(const FunctionComponentPtr& child)
{
	if (child->IsConstant()) {
		AddChild(std::make_shared<ConstantFunctionComponent>(-std::static_pointer_cast<ConstantFunctionComponent>(child)->GetValue()));
	}
	else {
		MultiplicationFunctionComponentPtr mult = std::make_shared<MultiplicationFunctionComponent>();
		mult->AddChild(child);
		mult->AddChild(std::make_shared<ConstantFunctionComponent>(-1.0));
		AddChild(mult);
	}
}


std::string AdditionFunctionComponent::Print(bool parentheses) const
{
	std::string res = (parentheses) ? "(" : "";
	const std::vector<FunctionComponentPtr>& children = GetChildren();
	int precedence = Precedence();
	for (const FunctionComponentPtr& child : children) {
		if (child->IsSameType(MultiplicationFunctionComponent()) && std::static_pointer_cast<MultiplicationFunctionComponent>(child)->HasNegative()) {
			res += ((child == children.front()) ? "-" : " - ") + std::static_pointer_cast<MultiplicationFunctionComponent>(child)->PrintNoMinus(child->Precedence() < precedence);
		}
		else if (child->IsSameType(ConstantFunctionComponent()) && std::static_pointer_cast<ConstantFunctionComponent>(child)->GetValue() < 0) {
			res += ((child == children.front()) ? "-" : " - ") + ConstantFunctionComponent(-std::static_pointer_cast<ConstantFunctionComponent>(child)->GetValue()).Print();
		}
		else {
			res += ((child == children.front()) ? "" : " + ") + child->Print(child->Precedence() < precedence);
		}
	}
	res += (parentheses) ? ")" : "";
	return res;
}

FunctionComponentPtr AdditionFunctionComponent::DoFactorBasic() const
{
	return ::FactorBasic(shared_from_this());
}

FunctionComponentPtr AdditionFunctionComponent::DoSimplifyConstants() const
{
	return ::SimplifyConstants(shared_from_this());
}


int AdditionFunctionComponent::Precedence() const
{
	return 1;
}


Associativities AdditionFunctionComponent::Associativity() const
{
	return Associativities::LEFT;
}


int AdditionFunctionComponent::Operands() const
{
	return 2;
}


void AdditionFunctionComponent::DoSort()
{
	std::sort(GetChildren().begin(), GetChildren().end(), 
		[](const FunctionComponentPtr& c1, const FunctionComponentPtr& c2)
	{
		return c1->AdditionOrder() > c2->AdditionOrder();
	});
}

int AdditionFunctionComponent::AdditionOrder() const	// will never be called
{
	return 0;
}

int AdditionFunctionComponent::MultiplicationOrder() const
{
	return ORDER_ADDITION;
}
