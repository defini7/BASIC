#pragma once

#include <string>
#include <vector>

namespace Basic
{
    struct Token
	{
        using Iter = std::vector<Token>::const_iterator;

		enum class Type
		{
			None,
			Literal_NumericBaseUnknown,
			Literal_NumericBase16,
			Literal_NumericBase10,
			Literal_NumericBase8,
			Literal_NumericBase2,
			Literal_String,
			Symbol,
			Colon,
			Semicolon,
			Operator,
			Parenthesis_Open,
			Parenthesis_Close,

			Keyword_Print,
			Keyword_Input,
			Keyword_Cls,
			Keyword_Let,
			Keyword_Rem,
			Keyword_Goto,
			Keyword_If,
			Keyword_Then,
			Keyword_Else,
			Keyword_For,
			Keyword_To,
			Keyword_Step,
			Keyword_Next,
			Keyword_Sleep,
			Keyword_Sin,
			Keyword_Cos,
			Keyword_Tan,
			Keyword_ArcSin,
			Keyword_ArcCos,
			Keyword_ArcTan,
			Keyword_Sqrt,
			Keyword_Log,
			Keyword_Exp,
			Keyword_Abs,
			Keyword_Sign,
			Keyword_Int,
            Keyword_Random,
            Keyword_End,
            Keyword_Val,
            Keyword_GoSub,
            Keyword_Return,

			Keyword_List,
			Keyword_Run,
            Keyword_New,
            Keyword_Load
		};

		Token() = default;
		Token(Type type, const std::string_view value = "");

		std::string ToString() const;
        bool IsFunction() const;

		Type type = Type::None;
		std::string value;

    };

    std::string TokensToString(const std::vector<Token>& tokens);
}
