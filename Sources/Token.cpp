#include "../Include/Token.hpp"
#include <sstream>

namespace Basic
{
    Token::Token(Type type, const std::string_view value) : type(type), value(value) {}

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
        case Token::Type::Keyword_Ln:
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
