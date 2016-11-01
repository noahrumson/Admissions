// Noah Rubin

#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <string>

#include "FunctionComponent.h"

FunctionComponentPtr ParseExpression(const std::string& str);

extern int maxDigits;

#endif