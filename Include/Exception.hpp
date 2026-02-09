#pragma once

#include <string>

namespace def
{
    struct Exception : std::exception
    {
        Exception(std::string_view message);

        const char* what() const noexcept override;

    private:
        std::string m_Message;

    };

    struct ParserException : Exception
    {
        ParserException(std::string_view message);
    };

    struct InterpreterException : Exception
    {
        InterpreterException(std::string_view message);
    };
}
