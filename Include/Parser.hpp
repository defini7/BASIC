#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "Operator.hpp"
#include "Token.hpp"
#include "Guard.hpp"
#include "Exception.hpp"

namespace Basic
{
    void String_ToUpper(std::string& s);

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

		// Splits input into tokens and returns line number, -1 if no line was specified
        void Tokenise(const std::string& input, std::vector<Token>& tokens);

	public:
		static std::unordered_map<std::string, Operator> s_Operators;

	};
}
