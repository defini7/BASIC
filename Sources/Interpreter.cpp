#include "Include/Interpreter.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

#define REAL_SIGN(v) (Real)((0.0l < v) - (v < 0.0l))

namespace Basic
{
	// Returns result of expression and position of next token after end of expression
    std::pair<Object, Token::Iter> Interpreter::ParseExpression(Token::Iter iter)
	{
		// Using Shunting yard algorithm

		std::deque<Token> holding, output;
		std::deque<Object> solving;

		Token prev(Token::Type::None);

		auto token = iter;
		for (; token != m_End; ++token)
		{
			switch (token->type)
			{
			case Token::Type::Literal_NumericBase10:
			case Token::Type::Literal_NumericBase16:
			case Token::Type::Literal_NumericBase2:
			case Token::Type::Literal_String:
			case Token::Type::Symbol:
				output.push_back(*token);
				break;

			// If we reach one of the keywords then just stop parsing
			case Token::Type::Keyword_Print:
			case Token::Type::Keyword_Input:
			case Token::Type::Keyword_Cls:
			case Token::Type::Keyword_Let:
			case Token::Type::Keyword_Rem:
			case Token::Type::Keyword_Goto:
			case Token::Type::Keyword_If:
			case Token::Type::Keyword_Then:
			case Token::Type::Keyword_Else:
			case Token::Type::Keyword_For:
			case Token::Type::Keyword_To:
			case Token::Type::Keyword_Step:
			case Token::Type::Keyword_Next:
			case Token::Type::Keyword_Sleep:
            case Token::Type::Keyword_GoSub:
            case Token::Type::Keyword_Return:
			case Token::Type::Semicolon:
			case Token::Type::Colon:
				goto out;

			case Token::Type::Keyword_Sin:
			case Token::Type::Keyword_Cos:
			case Token::Type::Keyword_Tan:
			case Token::Type::Keyword_ArcSin:
			case Token::Type::Keyword_ArcCos:
			case Token::Type::Keyword_ArcTan:
			case Token::Type::Keyword_Sqrt:
			case Token::Type::Keyword_Log:
			case Token::Type::Keyword_Exp:
			case Token::Type::Keyword_Abs:
			case Token::Type::Keyword_Sign:
			case Token::Type::Keyword_Int:
            case Token::Type::Keyword_Random:
            case Token::Type::Keyword_End:
            case Token::Type::Keyword_Val:
				holding.push_back(*token);
				break;

			case Token::Type::Operator:
			{
				Operator op = Parser::s_Operators[token->value];

				Token tok = *token;

				// Check for an unary operator
				if (tok.value == "+" || tok.value == "-")
				{
					constexpr std::array<Token::Type, 7> excluded =
					{
						Token::Type::Literal_NumericBase16,
						Token::Type::Literal_NumericBase10,
						Token::Type::Literal_NumericBase8,
						Token::Type::Literal_NumericBase2,
						Token::Type::Literal_String,
						Token::Type::Symbol,
						Token::Type::Parenthesis_Close
					};

					bool notExcluded = std::find(excluded.begin(), excluded.end(), prev.type) == excluded.end();

					if (notExcluded || prev.type == Token::Type::None)
					{
						// It is easier to handle unary operators as u+ or u-
						tok.value = "u" + tok.value;
					}
				}

				// Drain the stack out to the output stack until there's nothing to take or
				// the precedence of the current token is less than the precedence of the top-stack token
                while (!holding.empty())
                {
                    Token tok = holding.back();

                    // If top is a function token, it has highest precedence
                    if (tok.IsFunction())
                    {
                        output.push_back(tok);
                        holding.pop_back();
                        continue;
                    }

                    // For regular operators, check precedence
                    if (tok.type != Token::Type::Parenthesis_Open &&
                        op.precedence <= Parser::s_Operators[tok.value].precedence)
                    {
                        output.push_back(tok);
                        holding.pop_back();
                    }
                    else
                        break;
                }

				// ... only then append current token to the holding stack
				holding.push_back(tok);
			}
			break;

			case Token::Type::Parenthesis_Open:
				holding.push_back(*token);
			break;

			case Token::Type::Parenthesis_Close:
			{
				// Drain the holding stack out until an open parenthesis
				while (holding.back().type != Token::Type::Parenthesis_Open)
				{
					output.push_back(holding.back());
					holding.pop_back();
				}

				// And remove the parenthesis by itself
				holding.pop_back();

                // If the token before the open parenthesis was a function
                // then add it to the output now
                if (!holding.empty() && holding.back().IsFunction())
                {
                    output.push_back(holding.back());
                    holding.pop_back();
                }
			}
			break;

			}

			prev = *token;
		}

	out:

		// Drain out the holding stack at the end
		while (!holding.empty())
		{
			output.push_back(holding.back());
			holding.pop_back();
		}

		for (const auto& token : output)
		{
			switch (token.type)
			{
			case Token::Type::Literal_NumericBase10: solving.push_back(Numeric{ std::stold(token.value) }); break;
			case Token::Type::Literal_NumericBase16: solving.push_back(Numeric{ (Real)std::stoll(token.value, nullptr, 16) }); break;
			case Token::Type::Literal_NumericBase2:  solving.push_back(Numeric{ (Real)std::stoll(token.value, nullptr, 2) }); break;

			case Token::Type::Literal_String:
				solving.push_back(String{ token.value });
			break;

			case Token::Type::Symbol:
				solving.push_back(Symbol{ token.value });
			break;

			case Token::Type::Operator:
			{
				const auto& op = Parser::s_Operators[token.value];

				std::vector<Object> arguments(op.arguments);

				// Save all operator arguments if there is enough of them on the stack
				if (solving.size() < op.arguments)
                    throw Exception_Iter(iter, "Not enough arguments for the operator: " + token.value);

				for (auto& arg : arguments)
				{
					arg = solving.back();
					solving.pop_back();
				}

				Object object;

				switch (op.arguments)
				{
				case 1:
				{
					// Handle unary operators
                    Real number = UnwrapValue<Numeric>(iter, arguments[0], "Can't apply unary operator to non-numeric value");

					switch (op.type)
					{
					case Operator::Type::Subtraction: object = Numeric{ -number }; break;
					case Operator::Type::Addition:    object = Numeric{ +number }; break;
					}
				}
				break;

				case 2:
				{
					// Handle binary operators

					if (std::holds_alternative<String>(arguments[1]))
					{
						// You can concatenate a string with another string

                        std::string lhs = UnwrapValue<String>(iter, arguments[1], "");

						if (op.type != Operator::Type::Addition)
                            throw Exception_Iter(iter, "Can perform only concatenation (+) with strings: " + lhs);

                        std::string rhs = UnwrapValue<String>(iter, arguments[0], "Can only concatenate string with another string: " + lhs);

						object = String{ lhs + rhs };
					}
					else
					{
						#define DEFINE_CHECK_TYPES(op) \
							auto CheckTypes = [&]<class T>(const std::string& name) \
							{ \
								if (std::holds_alternative<T>(arguments[1]) || std::holds_alternative<Symbol>(arguments[1])) \
								{ \
                                    const auto lhs = UnwrapValue<T>(iter, arguments[1], ""); \
                                    const auto rhs = UnwrapValue<T>(iter, arguments[0], "Can only compare a " + name + " with another " + name); \
								\
									object = Numeric{ (Real)(lhs op rhs) }; \
									return true; \
								} \
								\
								return false; \
							}

						#define CASE_COMP(sign, op) \
							case Operator::Type::sign: \
							{ \
								DEFINE_CHECK_TYPES(op); \
							\
								if (CheckTypes.operator()<Numeric>("number")) {} \
								else if (CheckTypes.operator()<String>("string")) {} \
                                else \
									throw Exception_Iter(iter, "Can't compare 2 values"); \
							} \
							break;

						switch (op.type)
						{
							CASE_COMP(Equals, ==)
							CASE_COMP(NotEquals, !=)
							CASE_COMP(Less, <)
							CASE_COMP(Greater, >)
							CASE_COMP(LessEquals, <=)
							CASE_COMP(GreaterEquals, >=)

						case Operator::Type::Assign:
						{
							if (!std::holds_alternative<Symbol>(arguments[1]))
                                throw Exception_Iter(iter, "Can't create variable with invalid name");

							m_Variables.Set(
								std::get<Symbol>(arguments[1]).value,
								arguments[0]
							);

							object = arguments[0];
						}
						break;

						default:
						{
                            const auto lhs = UnwrapValue<Numeric>(iter, arguments[1], "You must have numeric values to perform arithmetic operations");
                            const auto rhs = UnwrapValue<Numeric>(iter, arguments[0], "You must have numeric values to perform arithmetic operations");

							switch (op.type)
							{
                            case Operator::Type::Subtraction:    object = Numeric{ lhs - rhs };     break;
                            case Operator::Type::Addition:       object = Numeric{ lhs + rhs };     break;
                            case Operator::Type::Multiplication: object = Numeric{ lhs * rhs };     break;
                            case Operator::Type::Division:       object = Numeric{ lhs / rhs };     break;
                            case Operator::Type::Power:          object = Numeric{ pow(lhs, rhs) }; break;
							}
						}

						};

						#undef DEFINE_CHECK_TYPES
						#undef CASE_COMP
					}
				}
				break;

				}

				solving.push_back(object);
			}
			break;

		#define CASE_FUNC(func, signature, token, bottom, top) \
			case Token::Type::Keyword_##token: \
			{ \
				if (solving.empty()) \
                    throw Exception_Iter(iter, "Not enough arguments: "#signature" <arg>"); \
			\
				if (!std::holds_alternative<Numeric>(solving.back())) \
                    throw Exception_Iter(iter, "Argument must be numeric: "#signature" <arg>"); \
			\
				Real value = std::get<Numeric>(solving.back()).value; \
			\
				if (value < bottom || value > top) \
                    throw Exception_Iter(iter, "Argument must be within the range: [" + std::to_string(bottom) + ", " + std::to_string(top) + "]"); \
			\
				solving.pop_back(); \
				solving.push_back(Numeric{ func(value) }); \
			} \
			break;
			
			CASE_FUNC(sin, SIN, Sin, Numeric::MIN, Numeric::MAX)
			CASE_FUNC(cos, COS, Cos, Numeric::MIN, Numeric::MAX)
			CASE_FUNC(tan, TAN, Tan, Numeric::MIN, Numeric::MAX)

			CASE_FUNC(asin, ARCSIN, ArcSin, -1.0, 1.0)
			CASE_FUNC(acos, ARCCOS, ArcCos, -1.0, 1.0)
			CASE_FUNC(atan, ARCTAN, ArcTan, -3.145926535 * 0.5, 3.145926535 * 0.5)

			CASE_FUNC(log10, LOG, Log, Numeric::EPS, Numeric::MAX)
			CASE_FUNC(exp, EXP, Exp, Numeric::MIN, Numeric::MAX)
			CASE_FUNC(fabs, ABS, Abs, Numeric::MIN, Numeric::MAX)
			CASE_FUNC(REAL_SIGN, SIGN, Sign, Numeric::MIN, Numeric::MAX)
			CASE_FUNC(trunc, INT, Int, Numeric::MIN, Numeric::MAX)

            case Token::Type::Keyword_Val:
            {
                if (solving.empty())
                    throw Exception_Iter(iter, "Not enough arguments: VAL <arg>");

                std::string value = UnwrapValue<String>(iter, solving.back(), "Argument must be string: VAL <arg>");

                solving.pop_back();
                solving.push_back(Numeric { std::stold(value) });
            }
            break;

			case Token::Type::Keyword_Random:
				solving.push_back(Numeric{ (Real)rand() / (Real)RAND_MAX });
			break;

			}

		}

		if (solving.empty())
		{
			// Nothing has been evaluated
			return std::make_pair(Object(), token);
		}

		Object obj = solving.back();

		// Possibly could be a variable name so let's extract a value from it
		if (std::holds_alternative<Symbol>(obj))
		{
			auto value = m_Variables.Get(std::get<Symbol>(obj).value);

			if (value)
				obj = *value;
		}

		return std::make_pair(obj, token);
	}

	int Interpreter::Execute(const std::vector<Token>& tokens, int line)
	{
        if (m_LineOffset >= (int)tokens.size())
		{
			m_LineOffset = 0;
            return Result_NextLine;
		}

        m_NextLine = Result_NextLine;
		m_End = tokens.end();

		m_Cursor = tokens.begin() + m_LineOffset;

		bool newStmt = true;

    #define NEW_STMT if (!newStmt) throw Exception_Iter(m_Cursor, "Expected : before new statement")

		while (m_Cursor != tokens.end())
		{
            try
            {
                switch (m_Cursor->type)
                {
                case Token::Type::Keyword_Print: NEW_STMT; HandlePrint(); newStmt = false; break;
                case Token::Type::Keyword_Input: NEW_STMT; HandleInput(); newStmt = false; break;
                case Token::Type::Keyword_Cls: NEW_STMT; HandleCls(); newStmt = false; break;
                case Token::Type::Keyword_Let: NEW_STMT; HandleLet(); newStmt = false; break;
                case Token::Type::Keyword_Rem: NEW_STMT; goto out1;
                case Token::Type::Keyword_Goto: NEW_STMT; HandleGoto(); goto out1;
                case Token::Type::Keyword_If: NEW_STMT; HandleIf(); newStmt = true; break;
                case Token::Type::Keyword_Else: HandleElse(); newStmt = false; break;

                case Token::Type::Keyword_For:
                {
                    NEW_STMT;
                    HandleFor();
                    newStmt = false;

                    ForNode& node = m_ForStack.back();

                    node.posInLine = std::distance(tokens.begin(), m_Cursor);
                    node.line = line;

                    m_LineOffset = node.posInLine + 1;
                }
                break;

                case Token::Type::Keyword_Next:
                {
                    NEW_STMT;
                    HandleNext();
                    newStmt = false;

                    if (m_NextLine != -1)
                    {
                        ForNode& node = m_ForStack.back();
                        m_LineOffset = node.posInLine + 1;

                        goto out1;
                    }
                }
                break;

                case Token::Type::Keyword_Sleep: NEW_STMT; HandleSleep(); newStmt = false; break;
                case Token::Type::Keyword_End: NEW_STMT; m_NextLine = Result_Terminate; goto out1;
                case Token::Type::Keyword_GoSub:
                {
                    NEW_STMT;
                    HandleGoSub();

                    m_ReturnLine = line;
                    m_ReturnPosInLine = std::distance(tokens.begin(), m_Cursor);

                    goto out1;
                }

                case Token::Type::Keyword_Return: NEW_STMT; HandleReturn(); goto out1;

                default:
                {
                    if (m_Cursor->type == Token::Type::Colon)
                    {
                        newStmt = true;
                        ++m_Cursor;
                    }
                    else
                        newStmt = false;

                    auto [_, end] = ParseExpression(m_Cursor);

                    if (m_Cursor == end)
                    {
                        m_NextLine = line;
                        m_LineOffset = std::distance(tokens.begin(), m_Cursor);
						
                        goto out1;
                    }
                    else
                        ++m_Cursor;
                }

                }
            }
            catch (const Exception_Iter& e)
            {
                throw e;
            }
		}

		m_LineOffset = 0;

		out1:

		return m_NextLine;
	}

	// PRINT <?expr>; <?expr>; ...
	void Interpreter::HandlePrint()
	{
		do
		{
			// PRINT / ;
			++m_Cursor;

            try
            {
                // <?expr>
                auto [res, end] = ParseExpression(m_Cursor);

                if (m_Cursor != end)
                {
                    std::visit([](const auto& obj)
                        {
                            std::cout << obj.value;
                        }, res);

                    m_Cursor = end;
                }
            }
            catch (const Exception_Iter& e)
            {
                throw e;
            }
		}
		while (m_Cursor != m_End && m_Cursor->type == Token::Type::Semicolon);

        if (std::prev(m_Cursor)->type != Token::Type::Semicolon)
            std::cout << std::endl;
	}

	// INPUT <?question>; <arg>
	void Interpreter::HandleInput()
	{
		// INPUT
		++m_Cursor;

        try
        {
            // <question> / <arg>
            auto [expr, end] = ParseExpression(m_Cursor);

            if (end != m_End && end->type == Token::Type::Semicolon)
            {
                // ;
                ++end;

                if (std::holds_alternative<String>(expr))
                    std::cout << std::get<String>(expr).value;

                // <arg>
                if (end->type != Token::Type::Symbol)
                    throw std::runtime_error("Expected variable name: INPUT <?question>; <arg>");
            }

            if (m_Cursor->type == Token::Type::Symbol)
            {
                std::string line;
                std::getline(std::cin >> std::ws, line);

                m_Variables.Set(m_Cursor->value, String{ line });
            }

            m_Cursor = end;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }
	}

	// CLS
	void Interpreter::HandleCls()
	{
	#ifdef _WIN32
		system("cls");
	#else
		system("clear");
	#endif

		++m_Cursor;
	}

	// LET <name> = <expr>
	void Interpreter::HandleLet()
	{
		// LET
		++m_Cursor;

		// <name>
		if (m_Cursor->type != Token::Type::Symbol)
            throw Exception_Iter(m_Cursor, "Expected variable name");

		std::string name = m_Cursor->value;
		++m_Cursor;

		// =
		if (m_Cursor->type != Token::Type::Operator)
            throw Exception_Iter(m_Cursor, "Expected =");

        try
        {
            // <expr>
            auto [res, end] = ParseExpression(m_Cursor + 1);

            m_Variables.Set(name, res);
            m_Cursor = end;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }
	}

	// GOTO <line>
	void Interpreter::HandleGoto()
	{
		// GOTO
		++m_Cursor;

        try
        {
            // <line>
            auto [res, end] = ParseExpression(m_Cursor);

            if (!std::holds_alternative<Numeric>(res))
                throw Exception_Iter(std::prev(end), "Expected line number to be numeric");

            m_NextLine = (int)std::get<Numeric>(res).value;
            m_Cursor = end;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }
	}

	// IF <expr> THEN <stmt> ELSE <stmt>
	void Interpreter::HandleIf()
	{
		// IF
		++m_Cursor;

        try
        {
            // <expr>
            auto [res, iter] = ParseExpression(m_Cursor);

            if (!std::holds_alternative<Numeric>(res))
                throw Exception_Iter(std::prev(iter), "Expected expression result to be numeric");

            // THEN
            if (iter->type != Token::Type::Keyword_Then)
                throw Exception_Iter(iter, "Expected THEN");

            ++iter;

            // if <expr>=0 then move to else block if it exists
            if (std::get<Numeric>(res).value == 0.0)
            {
                int elseBalancer = 0;

                // We need to find ELSE block and execute it
                m_SkipElse = false;

                // Searching for the corresponding ELSE block
                while (iter != m_End && iter->type != Token::Type::Colon)
                {
                    if (iter->type == Token::Type::Keyword_If)
                        ++elseBalancer;

                    if (iter->type == Token::Type::Keyword_Else)
                    {
                        if (elseBalancer == 0)
                        {
                            // Found corresponding ELSE block so just stop here
                            m_Cursor = iter + 1;
                            return;
                        }

                        --elseBalancer;
                    }

                    ++iter;
                }

                // No ELSE block
            }
            else // <expr> != 0
                m_SkipElse = true;

            m_Cursor = iter;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }
	}

	void Interpreter::HandleElse()
	{
		if (m_SkipElse)
		{
			// We need to skip the corresponding ELSE block so just move to the end of the line
			m_Cursor = m_End;
		}
		else
		{
			m_SkipElse = true;

			// ELSE
			++m_Cursor;
		}
	}

	// FOR <var>=<expr> TO <expr> ? STEP <expr>
	void Interpreter::HandleFor()
	{
		ForNode node;
		Object res;

		// FOR
		++m_Cursor;

		// <var>
		if (m_Cursor->type != Token::Type::Symbol)
            throw Exception_Iter(m_Cursor, "Expected variable name");

		node.varName = m_Cursor->value;
		++m_Cursor;

		// =
		if (m_Cursor->type != Token::Type::Operator)
            throw Exception_Iter(m_Cursor, "Expected =");

		++m_Cursor;

		try
		{
            // <expr>
			std::tie(res, m_Cursor) = ParseExpression(m_Cursor);

			if (!std::holds_alternative<Numeric>(res))
                throw Exception_Iter(m_Cursor, "Expected <expr> result to be numeric");

			node.startValue = std::get<Numeric>(res).value;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }

		// TO
		if (m_Cursor->type != Token::Type::Keyword_To)
            throw Exception_Iter(m_Cursor, "Expected TO after <expr>");

		++m_Cursor;

		try
		{
            // <expr>
			std::tie(res, m_Cursor) = ParseExpression(m_Cursor);

			if (!std::holds_alternative<Numeric>(res))
                throw Exception_Iter(m_Cursor, "Expected <expr> result to be numeric");

			node.endValue = std::get<Numeric>(res).value;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
		}

		// STEP
		if (m_Cursor != m_End && m_Cursor->type == Token::Type::Keyword_Step)
		{
			++m_Cursor;

			// <expr>
			try
			{
				std::tie(res, m_Cursor) = ParseExpression(m_Cursor);

				if (!std::holds_alternative<Numeric>(res))
                    throw std::runtime_error("Expected <expr> result to be numeric");

				node.step = std::get<Numeric>(res).value;
            }
            catch (const Exception_Iter& e)
            {
                throw e;
            }
		}
		else
			node.step = 1.0;

		m_Variables.Set(node.varName, Numeric{ node.startValue });
		m_ForStack.push_back(node);
	}

    // NEXT <var>
	void Interpreter::HandleNext()
	{        
		if (m_ForStack.empty())
            throw Exception_Iter(m_Cursor, "NEXT without FOR");

        // NEXT
        ++m_Cursor;

        // <var>
        if (m_Cursor != m_End && m_Cursor->type == Token::Type::Symbol)
        {
            // While FOR on top of stack doesn't have variable with specified name
            while (m_ForStack.back().varName != m_Cursor->value)
            {
                m_ForStack.pop_back();

                if (m_ForStack.empty())
                    throw Exception_Iter(m_Cursor, "Can't find FOR loop with specified var name");
            }

            ++m_Cursor;
        }

        ForNode& node = m_ForStack.back();

		node.startValue += node.step;
		m_Variables.Set(node.varName, Numeric{ node.startValue });

		if (node.startValue <= node.endValue)
			m_NextLine = node.line;
        else
            m_ForStack.pop_back();
	}

	// SLEEP <arg>
	void Interpreter::HandleSleep()
	{
		// SLEEP
		++m_Cursor;

        try
        {
            // <expr>
            auto [res, end] = ParseExpression(m_Cursor);

            if (std::holds_alternative<Numeric>(res))
            {
                int millis = std::get<Numeric>(res).value;

                if (millis < 0)
                    throw Exception_Iter(end, "Can't SLEEP < 0 milliseconds");

                std::this_thread::sleep_for(std::chrono::milliseconds(millis));
            }
            else
            {
                throw Exception_Iter(end, "Expected milliseconds to be numeric");
            }

            m_Cursor = end;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }
	}

    // GOSUB <line>
    void Interpreter::HandleGoSub()
    {
        // GOSUB
        ++m_Cursor;

        try
        {
            // <line>
            auto [res, end] = ParseExpression(m_Cursor);

            if (!std::holds_alternative<Numeric>(res))
                throw std::runtime_error("Provide a line number");

            int line = std::get<Numeric>(res).value;

            if (line < 0)
                throw std::runtime_error("Provide a line number > 0");

            m_Cursor = end;
            m_NextLine = line;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }
    }

    // RETURN
    void Interpreter::HandleReturn()
    {
        if (m_ReturnLine == Result_Undefined)
            throw Exception_Iter(m_Cursor, "RETURN without GOSUB");

        m_NextLine = m_ReturnLine;
        m_LineOffset = m_ReturnPosInLine;

        m_ReturnLine = Result_Undefined;
        m_ReturnPosInLine = Result_Undefined;
    }
}
