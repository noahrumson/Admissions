// Noah Rubin

#include "FunctionComponent.h"
#include "FunctionCallDerivative.h"
#include "simplify.h"
#include "MathHelper.h"
#include "defines.h"


FunctionCallFunctionComponent::FunctionCallFunctionComponent(EvaluableFunction func) :
	FunctionComponentImpl(),
	m_func(func),
	m_hasParentheses(false)
{
}


bool FunctionCallFunctionComponent::operator==(const FunctionComponent& a) const
{
	return IsSameType(a) && 
			static_cast<const FunctionCallFunctionComponent&>(a).m_func == m_func &&
			*GetChildren().front() == *a.GetChildren().front();
}


bool FunctionCallFunctionComponent::operator!=(const FunctionComponent& a) const
{
	return !operator==(a);
}


FunctionComponentPtr FunctionCallFunctionComponent::Differentiate() const
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	FunctionComponentPtr arg = functionDerivativeMap.find(m_func)->second(GetChildren()[0]);
	tree->AddChild(arg);
	tree->AddChild(GetChildren()[0]->Differentiate());
	return tree;
}


//FunctionComponentPtr FunctionCallFunctionComponent::DoSimplifyChildren(bool distribute) const
//{
//	static constexpr EvaluableFunction sqrtFunc = sqrt;
//	double val;
//	const FunctionComponent& arg = *GetChildren()[0];
//	if (arg.IsConstant()) {
//		val = m_func(static_cast<const ConstantFunctionComponent&>(arg).GetValue());
//		double dummy;
//		if (std::modf(val, &dummy) == 0.0 || std::modf(val / M_PI, &dummy) == 0.0) {
//			return std::make_shared<ConstantFunctionComponent>(val);
//		}
//	}
//	else if (m_func == sqrtFunc) {
//		ExponentationFunctionComponentPtr exp = std::make_shared<ExponentationFunctionComponent>();
//		exp->AddChild(GetChildren().front());
//		exp->AddChild(std::make_shared<ConstantFunctionComponent>(0.5));
//		return exp;
//	}
//	return nullptr;
//}

FunctionComponentPtr FunctionCallFunctionComponent::DoSimplifyConstants() const
{
	static EvaluableFunction sqrtFunc = sqrt;
	static double epsilon = std::numeric_limits<double>::epsilon();
	const FunctionComponentPtr arg = GetChildren().front();
	if (arg->IsConstant()) {
		double val = m_func(std::static_pointer_cast<ConstantFunctionComponent>(arg)->GetValue());
		double ipart;
		if (std::abs(std::modf(val, &ipart)) < epsilon) {
			return std::make_shared<ConstantFunctionComponent>(ipart);
		}
	}
	if (m_func == sqrtFunc) {
		ExponentationFunctionComponentPtr exp = std::make_shared<ExponentationFunctionComponent>();
		exp->AddChild(GetChildren().front());
		exp->AddChild(std::make_shared<ConstantFunctionComponent>(0.5));
		return exp;
	}
	return nullptr;
}


std::string FunctionCallFunctionComponent::Print(bool parentheses) const
{
	m_hasParentheses = true;
	return functionNameMap.find(m_func)->second + "(" + GetChildren()[0]->Print() + ")";
}


int FunctionCallFunctionComponent::Precedence() const
{
	return m_hasParentheses ? 5 : 2;
}


Associativities FunctionCallFunctionComponent::Associativity() const
{
	return Associativities::LEFT;
}


void FunctionCallFunctionComponent::AddChild(const FunctionComponentPtr& child)
{
	GetChildren().push_back(child);
}


int FunctionCallFunctionComponent::Operands() const
{
	return 1;
}

int FunctionCallFunctionComponent::AdditionOrder() const
{
	return 1;
}


int FunctionCallFunctionComponent::MultiplicationOrder() const
{
	static constexpr EvaluableFunction sinFunc = sin; // avoid overload ambiguity
	static constexpr EvaluableFunction logFunc = log;
	if (m_func == sinFunc) {
		return ORDER_SINE;
	}
	else if (m_func == logFunc) {
		return ORDER_LOG;
	}
	else {
		return ORDER_TRIG_FUNCTION;
	}
}
