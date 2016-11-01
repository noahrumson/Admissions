// Noah Rubin

#ifndef SIMPLIFY_H_INCLUDED
#define SIMPLIFY_H_INCLUDED

#define MAX_POWER_EXPAND 10
#define EXPAND_EXPONENTS true

#include "FunctionComponent.h"

/*	
	All the following functions assume the expressions they receive as inputs are not empty. They
	will also not simplify constant terms in the expressions, so expr->SimplifyConstants() should be
	called first.

	These are not member functions because the type of the resulting expression could be different than
	the type of the original expression. For example, calling Distribute on a multiplication could result
	in an addition. We could make these member functions but instead of editing the called upon object, just
	return a new object.
*/

/*
	Transforms expressions such as ab(c+d-e) -> abc+abd-abe
	If second argument is true will distribute exponents, i.e. (a+b)^2 -> a^2+2ab+b^2 up to
	a predefined maximum exponent, currently 10.

	If it could not distribute, i.e. expr does not contain an additive term, then it returns nullptr.
*/
FunctionComponentPtr Distribute(const ConstMultiplicationFunctionComponentPtr& expr, bool expandPowers);

/*
	Will distribute exponents, i.e. (a+b)^2 -> a^2+2ab+b^2 up to a predefined maximum exponent, currently 10.
*/
AdditionFunctionComponentPtr ExpandPower(const AdditionFunctionComponentPtr& expr, int exp);

/*	
	The inverse of the Distribute() function, will transform expressions such as abc+abd-abe -> ab(c+d-e)
	It cannot perform more complex factoring, such as transforming a^2-b^2 -> (a+b)(a-b)

	If it could not factor then it returns the original expression.
*/
FunctionComponentPtr FactorBasic(const ConstAdditionFunctionComponentPtr& expr);

/*
	The same as FactorBasic(expr) except this function will only attempt to factor out the terms listed
	in the vector, in the order that they are given.
*/
FunctionComponentPtr FactorBasic(const ConstAdditionFunctionComponentPtr& expr, const std::vector<FunctionCallFunctionComponentPtr>& terms);

/*
	Will combine multiple rational expressions into one rational expression. For example, 1/x + x/(x+1)^2 -> (2x^2+2x+1)/(x(x+1)^2)
*/
FunctionComponentPtr CombineFractions(const ConstAdditionFunctionComponentPtr& expr);

/*
	Transforms expressions such as a^2(a-b)/(a-b) -> a^2
	Assumes that all symbols are not equal to zero and that DistributePowers() and CombinePowerTowers() has been 
	called for each exponential product in expr.

	If it could not cancel a product it returns the original expression.
*/
FunctionComponentPtr CombineProducts(const ConstMultiplicationFunctionComponentPtr& expr);

/*
	Transforms expressions such as (ab)^3 -> a^3*b^3

	CombinePowerTowers() should be called first. It does not expand powers of polynomials, i.e. (a+b)^2
*/
FunctionComponentPtr DistributeExponents(const ConstMultiplicationFunctionComponentPtr& expr);

/*
	If it could not distribute a power it returns nullptr.
*/
FunctionComponentPtr DistributeExponents(const ConstExponentationFunctionComponentPtr& expr);

/*
	Transforms expressions such as (a^b)^c -> a^(bc)
*/
ExponentationFunctionComponentPtr CombinePowerTowers(const ConstExponentationFunctionComponentPtr& expr);

FunctionComponentPtr SimplifyConstants(const ConstAdditionFunctionComponentPtr& expr);

FunctionComponentPtr SimplifyConstants(const ConstMultiplicationFunctionComponentPtr& expr);

/*
	CombinePowerTowers() should be called first
*/
FunctionComponentPtr SimplifyConstants(const ConstExponentationFunctionComponentPtr& expr);

#endif // !SIMPLIFY_H_INCLUDED
