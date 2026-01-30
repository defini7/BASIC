#pragma once

#include <deque>
#include <variant>

#include "Operator.hpp"
#include "Parser.hpp"
#include "Token.hpp"
#include "VarStorage.hpp"

namespace def
{
	class Interpreter
	{
	public:
		Interpreter() = default;

		using TokenIter = std::vector<Token>::const_iterator;

	public:
		int Execute(bool isTerminalMode, const std::vector<Token>& tokens);

	private:
		std::pair<Object, Interpreter::TokenIter> ParseExpression(TokenIter iter);

	private:
		TokenIter HandlePrint(TokenIter iter);
		TokenIter HandleInput(TokenIter iter);
		TokenIter HandleCls(TokenIter iter);
		TokenIter HandleLet(TokenIter iter);
		TokenIter HandleGoto(TokenIter iter);
		TokenIter HandleIf(TokenIter iter);
		TokenIter HandleElse(TokenIter iter);
		TokenIter HandleFor(TokenIter iter);
		TokenIter HandleNext(TokenIter iter);
		TokenIter HandleSleep(TokenIter iter);

	private:
		VarStorage m_Variables;
		TokenIter m_EndIter;
		int m_NextLine;

	};
}
