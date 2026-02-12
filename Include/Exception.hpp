#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "Token.hpp"

namespace Basic
{
    struct Exception : std::exception
    {
        Exception(const std::string& line, int pos, const std::string& message);

        const char* what() const noexcept override;

    private:
        std::string m_Message;

    };

    // Not real std-ish exception but helps to transfer iterator
    // and error message between contexts
    struct Exception_Iter
    {
        Exception_Iter(Token::Iter iter, const std::string& message);

        // Iterator to word in code
        Token::Iter iterator;

        // Error message
        std::string message;

    };
}
