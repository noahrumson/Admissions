// Noah Rubin

#ifndef FUNCTION_CALL_DERIVATIVE_H_INCLUDED
#define FUNCTION_CALL_DERIVATIVE_H_INCLUDED

#include <unordered_map>

#include "FunctionComponent.h"

typedef FunctionComponentPtr(*FunctionDerivative)(const FunctionComponentPtr&);

extern const std::unordered_map<EvaluableFunction, FunctionDerivative> functionDerivativeMap;
extern const std::unordered_map<EvaluableFunction, std::string> functionNameMap;

double csc(double x);	// cosecant
double sec(double x);	// secant
double cot(double x);	// cotangent


#endif