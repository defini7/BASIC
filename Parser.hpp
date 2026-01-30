#pragma once

#include <unordered_map>
#include <string>
#include <list>

#include "Operator.hpp"
#include "Token.hpp"
#include "Guard.hpp"
#include "Exception.hpp"

namespace def
{
    void String_ToLower(std::string& s);

	class Parser
	{
	public:
		Parser() = default;

	public:
		enum class State
		{
			NewToken,
			CompleteToken,
			Literal_NumericBaseUnknown,
			Literal_NumericBase16,
			Literal_NumericBase10,
			Literal_NumericBase8,
			Literal_NumericBase2,
			Literal_String,
			Operator,
			Symbol
		};

		int Tokenise(std::string_view input, std::vector<Token>& tokens);

	public:
		static std::unordered_map<std::string, Operator> s_Operators;

	};
}
