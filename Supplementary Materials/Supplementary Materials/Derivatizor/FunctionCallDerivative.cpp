// Noah Rubin

#include <cmath>

#include "FunctionCallDerivative.h"

FunctionComponentPtr NaturalLogDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr ExpDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr SineDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr CosineDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr TangentDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr CosecantDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr SecantDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr CotangentDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr ArcsinDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr ArccosDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr ArctanDerivative(const FunctionComponentPtr& arg);
FunctionComponentPtr SqrtDerivative(const FunctionComponentPtr& arg);

const std::unordered_map<EvaluableFunction, FunctionDerivative> functionDerivativeMap = {
	{ log,	NaturalLogDerivative},
	{ exp,  ExpDerivative		},
	{ sin,	SineDerivative		},
	{ cos,	CosineDerivative	},
	{ tan,	TangentDerivative	},
	{ sec,	SecantDerivative	},
	{ csc,	CosecantDerivative	},
	{ cot,	CotangentDerivative	},
	{ asin, ArcsinDerivative	},
	{ acos, ArccosDerivative	},
	{ atan, ArctanDerivative	},
	{ sqrt, SqrtDerivative		}
};


const std::unordered_map<EvaluableFunction, std::string> functionNameMap = {
		{ log, "log" },
		{ exp, "exp" },
		{ sin, "sin" },
		{ cos, "cos" },
		{ tan, "tan" },
		{ sec, "sec" },
		{ csc, "csc" },
		{ cot, "cot" },
		{ asin, "asin" },
		{ acos, "acos" },
		{ atan, "atan" },
		{ sqrt, "sqrt" }
};


double sec(double x)
{
	return 1.0 / cos(x);
}

double csc(double x)
{
	return 1.0 / sin(x);
}

double cot(double x)
{
	return 1.0 / tan(x);
}


FunctionComponentPtr NaturalLogDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
	tree->AddInverseChild(arg);
	return tree;
}


FunctionComponentPtr ExpDerivative(const FunctionComponentPtr& arg)
{
	ExponentationFunctionComponentPtr res = std::make_shared<ExponentationFunctionComponent>();
	res->AddChild(std::make_shared<ConstantFunctionComponent>(M_E));
	res->AddChild(arg);
	return res;
}


FunctionComponentPtr SineDerivative(const FunctionComponentPtr& arg)
{
	FunctionCallFunctionComponentPtr tree = std::make_shared<FunctionCallFunctionComponent>(static_cast<EvaluableFunction>(cos));
	tree->AddChild(arg);
	return tree;
}


FunctionComponentPtr CosineDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared< MultiplicationFunctionComponent>();
	FunctionCallFunctionComponentPtr call = std::make_shared< FunctionCallFunctionComponent>(static_cast<EvaluableFunction>(sin));
	call->AddChild(arg);
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(-1.0));
	tree->AddChild(call);
	return tree;
}


FunctionComponentPtr TangentDerivative(const FunctionComponentPtr& arg)
{
	ExponentationFunctionComponentPtr tree = std::make_shared<ExponentationFunctionComponent>();
	FunctionCallFunctionComponentPtr call = std::make_shared<FunctionCallFunctionComponent>(sec);
	call->AddChild(arg);
	tree->AddChild(call);
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(2.0));
	return tree;
}


FunctionComponentPtr SecantDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	FunctionCallFunctionComponentPtr call1 = std::make_shared<FunctionCallFunctionComponent>(sec);
	FunctionCallFunctionComponentPtr call2 = std::make_shared<FunctionCallFunctionComponent>(static_cast<EvaluableFunction>(tan));
	call1->AddChild(arg);
	call2->AddChild(arg);
	tree->AddChild(call1);
	tree->AddChild(call2);
	return tree;
}


FunctionComponentPtr CosecantDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	MultiplicationFunctionComponentPtr mult = std::make_shared<MultiplicationFunctionComponent>();
	FunctionCallFunctionComponentPtr call1 = std::make_shared<FunctionCallFunctionComponent>(csc);
	FunctionCallFunctionComponentPtr call2 = std::make_shared<FunctionCallFunctionComponent>(cot);
	call1->AddChild(arg);
	call2->AddChild(arg);
	mult->AddChild(call1);
	mult->AddChild(call2);
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(-1.0));
	tree->AddChild(mult);
	return tree;
}


FunctionComponentPtr CotangentDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	ExponentationFunctionComponentPtr square = std::make_shared<ExponentationFunctionComponent>();
	FunctionCallFunctionComponentPtr call = std::make_shared<FunctionCallFunctionComponent>(csc);
	call->AddChild(arg);
	square->AddChild(call);
	square->AddChild(std::make_shared<ConstantFunctionComponent>(2.0));
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(-1.0));
	tree->AddChild(square);
	return tree;
}


FunctionComponentPtr ArcsinDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	ExponentationFunctionComponentPtr sqrt = std::make_shared<ExponentationFunctionComponent>();
	AdditionFunctionComponentPtr sub = std::make_shared<AdditionFunctionComponent>();
	ExponentationFunctionComponentPtr square = std::make_shared<ExponentationFunctionComponent>();
	square->AddChild(arg);
	square->AddChild(std::make_shared<ConstantFunctionComponent>(2.0));
	sub->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
	sub->AddInverseChild(square);
	sqrt->AddChild(sub);
	sqrt->AddChild(std::make_shared<ConstantFunctionComponent>(0.5));
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
	tree->AddInverseChild(sqrt);
	return tree;
}


FunctionComponentPtr ArccosDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(-1.0));
	tree->AddChild(ArcsinDerivative(arg));
	return tree;
}


FunctionComponentPtr ArctanDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr tree = std::make_shared<MultiplicationFunctionComponent>();
	AdditionFunctionComponentPtr add = std::make_shared<AdditionFunctionComponent>();
	ExponentationFunctionComponentPtr square = std::make_shared<ExponentationFunctionComponent>();
	square->AddChild(arg);
	square->AddChild(std::make_shared<ConstantFunctionComponent>(2.0));
	add->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
	add->AddChild(square);
	tree->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
	tree->AddInverseChild(add);
	return tree;
}


FunctionComponentPtr SqrtDerivative(const FunctionComponentPtr& arg)
{
	MultiplicationFunctionComponentPtr res = std::make_shared<MultiplicationFunctionComponent>();
	ExponentationFunctionComponentPtr sqrt = std::make_shared<ExponentationFunctionComponent>();
	sqrt->AddChild(arg);
	sqrt->AddChild(std::make_shared<ConstantFunctionComponent>(0.5));
	res->AddChild(std::make_shared<ConstantFunctionComponent>(1.0));
	res->AddInverseChild(std::make_shared<ConstantFunctionComponent>(2.0));
	res->AddInverseChild(sqrt);
	return res;
}