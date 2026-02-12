#include "Include/Exception.hpp"

namespace Basic
{
    Exception::Exception(const std::string& line, int pos, const std::string& message)
	{
        std::stringstream ss;

        ss << "| " << line;

        ss << "\n| ";

        for (int i = 0; i < pos; i++)
            ss << ' ';

        ss << "^\n| ";
        ss << "Error: " << message << " at " << pos;

        m_Message = ss.str();
	}

	const char* Exception::what() const noexcept
	{
        return m_Message.c_str();
	}

    Exception_Iter::Exception_Iter(Token::Iter iter, const std::string& message)
        : iterator(iter), message(message)
    {

    }
}
