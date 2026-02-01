#pragma once

#include <deque>
#include <variant>

#include "Operator.hpp"
#include "Parser.hpp"
#include "Token.hpp"
#include "VarStorage.hpp"

namespace def
{
	using TokenIter = std::vector<Token>::const_iterator;

	struct ForNode
	{
		std::string varName;

		int line;
		int posInLine;

		long double startValue;
		long double endValue;
		long double step;
	};

	class Interpreter
	{
	public:
		Interpreter() = default;

	public:
		int Execute(const std::vector<Token>& tokens, int line = -1);

	private:
		std::pair<Object, TokenIter> ParseExpression(TokenIter iter);

	private:
		void HandlePrint();
		void HandleInput();
		void HandleCls();
		void HandleLet();
		void HandleGoto();
		void HandleIf();
		void HandleElse();
		void HandleFor();
		void HandleNext();
		void HandleSleep();

	private:
		VarStorage m_Variables;
		TokenIter m_EndIter;

		int m_NextLine;
		int m_LineOffset = 0;
		TokenIter m_Token;

		std::deque<ForNode> m_ForStack;
		std::deque<ForNode> m_IfStack;

		bool m_SkipElse = true;

	};
}
