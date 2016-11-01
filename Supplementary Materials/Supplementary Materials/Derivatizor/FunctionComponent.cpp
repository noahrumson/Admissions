// Noah Rubin

#include "FunctionComponent.h"
#include "simplify.h"


FunctionComponent::FunctionComponent() :
	m_children()
{
}


FunctionComponent::~FunctionComponent()
{
	for (FunctionComponentPtr child : m_children) {
		child.reset();
	}
}


bool FunctionComponent::operator==(const FunctionComponent& a) const
{
	if (m_children.size() != a.m_children.size() || !IsSameType(a)) {
		return false;
	}
	for (int i = 0; i < m_children.size(); ++i) {
		if (*m_children[i] != *a.m_children[i]) {
			return false;
		}
	}
	return true;
}


bool FunctionComponent::operator!=(const FunctionComponent& a) const
{
	if (m_children.size() != a.m_children.size() || !IsSameType(a)) {
		return true;
	}
	for (int i = 0; i < m_children.size(); ++i) {
		if (*m_children[i] != *a.m_children[i]) {
			return true;
		}
	}
	return false;
}

FunctionComponentPtr FunctionComponent::Simplify(bool distribute) const
{
	FunctionComponentPtr temp = SimplifyConstants();
	FunctionComponentPtr func = (temp) ? temp : ShallowClone();
	temp = func->CombineProducts();					func = (temp) ? temp : func;
	temp = func->Distribute(distribute);			func = (temp) ? temp : func;
	temp = func->DistributeExponents();				func = (temp) ? temp : func;
	temp = func->SimplifyConstants();				func = (temp) ? temp : func;
	temp = func->CombineProducts();					func = (temp) ? temp : func;
	temp = func->FactorBasic();						func = (temp) ? temp : func;
	if (distribute) {
		temp = func->Distribute(!EXPAND_EXPONENTS);	func = (temp) ? temp : func;
	}
	temp = func->SimplifyConstants();				func = (temp) ? temp : func;
	return func;
}

FunctionComponentPtr FunctionComponent::Distribute(bool expandPowers) const
{
	std::vector<FunctionComponentPtr> children(m_children.size());
	bool anyDifferent = false;
	for (int i = 0; i < m_children.size(); ++i) {
		FunctionComponentPtr dist = m_children[i]->Distribute(expandPowers);
		if (dist) {
			anyDifferent = true;
			children[i] = dist;
		}
		else {
			children[i] = m_children[i];
		}
	}
	FunctionComponentPtr result = nullptr;
	FunctionComponentPtr dist;
	if (anyDifferent) {
		result = ShallowClone();
		result->GetChildren().clear();
		for (const FunctionComponentPtr& c : children) {
			result->AddChild(c);	// don't just swap b/c must check for same type
		}
		dist = result->DoDistribute(expandPowers);
	}
	else {
		dist = DoDistribute(expandPowers);
	}
	return (dist) ? dist : result;
}

FunctionComponentPtr FunctionComponent::FactorBasic() const
{
	std::vector<FunctionComponentPtr> children(m_children.size());
	bool anyDifferent = false;
	for (int i = 0; i < m_children.size(); ++i) {
		FunctionComponentPtr dist = m_children[i]->FactorBasic();
		if (dist) {
			anyDifferent = true;
			children[i] = dist;
		}
		else {
			children[i] = m_children[i];
		}
	}
	FunctionComponentPtr result = nullptr;
	FunctionComponentPtr dist;
	if (anyDifferent) {
		result = ShallowClone();
		result->GetChildren().clear();
		for (const FunctionComponentPtr& c : children) {
			result->AddChild(c);	// don't just swap b/c must check for same type
		}
		dist = result->DoFactorBasic();
	}
	else {
		dist = DoFactorBasic();
	}
	return (dist) ? dist : result;
}

FunctionComponentPtr FunctionComponent::DistributeExponents() const
{
	std::vector<FunctionComponentPtr> children(m_children.size());
	bool anyDifferent = false;
	for (int i = 0; i < m_children.size(); ++i) {
		FunctionComponentPtr dist = m_children[i]->DistributeExponents();
		if (dist) {
			anyDifferent = true;
			children[i] = dist;
		}
		else {
			children[i] = m_children[i];
		}
	}
	FunctionComponentPtr result = nullptr;
	FunctionComponentPtr dist;
	if (anyDifferent) {
		result = ShallowClone();
		result->GetChildren().clear();
		for (const FunctionComponentPtr& c : children) {
			result->AddChild(c);	// don't just swap b/c must check for same type
		}
		dist = result->DoDistributeExponents();
	}
	else {
		dist = DoDistributeExponents();
	}
	return (dist) ? dist : result;
}

FunctionComponentPtr FunctionComponent::CombineProducts() const
{
	std::vector<FunctionComponentPtr> children(m_children.size());
	bool anyDifferent = false;
	for (int i = 0; i < m_children.size(); ++i) {
		FunctionComponentPtr dist = m_children[i]->CombineProducts();
		if (dist) {
			anyDifferent = true;
			children[i] = dist;
		}
		else {
			children[i] = m_children[i];
		}
	}
	FunctionComponentPtr result = nullptr;
	FunctionComponentPtr dist;
	if (anyDifferent) {
		result = ShallowClone();
		result->GetChildren().clear();
		for (const FunctionComponentPtr& c : children) {
			result->AddChild(c);	// don't just swap b/c must check for same type
		}
		dist = result->DoCombineProducts();
	}
	else {
		dist = DoCombineProducts();
	}
	return (dist) ? dist : result;
}

FunctionComponentPtr FunctionComponent::SimplifyConstants() const
{
	std::vector<FunctionComponentPtr> children(m_children.size());
	bool anyDifferent = false;
	for (int i = 0; i < m_children.size(); ++i) {
		FunctionComponentPtr dist = m_children[i]->SimplifyConstants();
		if (dist) {
			anyDifferent = true;
			children[i] = dist;
		}
		else {
			children[i] = m_children[i];
		}
	}
	FunctionComponentPtr result = nullptr;
	FunctionComponentPtr dist;
	if (anyDifferent) {
		result = ShallowClone();
		result->GetChildren().clear();
		for (const FunctionComponentPtr& c : children) {
			result->AddChild(c);	// don't just swap b/c must check for same type
		}
		dist = result->DoSimplifyConstants();
	}
	else {
		dist = DoSimplifyConstants();
	}
	return (dist) ? dist : result;
}

FunctionComponentPtr FunctionComponent::DoDistribute(bool expandPowers) const
{
	return nullptr;
}

FunctionComponentPtr FunctionComponent::DoFactorBasic() const
{
	return nullptr;
}

FunctionComponentPtr FunctionComponent::DoDistributeExponents() const
{
	return nullptr;
}

FunctionComponentPtr FunctionComponent::DoCombineProducts() const
{
	return nullptr;
}

FunctionComponentPtr FunctionComponent::DoSimplifyConstants() const
{
	return nullptr;
}

void FunctionComponent::DoSort()
{
}

bool FunctionComponent::IsConstant() const
{
	return false;
}


int FunctionComponent::Precedence() const
{
	return 0;
}


Associativities FunctionComponent::Associativity() const
{
	return Associativities::NONE;
}


int FunctionComponent::Operands() const
{
	return 0;
}

void FunctionComponent::Sort()
{
	DoSort();
	for (FunctionComponentPtr c : m_children) {
		c->Sort();
	}
}


const std::vector<FunctionComponentPtr>& FunctionComponent::GetChildren() const
{
	return m_children;
}


std::vector<FunctionComponentPtr>& FunctionComponent::GetChildren()
{
	return m_children;
}


void FunctionComponent::AddChild(const FunctionComponentPtr& child)
{
	if (!TestAddSameType(child)) {
		m_children.push_back(child);
	}
}

void FunctionComponent::AddInverseChild(const FunctionComponentPtr& child)
{
	std::cerr << "AddInverseChild not overriden\n";
}


bool FunctionComponent::TestAddSameType(const FunctionComponentPtr& child)
{
	if (IsSameType(*child)) {
		for (const FunctionComponentPtr& c : child->m_children) {
			m_children.push_back(c);
		}
		return true;
	}
	return false;
}


bool FunctionComponent::IsSameType(const ConstantFunctionComponent& comp) const { return false; }
bool FunctionComponent::IsSameType(const AdditionFunctionComponent& comp) const { return false; }
bool FunctionComponent::IsSameType(const MultiplicationFunctionComponent& comp) const { return false; }
bool FunctionComponent::IsSameType(const ExponentationFunctionComponent& comp) const { return false; }
bool FunctionComponent::IsSameType(const FunctionCallFunctionComponent& comp) const { return false; }
bool FunctionComponent::IsSameType(const VariableFunctionComponent& comp) const { return false; }
