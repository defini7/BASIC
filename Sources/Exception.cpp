#include "Include/Exception.hpp"

namespace def
{
    Exception::Exception(std::string_view message)
	{
		m_Message = message;
	}

	const char* Exception::what() const noexcept
	{
		return m_Message.c_str();
	}

    ParserException::ParserException(std::string_view message) : Exception("[Parse Error] " + message)
	{

	}

    InterpreterException::InterpreterException(std::string_view message) : Exception("[Interpret Error] " + message)
	{

	}
}
