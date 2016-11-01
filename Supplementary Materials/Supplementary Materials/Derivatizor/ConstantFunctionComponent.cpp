// Noah Rubin

#include <sstream>
#include <iomanip>

#include "FunctionComponent.h"
#include "Parser.h"
#include "MathHelper.h"


ConstantFunctionComponent::ConstantFunctionComponent() :
	FunctionComponentImpl(),
	m_val(0.0)
{
}


ConstantFunctionComponent::ConstantFunctionComponent(double val) :
	FunctionComponentImpl(),
	m_val(val)
{
}


bool ConstantFunctionComponent::operator==(const FunctionComponent& a) const
{
	return IsSameType(a) && AlmostEqual(static_cast<const ConstantFunctionComponent&>(a).m_val, m_val);
}


bool ConstantFunctionComponent::operator!=(const FunctionComponent& a) const
{
	return !operator==(a);
}


// (C)' = 0
FunctionComponentPtr ConstantFunctionComponent::Differentiate() const
{
	return std::make_shared<ConstantFunctionComponent>(0.0);
}


std::string ConstantFunctionComponent::Print(bool parentheses) const
{
	double intpart;
	if (m_val == M_PI) {
		return "pi";
	}
	else if (m_val == M_E) {
		return "e";
	}
	else if (std::modf(m_val, &intpart) == 0.0) {
		return std::to_string((int) m_val);
	}
	std::ostringstream out;
	out << std::setprecision(maxDigits) << m_val;
	return out.str();
}


bool ConstantFunctionComponent::IsConstant() const
{
	return true;
}


int ConstantFunctionComponent::AdditionOrder() const
{
	return 0;
}


int ConstantFunctionComponent::MultiplicationOrder() const
{
	return ORDER_CONSTANT;
}