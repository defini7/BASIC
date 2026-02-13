#include "Include/Token.hpp"
#include <sstream>

namespace Basic
{
	Token::Token(Type type, const std::string_view value) : type(type), value(value)
	{

	}

	std::string Token::ToString() const
	{
		std::string tag;

		switch (type)
		{
		case Type::Literal_NumericBase16:  tag = "[Literal, Numeric 16 ] "; break;
		case Type::Literal_NumericBase10:  tag = "[Literal, Numeric 10 ] "; break;
		case Type::Literal_NumericBase8:   tag = "[Literal, Numeric 8  ] "; break;
		case Type::Literal_NumericBase2:   tag = "[Literal, Numeric 2  ] "; break;
		case Type::Literal_String:         tag = "[Literal, String     ] "; break;
		case Type::Symbol:			       tag = "[Symbol              ] "; break;
		case Type::Colon:			       tag = "[Colon               ] "; break;
		case Type::Operator:		       tag = "[Operator            ] "; break;
		case Type::Parenthesis_Open:       tag = "[Parenthesis, Open   ] "; break;
		case Type::Parenthesis_Close:      tag = "[Parenthesis, Close  ] "; break;
		case Type::Keyword_Print:		   tag = "[Keyword, Print      ] "; break;
		case Type::Keyword_Input:		   tag = "[Keyword, Input      ] "; break;
		case Type::Keyword_Cls:            tag = "[Keyword, Cls        ] "; break;
		case Type::Keyword_Let:            tag = "[Keyword, Let        ] "; break;
		case Type::Keyword_Rem:            tag = "[Keyword, Rem        ] "; break;
		case Type::Keyword_Goto:           tag = "[Keyword, Goto       ] "; break;
		case Type::Keyword_If:             tag = "[Keyword, If         ] "; break;
		case Type::Keyword_Then:           tag = "[Keyword, Then       ] "; break;
		case Type::Keyword_Else:           tag = "[Keyword, Else       ] "; break;
		case Type::Keyword_For:            tag = "[Keyword, For        ] "; break;
		case Type::Keyword_To:             tag = "[Keyword, To         ] "; break;
		case Type::Keyword_Step:           tag = "[Keyword, Step       ] "; break;
		case Type::Keyword_Next:           tag = "[Keyword, Next       ] "; break;
		case Type::Keyword_Sleep:          tag = "[Keyword, Sleep      ] "; break;
		case Type::Keyword_Sin:	           tag = "[Keyword, Sin        ] "; break;
		case Type::Keyword_Cos:	           tag = "[Keyword, Cos        ] "; break;
		case Type::Keyword_Tan:	           tag = "[Keyword, Tan        ] "; break;
		case Type::Keyword_ArcSin:         tag = "[Keyword, ArcSin     ] "; break;
		case Type::Keyword_ArcCos:         tag = "[Keyword, ArcCos     ] "; break;
		case Type::Keyword_ArcTan:         tag = "[Keyword, ArcTan     ] "; break;
		case Type::Keyword_Sqrt:           tag = "[Keyword, Sqrt       ] "; break;
		case Type::Keyword_Log:            tag = "[Keyword, Log        ] "; break;
		case Type::Keyword_Exp:            tag = "[Keyword, Exp        ] "; break;
		case Type::Keyword_Abs:            tag = "[Keyword, Abs        ] "; break;
		case Type::Keyword_Sign:           tag = "[Keyword, Sign       ] "; break;
		case Type::Keyword_Int:            tag = "[Keyword, Int        ] "; break;
        case Type::Keyword_Random:         tag = "[Keyword, Random     ] "; break;
		}

		return tag + value;
	}

    bool Token::IsFunction() const
    {
        switch (type)
        {
        case Token::Type::Keyword_Sin:
        case Token::Type::Keyword_Cos:
        case Token::Type::Keyword_Tan:
        case Token::Type::Keyword_ArcSin:
        case Token::Type::Keyword_ArcCos:
        case Token::Type::Keyword_ArcTan:
        case Token::Type::Keyword_Sqrt:
        case Token::Type::Keyword_Log:
        case Token::Type::Keyword_Exp:
        case Token::Type::Keyword_Abs:
        case Token::Type::Keyword_Sign:
        case Token::Type::Keyword_Int:
        case Token::Type::Keyword_Random:
        case Token::Type::Keyword_Val:
            return true;

        default:
            return false;
        }
    }

    std::string TokensToString(const std::vector<Token>& tokens)
    {
        std::stringstream ss;

        for (const auto& token : tokens)
        {
            ss << ' ';

            if (token.type == Basic::Token::Type::Literal_String)
                ss << '"' << token.value << '"';
            else
                ss << token.value;
        }

        return ss.str();
    }
}
