#pragma once

#include <deque>
#include <variant>
#include <map>

#include "Operator.hpp"
#include "Parser.hpp"
#include "Token.hpp"
#include "VarStorage.hpp"

namespace Basic
{
	inline Real Real_Sign(Real v)
	{
		return Real(int(0 < v) - int(v < 0));
	}

    Exception GenerateException(const std::vector<Basic::Token>& tokens, const std::string& input, const Basic::Exception_Iter& exception);

	struct ForNode
	{
		std::string varName;

		int line;
		int posInLine;

		Real startValue;
		Real endValue;
		Real step;
	};

	class Interpreter
	{
	public:
        enum Result
        {
            Result_Undefined = -3,
            Result_Terminate = -2,
            Result_NextLine = -1
        };

		Interpreter() = default;

	public:
        // Executes line and returns true if it was programm mode (i.e. with line number)
        bool RunLine(const std::vector<Token>& tokens, int lineNumber = -1);

		// Resets internal state so it is ready to run new line
		void Reset();

		// Checks if cursor is at end of current line
		inline bool IsEnd() const
		{
			return m_Cursor == m_End;
		}

	private:
		// Parses expression using tokens starting from iter and
		// returns last object and iterator to token after last-parsed one
        std::pair<Object, Token::Iter> ParseExpression(Token::Iter iter);

	private:
		template <class T>
        auto UnwrapValue(Token::Iter iter, const Object& obj, const std::string& error = "")
		{
			if (std::holds_alternative<Symbol>(obj))
			{
				const std::string& name = std::get<Symbol>(obj).value;

				// Let's check if a variable with the given name exists
				const auto variable = m_Variables.Get(name);

				if (variable)
				{
					// The variable exists...

					const auto value = variable.value().get();

					if (!std::holds_alternative<T>(value))
					{
						// ... but it doesn't have type that we want
                        throw Exception_Iter(iter, error);
					}

					// ... and it's of the right type so return it
					return std::get<T>(value).value;
				}

				// Couldn't find the variable so assume it was an invalid symbol
                throw Exception_Iter(iter, "Unexpected symbol: " + name);
			}

			if (!std::holds_alternative<T>(obj))
                throw Exception_Iter(iter, error);

			return std::get<T>(obj).value;
		}

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
        void HandleGoSub();
        void HandleReturn();
        void HandleList();
        void HandleNew();
        void HandleRun();
        void HandleLoad();

	private:
        std::map<int, std::vector<Basic::Token>> m_Programm;
		VarStorage m_Variables;

		int m_NextLine;
		int m_LineOffset = 0;
		
        Token::Iter m_Cursor;
		Token::Iter m_End;

		std::deque<ForNode> m_ForStack;
		std::deque<ForNode> m_IfStack;

        bool m_SkipElse = true;

        int m_ReturnLine = Result_Undefined;
        int m_ReturnPosInLine = Result_Undefined;

	};
}
