#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "Token.hpp"

namespace def
{
    struct Exception : std::exception
    {
        Exception(const std::string& line, int pos, const std::string& message);

        const char* what() const noexcept override;

    private:
        std::string m_Message;

    };

    struct Exception_Iter
    {
        Exception_Iter(Token::Iter iter, const std::string& message);

        Token::Iter iterator;
        std::string message;

    };
}
