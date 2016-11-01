// Noah Rubin

#include <algorithm>

#include "simplify.h"
#include "MathHelper.h"

namespace
{
	// Use AlmostEqual?
	bool IsConstantZero(const FunctionComponentPtr& comp)
	{
		return comp->IsConstant() && std::static_pointer_cast<ConstantFunctionComponent>(comp)->GetValue() == 0;
	}

	bool IsConstantOne(const FunctionComponentPtr& comp)
	{
		return comp->IsConstant() && std::static_pointer_cast<ConstantFunctionComponent>(comp)->GetValue() == 1;
	}

	FunctionComponentPtr CheckOneChild(FunctionComponentPtr comp)
	{
		if (comp->GetChildren().size() == 1) {
			FunctionComponentPtr c(comp->GetChildren().front());
			comp.reset();
			return c;
		}
		return comp;
	}

	AdditionFunctionComponentPtr Distribute(const AdditionFunctionComponentPtr& a, const AdditionFunctionComponentPtr& b)
	{
		AdditionFunctionComponentPtr res = std::make_shared<AdditionFunctionComponent>();
		for (const FunctionComponentPtr& c1 : a->GetChildren()) {
			for (const FunctionComponentPtr& c2 : b->GetChildren()) {
				MultiplicationFunctionComponentPtr m = std::make_shared<MultiplicationFunctionComponent>();
				m->AddChild(c1);
				m->AddChild(c2);
				res->AddChild(m);
			}
		}
		return res;
	}

	AdditionFunctionComponentPtr Distribute(const FunctionComponentPtr& mult, const AdditionFunctionComponentPtr& add)
	{
		if (mult->IsSameType(AdditionFunctionComponent())) {
			return Distribute(std::static_pointer_cast<AdditionFunctionComponent>(mult), add);
		}
		AdditionFunctionComponentPtr res = std::make_shared<AdditionFunctionComponent>();
		for (const FunctionComponentPtr& child : add->GetChildren()) {
			MultiplicationFunctionComponentPtr m = std::make_shared<MultiplicationFunctionComponent>();
			m->AddChild(mult);
			m->AddChild(child);
			res->AddChild(m);
		}
		return res;
	}

	FunctionComponentPtr BasicFactorImplHelper(FunctionComponentPtr comp, FunctionComponentPtr add) {
		if (comp) {
			if (comp->IsSameType(AdditionFunctionComponent())) {
				comp->AddChild(add);
				return comp;
			}
			else {
				AdditionFunctionComponentPtr a = std::make_shared<AdditionFunctionComponent>();
				a->AddChild(comp);
				a->AddChild(add);
				return a;
			}
		}
		else {
			return add;
		}
	}

	FunctionComponentPtr BasicFactorImpl(
		const ConstAdditionFunctionComponentPtr& expr, 
		const std::vector<FunctionComponentPtr>& terms
	)
	{
		std::vector<FunctionComponentPtr> coefficients(terms.size());	// other factors in terms with each factor
		std::vector<FunctionComponentPtr> children = expr->GetChildren();
		for (int i = 0; i < terms.size(); ++i) {
			for (int j = 0; j < children.size(); ++j) {
				if (*terms[i] == *children[j]) {
					coefficients[i] = BasicFactorImplHelper(coefficients[i], std::make_shared<ConstantFunctionComponent>(1.0));
					auto it = children.erase(children.begin() + j);
					j = it - children.begin() - 1;
				}
				else if (children[j]->IsSameType(MultiplicationFunctionComponent())) {
					const std::vector<FunctionComponentPtr> children2 = children[j]->GetChildren();
					for (int k = 0; k < children2.size(); ++k) {
						if (*terms[i] == *children2[k]) {
							FunctionComponentPtr coeff = children[j]->ShallowClone();
							coeff->GetChildren().erase(coeff->GetChildren().begin() + k);
							coefficients[i] = BasicFactorImplHelper(coefficients[i], CheckOneChild(coeff));
							auto it = children.erase(children.begin() + j);
							j = it - children.begin() - 1;
							break;
						}
					}
				}
			}
		}
		AdditionFunctionComponentPtr ret = std::make_shared<AdditionFunctionComponent>();
		for (int i = 0; i < terms.size(); ++i) {
			if (coefficients[i]) {
				if (IsConstantOne(coefficients[i])) {
					ret->AddChild(terms[i]);
				}
				else {
					MultiplicationFunctionComponentPtr mult = std::make_shared<MultiplicationFunctionComponent>();
					mult->AddChild(terms[i]);
					mult->AddChild(coefficients[i]);
					ret->AddChild(mult);
				}
			}
		}
		for (const FunctionComponentPtr& child : children) {	// terms that did not contain one of the factors
			ret->AddChild(child);
		}
		return CheckOneChild(ret);
	}

	FunctionComponentPtr RecursiveBasicFactor(
		const ConstAdditionFunctionComponentPtr& expr,
		const std::vector<FunctionComponentPtr>& terms
	)
	{
		FunctionComponentPtr fact = BasicFactorImpl(expr, terms);
		FunctionComponentPtr temp = fact->SimplifyConstants();
		fact = (temp) ? temp : fact;
		for (auto it = fact->GetChildren().begin(); it != fact->GetChildren().end(); ++it) {
			temp = (*it)->FactorBasic();
			if (temp) {
				*it = temp;
			}
		}
		return fact;
	}

	FunctionComponentPtr PowerTowerBaseAndExponent(const ConstExponentationFunctionComponentPtr& expr, MultiplicationFunctionComponentPtr& exponent)
	{
		exponent->AddChild(expr->GetChildren().back());
		FunctionComponentPtr base = expr->GetChildren().front();
		if (base->IsSameType(ExponentationFunctionComponent())) {
			return PowerTowerBaseAndExponent(std::static_pointer_cast<ExponentationFunctionComponent>(base), exponent);
		}
		return base;
	}

	bool AdditionInDenominator(const ConstMultiplicationFunctionComponentPtr& expr)
	{
		for (const FunctionComponentPtr& child : expr->GetChildren()) {
			if (child->IsSameType(ExponentationFunctionComponent()) &&
				child->GetChildren().front()->IsSameType(AdditionFunctionComponent()) &&
				child->GetChildren().back()->IsConstant() &&
				std::static_pointer_cast<ConstantFunctionComponent>(child->GetChildren().back())->GetValue() < 0
				)
			{
				return true;
			}
		}
		return false;
	}

	FunctionComponentPtr MakeExponentPositive(const ExponentationFunctionComponentPtr& exp)
	{
		const FunctionComponentPtr& exponent = exp->GetChildren().back();
		if (exponent->IsSameType(ConstantFunctionComponent())) {
			double abs = std::abs(std::static_pointer_cast<ConstantFunctionComponent>(exp->GetChildren().back())->GetValue());
			if (AlmostEqual(abs, 1.0)) {
				return exp->GetChildren().front();
			}
			FunctionComponentPtr res = exp->ShallowClone();
			res->GetChildren().back() = std::make_shared<ConstantFunctionComponent>(abs);
			return res;
		}
		else if (exponent->IsSameType(MultiplicationFunctionComponent())) {
			FunctionComponentPtr res = exp->ShallowClone();
			res->GetChildren().back() = std::static_pointer_cast<MultiplicationFunctionComponent>(exp->GetChildren().back())->WithoutNegative();
			return res;

		}
		return exp;
	}

	/*
		For example, if divs contains sin(x)^2 and exp = sin(x)^5, returns 3. If there is no shared exponent, it returns the
		degree of exp. 
	*/
	FunctionComponentPtr ExponentDifference(const std::vector<FunctionComponentPtr>& divs, const ExponentationFunctionComponentPtr& exp)
	{
		//auto 
		return nullptr;
	}
}

FunctionComponentPtr Distribute(const ConstMultiplicationFunctionComponentPtr& expr, bool expandPower)
{
	if (expr->GetChildren().size() < 2 || AdditionInDenominator(expr)) {
		return nullptr;
	}
	std::vector<AdditionFunctionComponentPtr> sums;
	MultiplicationFunctionComponentPtr prods = std::make_shared<MultiplicationFunctionComponent>();
	for (const FunctionComponentPtr& child : expr->GetChildren()) {
		if (child->IsSameType(AdditionFunctionComponent())) {
			sums.push_back(std::static_pointer_cast<AdditionFunctionComponent>(child));
		}
		else if (expandPower && child->IsSameType(ExponentationFunctionComponent()) && 
			child->GetChildren().front()->IsSameType(AdditionFunctionComponent()) &&
			child->GetChildren().back()->IsConstant() &&
			std::static_pointer_cast<ConstantFunctionComponent>(child->GetChildren().back())->GetValue() <= MAX_POWER_EXPAND && 
			std::static_pointer_cast<ConstantFunctionComponent>(child->GetChildren().back())->GetValue() > 1) 
		{
			sums.push_back(ExpandPower(
				std::static_pointer_cast<AdditionFunctionComponent>(child->GetChildren().front()),
				(int) std::static_pointer_cast<ConstantFunctionComponent>(child->GetChildren().back())->GetValue()
			));
		}
		else {
			prods->AddChild(child);
		}
	}
	if (sums.empty()) {
		return nullptr;
	}
	AdditionFunctionComponentPtr res;
	if (prods->GetChildren().empty()) {
		res = Distribute(sums[0], sums[1]);
		for (int i = 2; i < sums.size(); ++i) {
			AdditionFunctionComponentPtr temp = Distribute(res, sums[i]);
			res = temp;
		}
	}
	else {
		res = Distribute(prods, sums[0]);
		for (int i = 1; i < sums.size(); ++i) {
			AdditionFunctionComponentPtr temp = Distribute(res, sums[i]);
			res = temp;
		}
	}
	return res;
}

AdditionFunctionComponentPtr ExpandPower(const AdditionFunctionComponentPtr& expr, int exp)
{
	std::vector<int> coeffs = BinomialCoefficients(exp);
	FunctionComponentPtr term1;
	if (expr->GetChildren().size() > 2) {
		term1 = expr->ShallowClone();
		term1->GetChildren().pop_back();
	}
	else {
		term1 = expr->GetChildren().front();
	}
	FunctionComponentPtr term2 = expr->GetChildren().back();
	AdditionFunctionComponentPtr res = std::make_shared<AdditionFunctionComponent>();
	for (int i = 0; i <= exp; ++i) {
		ExponentationFunctionComponentPtr expon1 = std::make_shared<ExponentationFunctionComponent>();
		ExponentationFunctionComponentPtr expon2 = std::make_shared<ExponentationFunctionComponent>();
		MultiplicationFunctionComponentPtr mult = std::make_shared<MultiplicationFunctionComponent>();
		expon1->AddChild(term1);
		expon1->AddChild(std::make_shared<ConstantFunctionComponent>(exp - i));
		expon2->AddChild(term2);
		expon2->AddChild(std::make_shared<ConstantFunctionComponent>(i));
		mult->AddChild(expon1);
		mult->AddChild(expon2);
		mult->AddChild(std::make_shared<ConstantFunctionComponent>(coeffs[i]));
		res->AddChild(mult);
	}

	for (auto it = res->GetChildren().begin(); it != res->GetChildren().end(); ++it) {
		const FunctionComponentPtr& expon = (*it)->GetChildren().front();
		const FunctionComponentPtr& sum = expon->GetChildren().front();
		if (sum->IsSameType(AdditionFunctionComponent())) {
			*it = ExpandPower(std::static_pointer_cast<AdditionFunctionComponent>(sum),
				(int) std::static_pointer_cast<ConstantFunctionComponent>(expon->GetChildren().back())->GetValue()
				);
		}
	}
	AdditionFunctionComponentPtr simp = std::static_pointer_cast<AdditionFunctionComponent>(res->SimplifyConstants());
	return (simp) ? simp : res;
}

FunctionComponentPtr FactorBasic(const ConstAdditionFunctionComponentPtr& expr)
{
	/*
		Count how many times each factor appears in the terms of expr, then sort them
		by how often they appear and then pass that to FactorBasicImpl()
	*/
	std::vector<std::pair<FunctionComponentPtr, int>> factors;
	for (const FunctionComponentPtr& c : expr->GetChildren()) {
		if (c->IsSameType(MultiplicationFunctionComponent())) {
			for (const FunctionComponentPtr& c2 : c->GetChildren()) {
				auto it = std::find_if(factors.begin(), factors.end(), [&c2](const std::pair<FunctionComponentPtr, int>& a) { return a.first == c2 || *a.first == *c2; });
				if (it == factors.end()) {
					factors.push_back({ c2, 1 });
				}
				else {
					++(it->second);
				}
			}
		}
		else {
			auto it = std::find_if(factors.begin(), factors.end(), [&c](const std::pair<FunctionComponentPtr, int>& a) { return a.first == c || *a.first == *c; });
			if (it == factors.end()) {
				factors.push_back({ c, 1 });
			}
			else {
				++(it->second);
			}
		}
	}
	std::sort(factors.begin(), factors.end(), 
		[](const std::pair<FunctionComponentPtr, int>& a, const std::pair<FunctionComponentPtr, int>& b) 
		{
			return a.second > b.second;
		}
	);
	std::vector<FunctionComponentPtr> sorted;
	for (auto& f : factors) {
		sorted.push_back(f.first);
	}
	return RecursiveBasicFactor(expr, sorted);
}

FunctionComponentPtr FactorBasic(const ConstAdditionFunctionComponentPtr& expr, const std::vector<FunctionComponentPtr>& terms)
{
	return RecursiveBasicFactor(expr, terms);
}

FunctionComponentPtr CombineFractions(const ConstAdditionFunctionComponentPtr& expr)
{
	AdditionFunctionComponentPtr numerator = std::make_shared<AdditionFunctionComponent>();
	std::vector<FunctionComponentPtr> denomFactors;
	for (const FunctionComponentPtr& child : expr->GetChildren()) {
		if (child->IsSameType(MultiplicationFunctionComponent())) {
			const MultiplicationFunctionComponentPtr& mult = std::static_pointer_cast<MultiplicationFunctionComponent>(child);
			numerator->AddChild(mult->GetNumerator());
			std::vector<FunctionComponentPtr> divisors = mult->GetDenominator();
			for (const FunctionComponentPtr& factor : divisors) {
				auto it = std::find_if(denomFactors.begin(), denomFactors.end(), [&factor](const FunctionComponentPtr& a) { return factor == a || *factor == *a; });
				if (it == denomFactors.end()) {
					denomFactors.push_back(factor);
				}
			}
		}
		else if (child->IsSameType(ExponentationFunctionComponent()) && std::static_pointer_cast<ExponentationFunctionComponent>(child)->HasNegativeExponent()) {
			numerator->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
			denomFactors.push_back(child);
		}
		else {
			numerator->AddChild(child);
		}
	}
	for (int i = 0; i < expr->GetChildren().size(); ++i) {
		const FunctionComponentPtr& child = expr->GetChildren()[i];
		if (child->IsSameType(MultiplicationFunctionComponent())) {
			std::vector<FunctionComponentPtr> divisors = std::static_pointer_cast<MultiplicationFunctionComponent>(child)->GetDenominator();
			for (const FunctionComponentPtr& denom : denomFactors) {
				auto it = std::find_if(divisors.begin(), divisors.end(), [&denom](const FunctionComponentPtr& a) { return denom == a || *denom == *a; });
				if (it == divisors.end()) {
					numerator->GetChildren()[i]->AddChild(MakeExponentPositive(std::static_pointer_cast<ExponentationFunctionComponent>(denom)));
				}
			}
		}
		else if (child->IsSameType(ExponentationFunctionComponent())) {
			FunctionComponentPtr mult;
			for (const FunctionComponentPtr& denom : denomFactors) {
				if (!(child == denom || *child == *denom)) {
					if (!mult) {
						mult = MakeExponentPositive(std::static_pointer_cast<ExponentationFunctionComponent>(denom));
					}
					else if (!mult->IsSameType(MultiplicationFunctionComponent())) {
						FunctionComponentPtr temp = mult;
						mult = std::make_shared<MultiplicationFunctionComponent>();
						mult->AddChild(temp);
						mult->AddChild(MakeExponentPositive(std::static_pointer_cast<ExponentationFunctionComponent>(denom)));
					}
				}
			}
			if (mult) {
				numerator->GetChildren()[i] = mult;
			}
		}
		else {
			MultiplicationFunctionComponentPtr mult = std::make_shared<MultiplicationFunctionComponent>();
			mult->AddChild(child);
			for (const FunctionComponentPtr& denom : denomFactors) {
				mult->AddChild(MakeExponentPositive(std::static_pointer_cast<ExponentationFunctionComponent>(denom)));
			}
			numerator->GetChildren()[i] = mult;
		}
	}
	MultiplicationFunctionComponentPtr res = std::make_shared<MultiplicationFunctionComponent>();
	res->AddChild(numerator);
	for (const FunctionComponentPtr& denom : denomFactors) {
		res->AddChild(denom);
	}
	return res;
}

FunctionComponentPtr CombineFractions(const ConstMultiplicationFunctionComponentPtr & frac1, const ConstMultiplicationFunctionComponentPtr & frac2)
{
	return FunctionComponentPtr();
}

FunctionComponentPtr CombineProducts(const ConstMultiplicationFunctionComponentPtr& expr)
{
	std::vector<FunctionComponentPtr> factors;
	for (const FunctionComponentPtr& c : expr->GetChildren()) {
		if (c->IsSameType(ExponentationFunctionComponent())) {
			factors.push_back(c->GetChildren().front());
		}
		else if (!c->IsConstant()) {
			factors.push_back(c);
		}
	}
	std::vector<FunctionComponentPtr> exponents(factors.size());	// other factors in terms with each factor
	std::vector<FunctionComponentPtr> children = expr->GetChildren();
	for (int i = 0; i < factors.size(); ++i) {
		for (int j = 0; j < children.size(); ++j) {
			if (*factors[i] == *children[j]) {
				exponents[i] = BasicFactorImplHelper(exponents[i], std::make_shared<ConstantFunctionComponent>(1.0));
				auto it = children.erase(children.begin() + j);
				j = it - children.begin() - 1;
			}
			else if (children[j]->IsSameType(ExponentationFunctionComponent())) {
				const std::vector<FunctionComponentPtr> children2 = children[j]->GetChildren();
				if (*factors[i] == *children[j]->GetChildren().front()) {
					FunctionComponentPtr exp = children[j]->GetChildren().back();
					exponents[i] = BasicFactorImplHelper(exponents[i], exp);
					auto it = children.erase(children.begin() + j);
					j = it - children.begin() - 1;
				}
			}
		}
	}
	MultiplicationFunctionComponentPtr ret = std::make_shared<MultiplicationFunctionComponent>();
	for (int i = 0; i < factors.size(); ++i) {
		if (exponents[i]) {
			if (IsConstantZero(exponents[i])) {
				ret->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
			}
			else if (IsConstantOne(exponents[i])) {
				ret->AddChild(factors[i]);
			}
			else {
				ExponentationFunctionComponentPtr exp = std::make_shared<ExponentationFunctionComponent>();
				exp->AddChild(factors[i]);
				if (exponents[i]->IsSameType(AdditionFunctionComponent())) {
					exp->AddChild(SimplifyConstants(std::static_pointer_cast<AdditionFunctionComponent>(exponents[i])));
				}
				else {
					exp->AddChild(exponents[i]);
				}
				ret->AddChild(exp);
			}
		}
	}
	for (const FunctionComponentPtr& child : children) {
		ret->AddChild(child);
	}
	return CheckOneChild(ret);
}

FunctionComponentPtr DistributeExponents(const ConstMultiplicationFunctionComponentPtr& expr)
{
	FunctionComponentPtr res = expr->ShallowClone();
	for (auto it = res->GetChildren().begin(); it != res->GetChildren().end(); ++it) {
		if ((*it)->IsSameType(ExponentationFunctionComponent())) {
			FunctionComponentPtr dist = DistributeExponents(std::static_pointer_cast<ExponentationFunctionComponent>(*it));
			if (dist) {
				*it = dist;
			}
		}
	}
	return res;
}

FunctionComponentPtr DistributeExponents(const ConstExponentationFunctionComponentPtr& expr)
{
	if (expr->GetChildren().front()->IsSameType(MultiplicationFunctionComponent())) {
		MultiplicationFunctionComponentPtr res = std::make_shared<MultiplicationFunctionComponent>();
		for (const FunctionComponentPtr& child : expr->GetChildren().front()->GetChildren()) {
			ExponentationFunctionComponentPtr exp = std::make_shared<ExponentationFunctionComponent>();
			exp->AddChild(child);
			exp->AddChild(expr->GetChildren().back());
			res->AddChild(exp);
		}
		return res;
	}
	return nullptr;
}

ExponentationFunctionComponentPtr CombinePowerTowers(const ConstExponentationFunctionComponentPtr& expr)
{
	MultiplicationFunctionComponentPtr exponent = std::make_shared<MultiplicationFunctionComponent>();
	FunctionComponentPtr base = PowerTowerBaseAndExponent(expr, exponent);
	ExponentationFunctionComponentPtr result = std::make_shared<ExponentationFunctionComponent>();
	result->AddChild(base);
	result->AddChild(SimplifyConstants(exponent));
	return result;
}

FunctionComponentPtr SimplifyConstants(const ConstAdditionFunctionComponentPtr& expr)
{
	FunctionComponentPtr ret = expr->ShallowClone();
	double sum = 0.0;
	for (int i = 0; i < ret->GetChildren().size(); ++i) {
		if (ret->GetChildren()[i]->IsConstant()) {
			sum += std::static_pointer_cast<ConstantFunctionComponent>(ret->GetChildren()[i])->GetValue();
			auto it = ret->GetChildren().erase(ret->GetChildren().begin() + i);
			i = std::distance(ret->GetChildren().begin(), it) - 1;
		}
	}
	if (sum != 0.0 || ret->GetChildren().empty()) {	// use AlmostEqual?
		ret->AddChild(std::make_shared<ConstantFunctionComponent>(sum));
	}
	return CheckOneChild(ret);
}

FunctionComponentPtr SimplifyConstants(const ConstMultiplicationFunctionComponentPtr& expr)
{
	FunctionComponentPtr ret = expr->ShallowClone();
	double product = 1.0;
	for (int i = 0; i < ret->GetChildren().size(); ++i) {
		if (ret->GetChildren()[i]->IsConstant()) {
			product *= std::static_pointer_cast<ConstantFunctionComponent>(ret->GetChildren()[i])->GetValue();
			auto it = ret->GetChildren().erase(ret->GetChildren().begin() + i);
			i = std::distance(ret->GetChildren().begin(), it) - 1;
		}
	}
	if (AlmostEqual(product, 0.0)) {
		return std::make_shared<ConstantFunctionComponent>(0.0);
	}
	else if (product != 1.0 || ret->GetChildren().empty()) {	// use AlmostEqual?
		ret->AddChild(std::make_shared<ConstantFunctionComponent>(product));
	}
	return CheckOneChild(ret);
}

FunctionComponentPtr SimplifyConstants(const ConstExponentationFunctionComponentPtr& expr)
{
	const FunctionComponentPtr base = expr->GetChildren().front();
	const FunctionComponentPtr exp = expr->GetChildren().back();
	bool b1 = base->IsConstant();
	bool b2 = exp->IsConstant();
	if (b1 && b2) {
		double d1 = std::static_pointer_cast<ConstantFunctionComponent>(base)->GetValue();
		double d2 = std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue();
		return std::make_shared<ConstantFunctionComponent>(std::pow(d1, d2));
	}
	else if (b1) {
		double d1 = std::static_pointer_cast<ConstantFunctionComponent>(base)->GetValue();
		if (AlmostEqual(d1, 0.0)) {
			return std::make_shared<ConstantFunctionComponent>(0.0);
		}
		else if (AlmostEqual(d1, 1.0)) {
			return std::make_shared<ConstantFunctionComponent>(1.0);
		}
	}
	else if (b2) {
		double d2 = std::static_pointer_cast<ConstantFunctionComponent>(exp)->GetValue();
		if (AlmostEqual(d2, 0.0)) {
			return std::make_shared<ConstantFunctionComponent>(1.0);
		}
		else if (AlmostEqual(d2, 1.0)) {
			return base;
		}
	}
	return expr->ShallowClone();
}
