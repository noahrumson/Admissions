// Noah Rubin

#include <algorithm>

#include "FunctionComponent.h"
#include "MathHelper.h"
#include "simplify.h"

namespace
{
	FunctionComponentPtr DifferentiateMultiply(const FunctionComponentPtr& a, const FunctionComponentPtr& b)
	{
		FunctionComponentPtr add = std::make_shared<AdditionFunctionComponent>();
		MultiplicationFunctionComponentPtr term1 = std::make_shared<MultiplicationFunctionComponent>();
		MultiplicationFunctionComponentPtr term2 = std::make_shared<MultiplicationFunctionComponent>();
		term1->AddChild(a);
		term1->AddChild(b->Differentiate());
		term2->AddChild(b);
		term2->AddChild(a->Differentiate());
		add->AddChild(term1);
		add->AddChild(term2);
		return add;
	}


	FunctionComponentPtr DifferentiateDivide(const FunctionComponentPtr& a, const FunctionComponentPtr& b)
	{
		MultiplicationFunctionComponentPtr div = std::make_shared<MultiplicationFunctionComponent>();
		AdditionFunctionComponentPtr numerator = std::make_shared<AdditionFunctionComponent>();
		MultiplicationFunctionComponentPtr term1 = std::make_shared<MultiplicationFunctionComponent>();
		MultiplicationFunctionComponentPtr term2 = std::make_shared<MultiplicationFunctionComponent>();
		ExponentationFunctionComponentPtr denom = std::make_shared<ExponentationFunctionComponent>();
		term1->AddChild(a->Differentiate());
		term1->AddChild(b);
		term2->AddChild(b->Differentiate());
		term2->AddChild(a);
		numerator->AddChild(term1);
		numerator->AddInverseChild(term2);
		denom->AddChild(b);
		denom->AddChild(std::make_shared<ConstantFunctionComponent>(2.0));
		div->AddChild(numerator);
		div->AddInverseChild(denom);
		return div;
	}


	// If num == m * base^n for integer m and n, sets pow = n, mult = m and returns true
	bool MultipleOfPower(double base, double num, double& pow, double& mult)
	{
		std::modf(std::log(num) / std::log(base), &pow);
		mult = num / std::pow(base, pow);
		double dummy;
		return pow != 0.0 && std::modf(mult, &dummy) == 0.0;
	}


	bool CheckCommonBase(FunctionComponentPtr& comp, double product, double& mult)
	{
		if (comp->IsSameType(ExponentationFunctionComponent())) {
			/*double intpart;
			if (comp.GetChildren().front()->IsConstant() &&
				std::modf(std::log(product) / std::log(static_cast<ConstantFunctionComponent*>(comp.GetChildren().front())->GetValue()), &intpart) == 0.0)*/
			double pow;
			if (comp->GetChildren().front()->IsConstant() &&
				MultipleOfPower(std::static_pointer_cast<ConstantFunctionComponent>(comp->GetChildren().front())->GetValue(), product, pow, mult)
			)
			{
				AdditionFunctionComponentPtr add = std::make_shared<AdditionFunctionComponent>();
				add->AddChild(comp->GetChildren().back());
				add->AddChild(std::make_shared<ConstantFunctionComponent>(pow));
				comp->GetChildren().back() = add;
				return true;
			}
		}
		return false;
	}

	void HandleOmitNegative(std::vector<FunctionComponentPtr>& comps, const ConstantFunctionComponentPtr& constant)
	{
		if (constant->GetValue() != -1.0) {
			if (constant->GetValue() < 0) {	// AlmostEqual?
				comps.push_back(std::make_shared<ConstantFunctionComponent>(-constant->GetValue()));
			}
			else {
				comps.push_back(constant);
			}
		}
	}
}


// (f * g)' = fg' + g'f
FunctionComponentPtr MultiplicationFunctionComponent::Differentiate() const
{
	auto children = GetChildren();
	FunctionComponentPtr result;
	MultiplicationFunctionComponentPtr mult = std::make_shared<MultiplicationFunctionComponent>();
	if (children.size() == 2) {
		result = DifferentiateMultiply(children[0], children[1]);
	}
	else {
		for (int i = 0; i + 1 < children.size(); ++i) {
			mult->AddChild(children[i]);
		}
		result = DifferentiateMultiply(mult, children.back());
	}
	return result;
}

void MultiplicationFunctionComponent::AddInverseChild(const FunctionComponentPtr& child)
{
	if (child->IsConstant()) {
		AddChild(std::make_shared<ConstantFunctionComponent>(1.0 / std::static_pointer_cast<ConstantFunctionComponent>(child)->GetValue()));
	}
	else {
		ExponentationFunctionComponentPtr exp = std::make_shared<ExponentationFunctionComponent>();
		exp->AddChild(child);
		exp->AddChild(std::make_shared<ConstantFunctionComponent>(-1.0));
		AddChild(exp);
	}
}

MultiplicationFunctionComponentPtr MultiplicationFunctionComponent::GetNumerator() const
{
	MultiplicationFunctionComponentPtr res = std::static_pointer_cast<MultiplicationFunctionComponent>(ShallowClone());
	for (auto it = res->GetChildren().begin(); it != res->GetChildren().end();) {
		if ((*it)->IsSameType(ExponentationFunctionComponent()) &&
			std::static_pointer_cast<ExponentationFunctionComponent>(*it)->HasNegativeExponent())
		{
			it = res->GetChildren().erase(it);
		}
		else {
			++it;
		}
	}
	return res;
}

std::vector<FunctionComponentPtr> MultiplicationFunctionComponent::GetDenominator() const
{
	std::vector<FunctionComponentPtr> denoms;
	for (const FunctionComponentPtr& factor : GetChildren()) {
		if (factor->IsSameType(ExponentationFunctionComponent()) &&
			std::static_pointer_cast<ExponentationFunctionComponent>(factor)->HasNegativeExponent())
		{
			denoms.push_back(factor);
		}
	}
	return denoms;
}

std::string MultiplicationFunctionComponent::Print(bool parentheses) const
{
	std::string res = (parentheses) ? "(" : "";
	const std::vector<FunctionComponentPtr>& children = GetChildren();
	int precedence = Precedence();
	std::vector<FunctionComponentPtr> mult;
	std::vector<FunctionComponentPtr> divs;
	for (const FunctionComponentPtr& child : children) {
		if (child->IsSameType(ExponentationFunctionComponent()) && std::static_pointer_cast<ExponentationFunctionComponent>(child)->HasNegativeExponent()) {
			divs.push_back(child);
		}
		else {
			mult.push_back(child);
		}
	}
	if (mult.empty()) {
		res += "1";
	}
	else {
		for (const FunctionComponentPtr& child : mult) {
			res += ((child == mult.front()) ? "" : "*") + child->Print(child->Precedence() < precedence);
		}
	}
	if (!divs.empty()) {
		res += "/";
	}
	if (divs.size() > 1) {
		res += "(";
	}
	for (const FunctionComponentPtr& child : divs) {
		if (child->IsSameType(ExponentationFunctionComponent())) {
			res += ((child == divs.front()) ? "" : "*") + std::static_pointer_cast<ExponentationFunctionComponent>(child)->PrintNoMinus(child->Precedence() < precedence);
		}
		else {
			res += ((child == divs.front()) ? "" : "*") + child->Print(child->Precedence() < precedence);
		}
	}
	if (divs.size() > 1) {
		res += ")";
	}
	res += (parentheses) ? ")" : "";
	return res;
}

std::string MultiplicationFunctionComponent::PrintNoMinus(bool parentheses) const
{
	std::string res = (parentheses) ? "(" : "";
	const std::vector<FunctionComponentPtr>& children = GetChildren();
	int precedence = Precedence();
	std::vector<FunctionComponentPtr> mult;
	std::vector<ExponentationFunctionComponentPtr> divs;
	for (const FunctionComponentPtr& child : children) {
		if (child->IsSameType(ExponentationFunctionComponent()) && std::static_pointer_cast<ExponentationFunctionComponent>(child)->HasNegativeExponent()) {
			divs.push_back(std::static_pointer_cast<ExponentationFunctionComponent>(child));
		}
		else if (child->IsConstant()) {
			HandleOmitNegative(mult, std::static_pointer_cast<ConstantFunctionComponent>(child));
		}
		else {
			mult.push_back(child);
		}
	}
	if (mult.empty()) {
		res += "1";
	}
	else {
		for (const FunctionComponentPtr& child : mult) {
			res += ((child == mult.front()) ? "" : "*") + child->Print(child->Precedence() < precedence);
		}
	}
	if (!divs.empty()) {
		res += "/";
	}
	if (divs.size() > 1) {
		res += "(";
	}
	for (const ExponentationFunctionComponentPtr& child : divs) {
		FunctionComponentPtr print = child->WithoutNegative();
		res += ((child == divs.front()) ? "" : "*") + print->Print(print->Precedence() < precedence); //child->Print(child->Precedence() < precedence);
	}
	if (divs.size() > 1) {
		res += ")";
	}
	res += (parentheses) ? ")" : "";
	return res;
}

FunctionComponentPtr MultiplicationFunctionComponent::DoDistribute(bool expandPowers) const
{
	return ::Distribute(shared_from_this(), !EXPAND_EXPONENTS);
}

FunctionComponentPtr MultiplicationFunctionComponent::DoDistributeExponents() const
{
	return ::DistributeExponents(shared_from_this());
}

FunctionComponentPtr MultiplicationFunctionComponent::DoCombineProducts() const
{
	return ::CombineProducts(shared_from_this());
}

FunctionComponentPtr MultiplicationFunctionComponent::DoSimplifyConstants() const
{
	return ::SimplifyConstants(shared_from_this());
}


int MultiplicationFunctionComponent::Precedence() const
{
	return 3;
}


Associativities MultiplicationFunctionComponent::Associativity() const
{
	return Associativities::LEFT;
}


int MultiplicationFunctionComponent::Operands() const
{
	return 2;
}


bool MultiplicationFunctionComponent::HasNegative() const
{
	int negatives = 0;
	for (const FunctionComponentPtr& c : GetChildren()) {
		if (c->IsConstant() && std::static_pointer_cast<ConstantFunctionComponent>(c)->GetValue() < 0.0) {
			++negatives;
		}
	}
	return negatives % 2 == 1;
}

FunctionComponentPtr MultiplicationFunctionComponent::WithoutNegative() const
{
	FunctionComponentPtr res = ShallowClone();
	for (auto it = res->GetChildren().begin(); it != res->GetChildren().end();) {
		if ((*it)->IsConstant()) {
			double val = std::static_pointer_cast<ConstantFunctionComponent>(*it)->GetValue();
			if (AlmostEqual(val, -1.0)) {
				it = res->GetChildren().erase(it);
				continue;	// don't increment it
			}
			else if (val < 0.0) {
				*it = std::make_shared<ConstantFunctionComponent>(-std::static_pointer_cast<ConstantFunctionComponent>(*it)->GetValue());
			}
		}
		++it;
	}
	return res;
}

void MultiplicationFunctionComponent::DoSort()
{
	std::sort(GetChildren().begin(), GetChildren().end(),
		[](const FunctionComponentPtr& c1, const FunctionComponentPtr& c2)
	{
		return c1->MultiplicationOrder() < c2->MultiplicationOrder();
	});
}


int MultiplicationFunctionComponent::AdditionOrder() const
{
	double degree = 1.0;
	for (const FunctionComponentPtr& c : GetChildren()) {
		if (c->IsSameType(ExponentationFunctionComponent())) {
			if (c->GetChildren().back()->IsSameType(ConstantFunctionComponent())) {
				degree = std::max(degree, std::static_pointer_cast<ConstantFunctionComponent>(c->GetChildren().back())->GetValue());
			}
			else {
				degree = 100000.0;
			}
		}
	}
	return (int)(10 * degree) + GetChildren().size() - 1;
}


int MultiplicationFunctionComponent::MultiplicationOrder() const	// will never be called
{
	return 0;
}
