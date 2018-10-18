// Noah Rubin

/*
	The program breaks down expressions by creating a tree. For example, if the
	user enters "x^2 + 3*sin(x)", then this tree will be built:
	               +
				/     \
			 ^          *
		   /   \      /   \
		  x     2    3     sin
							|
							x

	For user inputs this tree is built during the parsing phase. See parser.cpp
*/

#ifndef FUNCTION_COMPONENT_H_INCLUDED
#define FUNCTION_COMPONENT_H_INCLUDED

#include <vector>
#include <string>
#include <memory>

#include "PoolAllocator.h"
#include "defines.h"


typedef double (*EvaluableFunction)(double);

class FunctionComponent;
class ConstantFunctionComponent;
class AdditionFunctionComponent;
class MultiplicationFunctionComponent;
class ExponentationFunctionComponent;
class FunctionCallFunctionComponent;
class VariableFunctionComponent;

typedef std::shared_ptr<FunctionComponent>					FunctionComponentPtr;
typedef std::shared_ptr<ConstantFunctionComponent>			ConstantFunctionComponentPtr;
typedef std::shared_ptr<AdditionFunctionComponent>			AdditionFunctionComponentPtr;
typedef std::shared_ptr<MultiplicationFunctionComponent>	MultiplicationFunctionComponentPtr;
typedef std::shared_ptr<ExponentationFunctionComponent>		ExponentationFunctionComponentPtr;
typedef std::shared_ptr<FunctionCallFunctionComponent>		FunctionCallFunctionComponentPtr;
typedef std::shared_ptr<VariableFunctionComponent>			VariableFunctionComponentPtr;

typedef std::shared_ptr<const FunctionComponent>				ConstFunctionComponentPtr;
typedef std::shared_ptr<const ConstantFunctionComponent>		ConstConstantFunctionComponentPtr;
typedef std::shared_ptr<const AdditionFunctionComponent>		ConstAdditionFunctionComponentPtr;
typedef std::shared_ptr<const MultiplicationFunctionComponent>	ConstMultiplicationFunctionComponentPtr;
typedef std::shared_ptr<const ExponentationFunctionComponent>	ConstExponentationFunctionComponentPtr;
typedef std::shared_ptr<const FunctionCallFunctionComponent>	ConstFunctionCallFunctionComponentPtr;
typedef std::shared_ptr<const VariableFunctionComponent>		ConstVariableFunctionComponentPtr;

enum class Associativities
{
	NONE,
	LEFT,
	RIGHT
};


enum MultiplicationOrder
{
	ORDER_CONSTANT,
	ORDER_EXPONENTIAL,
	ORDER_ADDITION,
	ORDER_VARIABLE,
	ORDER_LOG,
	ORDER_SINE,
	ORDER_TRIG_FUNCTION
};

class FunctionComponent
{
public:
	FunctionComponent();

	virtual ~FunctionComponent();

	virtual bool operator==(const FunctionComponent& a) const;

	virtual bool operator!=(const FunctionComponent& a) const;

	static const bool DISTRIBUTE	= true;
	static const bool FACTOR		= false;

	FunctionComponentPtr Simplify(bool distribute) const;

	virtual FunctionComponentPtr Differentiate() const = 0;

	virtual FunctionComponentPtr ShallowClone() const = 0;

	virtual std::string Print(bool parentheses = false) const = 0;

	virtual int Precedence() const;

	virtual Associativities Associativity() const;

	virtual int Operands() const;

	void Sort();

	virtual int AdditionOrder() const = 0;

	virtual int MultiplicationOrder() const = 0;

	virtual bool IsConstant() const;

	const std::vector<FunctionComponentPtr>& GetChildren() const;

	std::vector<FunctionComponentPtr>& GetChildren();

	virtual void AddChild(const FunctionComponentPtr& child);

	virtual void AddInverseChild(const FunctionComponentPtr& child);

	virtual bool IsSameType(const FunctionComponent& comp) const = 0;
	virtual bool IsSameType(const ConstantFunctionComponent& comp) const;
	virtual bool IsSameType(const AdditionFunctionComponent& comp) const;
	virtual bool IsSameType(const MultiplicationFunctionComponent& comp) const;
	virtual bool IsSameType(const ExponentationFunctionComponent& comp) const;
	virtual bool IsSameType(const FunctionCallFunctionComponent& comp) const;
	virtual bool IsSameType(const VariableFunctionComponent& comp) const;

	FunctionComponentPtr Distribute(bool expandPower) const;

	FunctionComponentPtr FactorBasic() const;

	FunctionComponentPtr DistributeExponents() const;

	FunctionComponentPtr CombineProducts() const;

	FunctionComponentPtr SimplifyConstants() const;

private:
	std::vector<FunctionComponentPtr> m_children;

	virtual FunctionComponentPtr DoDistribute(bool expandPower) const;

	virtual FunctionComponentPtr DoFactorBasic() const;

	virtual FunctionComponentPtr DoDistributeExponents() const;

	virtual FunctionComponentPtr DoCombineProducts() const;

	virtual FunctionComponentPtr DoSimplifyConstants() const;

	virtual void DoSort();

	virtual bool TestAddSameType(const FunctionComponentPtr& child);
};


template<class T>
class FunctionComponentImpl : 
	public FunctionComponent, 
	public std::enable_shared_from_this<T>
{
public:
	void* operator new(std::size_t size);

	void operator delete(void* ptr, std::size_t size);

	virtual FunctionComponentPtr ShallowClone() const override;

	virtual bool IsSameType(const FunctionComponent& comp) const override final;

	virtual bool IsSameType(const T& comp) const override final;

private:
	static PoolAllocator<T> sm_allocator;
};


class ConstantFunctionComponent : public FunctionComponentImpl<ConstantFunctionComponent>
{
public:
	ConstantFunctionComponent();

	explicit ConstantFunctionComponent(double val);

	virtual bool operator==(const FunctionComponent& a) const override;

	virtual bool operator!=(const FunctionComponent& a) const override;

	virtual FunctionComponentPtr Differentiate() const override;

	virtual std::string Print(bool parentheses = false) const override;

	virtual bool IsConstant() const override;

	virtual int AdditionOrder() const override;

	virtual int MultiplicationOrder() const override;

	double GetValue() const;

private:
	const double m_val;
};


class VariableFunctionComponent : public FunctionComponentImpl<VariableFunctionComponent>
{
public:
	virtual FunctionComponentPtr Differentiate() const override;

	virtual int AdditionOrder() const override;

	virtual int MultiplicationOrder() const override;

	virtual std::string Print(bool parentheses = false) const override;

private:
	static VariableFunctionComponent* sm_singleton;
};


class AdditionFunctionComponent : public FunctionComponentImpl<AdditionFunctionComponent>
{
public:
	virtual FunctionComponentPtr Differentiate() const override;

	virtual std::string Print(bool parentheses = false) const override;

	virtual void AddInverseChild(const FunctionComponentPtr& child) override;

	virtual int Precedence() const override;

	virtual Associativities Associativity() const override;

	virtual int Operands() const override;

	virtual int AdditionOrder() const override;

	virtual int MultiplicationOrder() const override;

private:
	virtual FunctionComponentPtr DoFactorBasic() const override;

	virtual FunctionComponentPtr DoSimplifyConstants() const override;

	virtual void DoSort() override;
};


class MultiplicationFunctionComponent : public FunctionComponentImpl<MultiplicationFunctionComponent>
{
public:
	virtual FunctionComponentPtr Differentiate() const override;

	virtual void AddInverseChild(const FunctionComponentPtr& child) override;

	MultiplicationFunctionComponentPtr GetNumerator() const;

	std::vector<FunctionComponentPtr> GetDenominator() const;

	virtual std::string Print(bool parentheses = false) const override;

	std::string PrintNoMinus(bool parentheses) const;

	virtual int Precedence() const override;

	virtual Associativities Associativity() const override;

	virtual int Operands() const override;

	bool HasNegative() const;

	FunctionComponentPtr WithoutNegative() const;

	virtual int AdditionOrder() const override;

	virtual int MultiplicationOrder() const override;

private:
	virtual FunctionComponentPtr DoDistribute(bool expandPower) const override;

	virtual FunctionComponentPtr DoDistributeExponents() const override;

	virtual FunctionComponentPtr DoCombineProducts() const override;

	virtual FunctionComponentPtr DoSimplifyConstants() const override;

	virtual void DoSort() override;
};


class ExponentationFunctionComponent : public FunctionComponentImpl<ExponentationFunctionComponent>
{
public:
	using FunctionComponentImpl::FunctionComponentImpl;

	virtual FunctionComponentPtr Differentiate() const override;

	virtual int Precedence() const override;

	virtual Associativities Associativity() const override;

	virtual int Operands() const override;

	virtual void AddChild(const FunctionComponentPtr& child) override;

	FunctionComponentPtr WithoutNegative() const;

	virtual int AdditionOrder() const override;

	virtual int MultiplicationOrder() const override;

	virtual std::string Print(bool parentheses = false) const override;

	std::string PrintNoMinus(int precedence) const;

	bool HasNegativeExponent() const;

private:
	virtual bool TestAddSameType(const FunctionComponentPtr& comp) override;

	virtual FunctionComponentPtr DoSimplifyConstants() const override;

	virtual FunctionComponentPtr DoDistribute(bool expandPowers) const override;

	virtual FunctionComponentPtr DoDistributeExponents() const override;
};


class FunctionCallFunctionComponent : public FunctionComponentImpl<FunctionCallFunctionComponent>
{
public:
	explicit FunctionCallFunctionComponent(EvaluableFunction func);

	virtual bool operator==(const FunctionComponent& a) const override;

	virtual bool operator!=(const FunctionComponent& a) const override;

	virtual FunctionComponentPtr Differentiate() const override;

	virtual std::string Print(bool parentheses = false) const override;

	virtual int Precedence() const override;

	virtual Associativities Associativity() const override;

	virtual void AddChild(const FunctionComponentPtr& child) override;

	virtual int Operands() const override;

	virtual int AdditionOrder() const override;

	virtual int MultiplicationOrder() const override;

	void SetParentheses(bool b);

private:
	EvaluableFunction m_func;
	mutable bool m_hasParentheses;

	virtual FunctionComponentPtr DoSimplifyConstants() const override;
};


inline double ConstantFunctionComponent::GetValue() const
{
	return m_val;
}


inline void FunctionCallFunctionComponent::SetParentheses(bool b)
{
	m_hasParentheses = b;
}


template<class T>
PoolAllocator<T> FunctionComponentImpl<T>::sm_allocator;


template<class T>
inline void* FunctionComponentImpl<T>::operator new(std::size_t size)
{
	return sm_allocator.Allocate();
}

template<class T>
inline void FunctionComponentImpl<T>::operator delete(void* ptr, std::size_t size)
{
	sm_allocator.Destroy(ptr);
}

template<class T>
inline bool FunctionComponentImpl<T>::IsSameType(const FunctionComponent& comp) const
{
	return comp.IsSameType(static_cast<const T&>(*this));
}


template<class T>
inline bool FunctionComponentImpl<T>::IsSameType(const T& comp) const
{
	return true;
}

template<class T>
inline FunctionComponentPtr FunctionComponentImpl<T>::ShallowClone() const
{
	return std::make_shared<T>(*static_cast<const T*>(this));
}


#endif // !FUNCTION_COMPONENT_H_INCLUDED
