// Noah Rubin

#include <cmath>
#include <limits>

#include "FunctionComponent.h"
#include "FunctionCallDerivative.h"
#include "MathHelper.h"
#include "simplify.h"


FunctionComponentPtr ExponentationFunctionComponent::Differentiate() const
{
	FunctionComponentPtr base = GetChildren().front();
	FunctionComponentPtr exponent = GetChildren().back();

	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	ExponentationFunctionComponentPtr exp = std::make_shared<ExponentationFunctionComponent>();
	AdditionFunctionComponentPtr sub = std::make_shared<AdditionFunctionComponent>();
	AdditionFunctionComponentPtr add = std::make_shared<AdditionFunctionComponent>();
	MultiplicationFunctionComponentPtr term1 = std::make_shared<MultiplicationFunctionComponent>();
	MultiplicationFunctionComponentPtr term2 = std::make_shared<MultiplicationFunctionComponent>();
	FunctionCallFunctionComponentPtr call = std::make_shared<FunctionCallFunctionComponent>(static_cast<EvaluableFunction>(log));
	sub->AddChild(exponent);
	sub->AddInverseChild(std::make_shared<ConstantFunctionComponent>(1.0));
	exp->AddChild(base);
	exp->AddChild(sub);
	term1->AddChild(exponent);
	term1->AddChild(base->Differentiate());
	call->AddChild(base);
	term2->AddChild(base);
	term2->AddChild(call);
	term2->AddChild(exponent->Differentiate());
	add->AddChild(term1);
	add->AddChild(term2);
	tree->AddChild(exp);
	tree->AddChild(add);

	return tree;
}


FunctionComponentPtr ExponentationFunctionComponent::DoSimplifyConstants() const
{
	return ::SimplifyConstants(CombinePowerTowers(shared_from_this()));
}

FunctionComponentPtr ExponentationFunctionComponent::DoDistribute(bool expandPowers) const
{
	return (expandPowers && GetChildren().front()->IsSameType(AdditionFunctionComponent()) && 
		GetChildren().back()->IsConstant() &&
		std::static_pointer_cast<ConstantFunctionComponent>(GetChildren().back())->GetValue() <= MAX_POWER_EXPAND &&
		std::static_pointer_cast<ConstantFunctionComponent>(GetChildren().back())->GetValue() > 1) ?
		::ExpandPower(
			std::static_pointer_cast<AdditionFunctionComponent>(GetChildren().front()),
			(int) std::static_pointer_cast<ConstantFunctionComponent>(GetChildren().back())->GetValue()
		) : nullptr;
}

bool ExponentationFunctionComponent::TestAddSameType(const FunctionComponentPtr& comp)
{
	return false;
}

FunctionComponentPtr ExponentationFunctionComponent::DoDistributeExponents() const
{
	return ::DistributeExponents(shared_from_this());
}


int ExponentationFunctionComponent::Precedence() const
{
	return 4;
}


Associativities ExponentationFunctionComponent::Associativity() const
{
	return Associativities::RIGHT;
}


int ExponentationFunctionComponent::Operands() const
{
	return 2;
}

void ExponentationFunctionComponent::AddChild(const FunctionComponentPtr& child)
{
	GetChildren().push_back(child);
}

FunctionComponentPtr ExponentationFunctionComponent::WithoutNegative() const
{
	FunctionComponentPtr res = ShallowClone();
	const FunctionComponentPtr& exp = GetChildren().back();
	if (exp->IsSameType(ConstantFunctionComponent()) && std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue() < 0.0) {
		double abs = -std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue();
		if (abs != 1.0) { // AlmostEqual?
			res->GetChildren().back() = std::make_shared<ConstantFunctionComponent>(abs);
		}
		else {
			res = res->GetChildren().front();
		}
	}
	else if (exp->IsSameType(MultiplicationFunctionComponent()) && std::static_pointer_cast<MultiplicationFunctionComponent>(exp)->HasNegative()) {
		res->GetChildren().back() = std::static_pointer_cast<MultiplicationFunctionComponent>(exp)->WithoutNegative();
	}
	return res;
}

int ExponentationFunctionComponent::AdditionOrder() const
{
	FunctionComponentPtr exp = GetChildren().back();
	if (exp->IsConstant()) {
		return 10 * std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue();
	}
	else {
		return 100000.0;
	}
}


int ExponentationFunctionComponent::MultiplicationOrder() const
{
	return ORDER_EXPONENTIAL;
}


std::string ExponentationFunctionComponent::Print(bool parentheses) const
{
	if (HasNegativeExponent()) {
		MultiplicationFunctionComponent temp;
		FunctionComponentPtr without = WithoutNegative();
		temp.AddInverseChild((without) ? without : ShallowClone());
		return temp.Print(parentheses);
	}
	else {
		std::string res = (parentheses) ? "(" : "";
		const std::vector<FunctionComponentPtr>& children = GetChildren();
		int precedence = Precedence();
		if (children.back()->IsConstant() && AlmostEqual(std::static_pointer_cast<ConstantFunctionComponent>(children.back())->GetValue(), 0.5)) {
			res += "sqrt(" + children.front()->Print() + ")";
		}
		else {
			res += children.front()->Print(children.front()->Precedence() < precedence || children.front()->IsSameType(*this)) + 
				"^" + 
				children.back()->Print(children.back()->Precedence() < precedence);
		}
		res += (parentheses) ? ")" : "";
		return res;
	}
}

std::string ExponentationFunctionComponent::PrintNoMinus(int precedence) const
{
	const std::vector<FunctionComponentPtr>& children = GetChildren();
	const FunctionComponentPtr& exp = children.back();
	if (exp->IsSameType(ConstantFunctionComponent()) && std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue() < 0.0) {
		FunctionComponentPtr temp = ShallowClone();
		double abs = -std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue();
		if (abs != 1.0) { // AlmostEqual?
			temp->GetChildren().back() = std::make_shared<ConstantFunctionComponent>(abs);
			return temp->Print(false);
		}
		else {
			return temp->GetChildren().front()->Print(false);
		}
	}
	else if (exp->IsSameType(MultiplicationFunctionComponent()) && std::static_pointer_cast<MultiplicationFunctionComponent>(exp)->HasNegative()) {
		FunctionComponentPtr temp = std::static_pointer_cast<MultiplicationFunctionComponent>(exp)->WithoutNegative();
		return children.front()->Print(children.front()->Precedence() < precedence || children.front()->IsSameType(*this)) +
			"^" + temp->Print(temp->Precedence() > precedence);
	}
	return Print(false);
}

bool ExponentationFunctionComponent::HasNegativeExponent() const
{
	const FunctionComponentPtr& exp = GetChildren().back();
	return	(exp->IsSameType(ConstantFunctionComponent()) && std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue() < 0.0) ||
			(exp->IsSameType(MultiplicationFunctionComponent()) && std::static_pointer_cast<MultiplicationFunctionComponent>(exp)->HasNegative());
}
