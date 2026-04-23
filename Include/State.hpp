#pragma once

#include "Interpreter.hpp"
#include "Parser.hpp"

namespace Basic
{
	class State
	{
	public:
		State();

		void DoFile(std::string_view path);
		void DoString(std::string_view);

	private:
		Interpreter m_Interpreter;
		Parser m_Parser;

	};
}