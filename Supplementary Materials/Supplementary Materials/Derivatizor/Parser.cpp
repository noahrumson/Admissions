// Noah Rubin

#include <vector>
#include <map>
#include <cmath>
#include <cstring>
#include <stack>
#include <algorithm>

#include "Parser.h"
#include "FunctionComponent.h"
#include "FunctionCallDerivative.h"

int maxDigits = 0;

namespace
{
	struct Token
	{
		enum TokenTypes
		{
			NUMBER,
			VARIABLE,
			FUNCTION,
			OPERATION,
			LEFT_PARENTHESES,
			RIGHT_PARENTHESES
		};
		FunctionComponentPtr fcomp;
		double dval;
		TokenTypes type;
		bool inverse;

		Token();
		Token(const Token& copy);
		Token(Token&& move);
		Token& operator=(const Token& copy);
		Token& operator=(Token&& move);
	};


	Token::Token() :
		dval(0.0),
		fcomp(nullptr),
		type(NUMBER),
		inverse(false)
	{
	}

	Token::Token(const Token& copy) :
		dval(0.0),
		fcomp(nullptr),
		type(copy.type),
		inverse(copy.inverse)
	{
		if (type == VARIABLE || type == FUNCTION || type == OPERATION) {
			fcomp = copy.fcomp;
		}
		else {
			dval = copy.dval;
		}
	}

	
	Token::Token(Token&& move) :
		dval(0.0),
		fcomp(nullptr),
		type(move.type),
		inverse(move.inverse)
	{
		if (type == VARIABLE || type == FUNCTION || type == OPERATION) {
			fcomp = move.fcomp;
		}
		else {
			dval = move.dval;
		}
		move.fcomp = nullptr;
	}


	Token& Token::operator=(const Token& copy)
	{
		type = copy.type;
		inverse = copy.inverse;
		if (type == VARIABLE || type == FUNCTION || type == OPERATION) {
			fcomp = copy.fcomp;
		}
		else {
			dval = copy.dval;
		}
		return *this;
	}


	Token& Token::operator=(Token&& move)
	{
		type = move.type;
		inverse = move.inverse;
		if (type == VARIABLE || type == FUNCTION || type == OPERATION) {
			fcomp = move.fcomp;
		}
		else {
			dval = move.dval;
		}
		move.fcomp = nullptr;
		return *this;
	}

	struct TokenPath
	{
		TokenPath() = default;
		TokenPath(char c, bool stop, const std::vector<TokenPath>& paths);

		char c;
		bool stop;
		std::vector<TokenPath> paths;
	};

	TokenPath::TokenPath(char c, bool stop, const std::vector<TokenPath>& paths) :
		c(c),
		stop(stop),
		paths(paths)
	{
	}

	std::map<std::string, EvaluableFunction> functionMap {
			{ "log", &log }, { "ln", &log }, { "exp", &exp },
			{ "sin", &sin }, { "cos", &cos }, { "tan", &tan },
			{ "csc", &csc }, { "sec", &sec }, { "cot", &cot },
			{ "asin", &asin }, { "arcsin", &asin }, { "sin^-1", &asin },
			{ "acos", &acos }, { "arccos", &acos }, { "cos^-1", &acos },
			{ "atan", &atan }, { "arctan", &atan }, { "tan^-1", &atan },
			{ "sqrt", &sqrt }
	};
	
	std::string readThisToken;
	TokenPath* currentToken;
	int fallbackIndex;


	void InsertWithLetter(std::vector<std::vector<std::string>>& vec, const std::string& str)
	{
		for (int i = 0; i < vec.size(); ++i) {
			if (vec[i][0][0] == str[0]) {
				vec[i].push_back(str);
				return;
			}
		}
		vec.push_back({ str });
	}


	bool HasLengthOneElement(const std::vector<std::string>& vec) 
	{
		for (const std::string& str : vec) {
			if (str.size() == 1) {
				return true;
			}
		}
		return false;
	}


	void RemoveFirstLetter(std::vector<std::string>& vec)
	{
		auto it = vec.begin();
		while (it != vec.end()) {
			it->erase(it->begin());
			if (it->size() == 0) {
				it = vec.erase(it);
			}
			else {
				++it;
			}
		}
	}

	std::vector<TokenPath> GenerateTokenPaths(const std::vector<std::string>& names)
	{
		std::vector<std::vector<std::string>> sameFirstLetter;
		for (const std::string& str : names) {
			InsertWithLetter(sameFirstLetter, str);
		}
		std::vector<TokenPath> res(sameFirstLetter.size());
		for (int i = 0; i < res.size(); ++i) {
			res[i].c = sameFirstLetter[i][0][0];
			res[i].stop = HasLengthOneElement(sameFirstLetter[i]);
			RemoveFirstLetter(sameFirstLetter[i]);
			res[i].paths = GenerateTokenPaths(sameFirstLetter[i]);
		}
		return res;
	}


	bool IsPiOrE(const Token& t)
	{
		return t.type == Token::NUMBER && (t.dval == M_E || t.dval == M_PI);
	}

	
	bool IsVariable(char c)
	{
		return c == 'x' || c == 'X';
	}


	bool IsFunction(char c, Token& token)
	{
		static std::vector<TokenPath> paths = GenerateTokenPaths ({
			"log", "ln", "exp",
			"sin", "cos", "tan",
			"csc", "sec", "cot",
			"asin", "arcsin", "sin^-1",
			"acos", "arccos", "cos^-1",
			"atan", "arctan", "tan^-1",
			"sqrt"
		});
		std::vector<TokenPath>& search = (currentToken ? currentToken->paths : paths);
		for (TokenPath& p : search) {
			if (p.c == c) {
				readThisToken += c;
				currentToken = &p;
				if (currentToken->stop) {
					fallbackIndex = readThisToken.size() - 1;
				}
				if (currentToken->paths.empty()) {
					token.fcomp = std::make_shared<FunctionCallFunctionComponent>(functionMap[readThisToken]);
					token.type = Token::FUNCTION;
					fallbackIndex = 0;
					currentToken = nullptr;
				}
				return true;
			}
		}
		if (fallbackIndex) {
			readThisToken.erase(fallbackIndex + 1);
			token.fcomp = std::make_shared<FunctionCallFunctionComponent>(functionMap[readThisToken]);
			token.type = Token::FUNCTION;
			fallbackIndex = 0;
			currentToken = nullptr;
			return true;
		}
		currentToken = nullptr;
		readThisToken.clear();
		return false;
	}


	bool IsOperator(char c, Token& token)
	{
		switch (c) {
		case '+': 
			token.fcomp = std::make_shared<AdditionFunctionComponent>();
			token.type = Token::OPERATION;
			return true;
		case '-': 
			token.fcomp = std::make_shared<AdditionFunctionComponent>();
			token.inverse = true;
			token.type = Token::OPERATION;
			return true;
		case '*': 
			token.fcomp = std::make_shared<MultiplicationFunctionComponent>();
			token.type = Token::OPERATION;
			return true;
		case '/': 
			token.fcomp = std::make_shared<MultiplicationFunctionComponent>();
			token.inverse = true;
			token.type = Token::OPERATION;
			return true;
		case '^': 
			token.fcomp = std::make_shared<ExponentationFunctionComponent>();
			token.type = Token::OPERATION;
			return true;
		case '(':
			token.type = Token::LEFT_PARENTHESES;
			return true;
		case ')':
			token.type = Token::RIGHT_PARENTHESES;
			return true;
		default: return false;
		}
	}


	bool ParseToken(const std::string& str, Token& t, int& advance)
	{
		int i = 0;
		while (str[i] == ' ') {
			++i;
		}
		int k = i;
		while (IsFunction(str[k], t) && !t.fcomp) {
			++k;
		}
		if (k != i && t.fcomp) {
			advance = i + readThisToken.size();
			readThisToken.clear();
			return true;
		}
		else if (IsOperator(str[i], t)) {
			advance = i + 1;
			return true;
		}
		else if (IsVariable(str[i])) {
			t.fcomp = std::make_shared<VariableFunctionComponent>();
			t.type = Token::VARIABLE;
			advance = i + 1;
			return true;
		}
		else if (str[i] == 'e') {
			t.dval = M_E;
			t.type = Token::NUMBER;
			advance = i + 1;
			return true;
		}
		else if (i + 1 < str.size() && str[i] == 'p' && str[i + 1] == 'i') {
			t.dval = M_PI;
			t.type = Token::NUMBER;
			advance = i + 2;
			return true;
		}
		char* firstNotNumerical;
		double num = strtod(&str[0], &firstNotNumerical);
		if (firstNotNumerical != &str[0]) {	// is number
			t.dval = num;
			t.type = Token::NUMBER;
			advance = firstNotNumerical - &str[0];
			const char* dummy = firstNotNumerical;
			if (std::find_if(&str[0], dummy, [](char c) { return c == '.' || c == ','; }) != dummy) {
				maxDigits = std::max(maxDigits, advance - 1);
			}
			else {
				maxDigits = std::max(maxDigits, advance);
			}
			return true;
		}
		return false;
	}


	bool IsImplicitMultiplication(const Token& t1, const Token& t2)
	{
		switch (t1.type) {
		case Token::NUMBER: return (t2.type == Token::FUNCTION || t2.type == Token::VARIABLE || t2.type == Token::LEFT_PARENTHESES || 
									(t2.type == Token::NUMBER && (IsPiOrE(t1) || IsPiOrE(t2))));
		case Token::VARIABLE: return (t2.type == Token::FUNCTION || t2.type == Token::LEFT_PARENTHESES || t2.type == Token::VARIABLE);
		case Token::RIGHT_PARENTHESES: return (t2.type != Token::RIGHT_PARENTHESES && t2.type != Token::OPERATION);
		default: return false;
		}
	}


	bool AllWhitespace(const std::string& str)
	{
		return str.find_first_not_of(' ') == std::string::npos;
	}


	bool Tokenize(std::string str, std::vector<Token>& tokens)
	{
		int advance;
		int i = 0;
		while (str.size() > 0 && !AllWhitespace(str)) {
			tokens.push_back(Token());
			if (!ParseToken(str, tokens[i], advance)) {
				return false;
			}
			str = str.substr(advance);
			++i;
		}
		for (int i = 0; i < tokens.size() - 1; ++i) {
			if (IsImplicitMultiplication(tokens[i], tokens[i + 1])) {
				tokens.insert(tokens.begin() + i + 1, Token());
				tokens[i + 1].fcomp = std::make_shared<MultiplicationFunctionComponent>();
				tokens[i + 1].type = Token::OPERATION;
			}
			if (tokens[i].type == Token::OPERATION && tokens[i].fcomp->IsSameType(AdditionFunctionComponent()) && tokens[i].inverse) {
				if (tokens[i + 1].type == Token::VARIABLE || tokens[i + 1].type == Token::FUNCTION || tokens[i + 1].type == Token::LEFT_PARENTHESES) {
					if (i > 0 && (tokens[i - 1].type == Token::VARIABLE || tokens[i - 1].type == Token::NUMBER || tokens[i - 1].type == Token::RIGHT_PARENTHESES)) {
						continue;
					}
					tokens[i].dval = -1.0;
					tokens[i].type = Token::NUMBER;
					tokens.insert(tokens.begin() + i + 1, Token());
					tokens[i + 1].fcomp = std::make_shared<MultiplicationFunctionComponent>();
					tokens[i + 1].type = Token::OPERATION;
				}
				if ((i > 0 && (tokens[i - 1].type == Token::FUNCTION || tokens[i - 1].type == Token::LEFT_PARENTHESES)) && tokens[i + 1].type == Token::NUMBER) {	// don't question it
					tokens[i + 1].dval = -tokens[i + 1].dval;
					tokens.erase(tokens.begin() + i);
					--i;
				}
			}
		}
		if (tokens.size() >= 2 && tokens[0].fcomp && tokens[0].fcomp->IsSameType(AdditionFunctionComponent()) && tokens[1].type == Token::NUMBER) {	// negative number first
			tokens[1].dval = -tokens[1].dval;
			tokens.erase(tokens.begin());
		}
		return true;
	}


	bool SemanticCheck(std::vector<Token>& tokens)
	{
		for (int i = 0; i < tokens.size() - 1; ++i) {
			if (tokens[i].type == Token::OPERATION && tokens[i + 1].type == Token::OPERATION) {
				return false;
			}
			else if (tokens[i].type == Token::NUMBER && tokens[i + 1].type == Token::NUMBER && !(IsPiOrE(tokens[i]) || IsPiOrE(tokens[i + 1]))) {
				return false;
			}
			else if (tokens[i].type == Token::LEFT_PARENTHESES && tokens[i + 1].type == Token::RIGHT_PARENTHESES) {
				return false;
			}
			else if (tokens[i].type == Token::FUNCTION && tokens[i + 1].type == Token::OPERATION && !tokens[i + 1].fcomp->IsSameType(ExponentationFunctionComponent())) {
				return false;
			}
		}
		if (tokens.back().type == Token::OPERATION || tokens.back().type == Token::FUNCTION) {
			return false;
		}
		return true;
	}


	bool MakeNode(std::stack<FunctionComponentPtr>& stack, FunctionComponentPtr op, bool inverse)
	{
		if (op->Operands() == 2) {
			FunctionComponentPtr c2 = stack.top();
			stack.pop();
			if (stack.empty()) {
				return false;
			}
			FunctionComponentPtr c1 = stack.top();
			stack.pop();
			stack.push(op);
			stack.top()->AddChild(c1);
			if (inverse) {
				stack.top()->AddInverseChild(c2);
			}
			else {
				stack.top()->AddChild(c2);
			}
		}
		else if (op->Operands() == 1) {
			FunctionComponentPtr c1 = stack.top();
			stack.pop();
			stack.push(op);
			stack.top()->AddChild(c1);
		}
		return true;
	}
}


FunctionComponentPtr ParseExpression(const std::string& str)
{
	std::stack<Token> operatorStack;
	std::stack<FunctionComponentPtr> operandStack;
	std::vector<Token> tokens;
	if (!Tokenize(str, tokens)) {
		return nullptr;
	}
	if (!SemanticCheck(tokens)) {
		return nullptr;
	}
	for (auto it = tokens.begin(); it != tokens.end(); ++it) {
		const Token& t = *it;
		switch (t.type) {
		case Token::NUMBER:
			operandStack.push(std::make_shared<ConstantFunctionComponent>(t.dval));
			break;
		case Token::VARIABLE:
			operandStack.push(t.fcomp);
			break;
		case Token::FUNCTION:
			operatorStack.push(t);
			break;
		case Token::OPERATION:
			while (!operatorStack.empty() && operatorStack.top().type != Token::LEFT_PARENTHESES && 
				((t.fcomp->Associativity() == Associativities::LEFT || operatorStack.top().type == Token::FUNCTION) && 
				t.fcomp->Precedence() <= operatorStack.top().fcomp->Precedence()))
			{
				FunctionComponentPtr comp = operatorStack.top().fcomp;
				bool inverse = operatorStack.top().inverse;
				operatorStack.pop();
				MakeNode(operandStack, comp, inverse);
			}
			operatorStack.push(t);
			break;
		case Token::LEFT_PARENTHESES:
			if (!operatorStack.empty() && operatorStack.top().type == Token::FUNCTION) {
				std::static_pointer_cast<FunctionCallFunctionComponent>(operatorStack.top().fcomp)->SetParentheses(true);
			}
			operatorStack.push(t);
			break;
		case Token::RIGHT_PARENTHESES:
			while (!operatorStack.empty() && operatorStack.top().type != Token::LEFT_PARENTHESES) {
				FunctionComponentPtr comp = operatorStack.top().fcomp;
				bool inverse = operatorStack.top().inverse;
				operatorStack.pop();
				MakeNode(operandStack, comp, inverse);
			}
			if (operatorStack.empty()) {
				return nullptr;	// mismatched parentheses
			}
			else {
				operatorStack.pop(); // pop the left parentheses
			}
			break;
		}
	}
	while (!operatorStack.empty()) {
		if (!operatorStack.top().fcomp) {
			std::cout << "Check for mismatched parentheses\n";
			return nullptr;
		}
		if (!MakeNode(operandStack, operatorStack.top().fcomp, operatorStack.top().inverse)) {
			std::cout << "Enter expressions such as sin^2(x) as sin(x)^2\n";
			return nullptr;
		}
		operatorStack.pop();
	}
	maxDigits = 0;
	return operandStack.top();
}