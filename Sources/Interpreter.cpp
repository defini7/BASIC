#include "Include/Interpreter.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#define REAL_SIGN(v) (Real)((0.0l < v) - (v < 0.0l))

namespace def
{
	// Returns result of expression and position of next token after end of expression
	std::pair<Object, TokenIter> Interpreter::ParseExpression(TokenIter iter)
	{
		// Using Shunting yard algorithm

		std::deque<Token> holding, output;
		std::deque<Object> solving;

		Token prev(Token::Type::None);

		auto token = iter;
		for (; token != m_EndIter; ++token)
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
						tok.value = "u" + tok.value;
				}

				// Drain the stack out to the output stack until there's nothing to take or
				// the precedence of the current token is less than the precedence of the top-stack token
				while (!holding.empty() && holding.back().type != Token::Type::Parenthesis_Open && op.precedence <= Parser::s_Operators[holding.back().value].precedence)
				{
					output.push_back(holding.back());
					holding.pop_back();
				}

				// only then append current token to the holding stack
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

		auto UnwrapValue = [&]<class T>(const Object& obj, const std::string& error = "")
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
						throw InterpreterException(error);
					}

					// ... and it's of the right type so return it
					return std::get<T>(value).value;
				}

				// Couldn't find the variable so assume it was an invalid symbol
				throw InterpreterException("Unexpected symbol: " + name);
			}

			if (!std::holds_alternative<T>(obj))
				throw InterpreterException(error);

			return std::get<T>(obj).value;
		};

	#define UnwrapValue(type, obj, error) UnwrapValue.operator()<type>(obj, error)

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

				// Save all operator arguments if there are enough on the stack
				if (solving.size() < op.arguments)
					throw InterpreterException("Not enough arguments for the operator: " + token.value);

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
					Real number = UnwrapValue(Numeric, arguments[0], "Can't apply unary operator to non-numeric value");

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

						const std::string lhs = UnwrapValue(String, arguments[1], "");

						if (op.type != Operator::Type::Addition)
							throw InterpreterException("Can perform only concatenation (+) with strings: " + lhs);

						const std::string rhs = UnwrapValue(String, arguments[0], "Can only concatenate string with another string: " + lhs);

						object = String{ lhs + rhs };
					}
					else
					{
						#define DEFINE_CHECK_TYPES(op) \
							auto CheckTypes = [&]<class T>(const std::string& name) \
							{ \
								if (std::holds_alternative<T>(arguments[1]) || std::holds_alternative<Symbol>(arguments[1])) \
								{ \
									const auto lhs = UnwrapValue(T, arguments[1], ""); \
									const auto rhs = UnwrapValue(T, arguments[0], "Can only compare a " + name + " with another " + name); \
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
								else throw InterpreterException("Can't compare 2 values"); \
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
                                throw InterpreterException("Can't create variable with invalid name");

							m_Variables.Set(
								std::get<Symbol>(arguments[1]).value,
								arguments[0]
							);

							object = arguments[0];
						}
						break;

						default:
						{
							const auto lhs = UnwrapValue(Numeric, arguments[1], "You must have numeric values to perform arithmetic operations");
							const auto rhs = UnwrapValue(Numeric, arguments[0], "You must have numeric values to perform arithmetic operations");

							switch (op.type)
							{
							case Operator::Type::Subtraction:    object = Numeric{ lhs - rhs };  break;
							case Operator::Type::Addition:       object = Numeric{ lhs + rhs };  break;
							case Operator::Type::Multiplication: object = Numeric{ lhs * rhs };  break;
							case Operator::Type::Division:       object = Numeric{ lhs / rhs };  break;
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

		#define FUNC(func, signature, token, bottom, top) \
			case Token::Type::Keyword_##token: \
			{ \
				if (solving.empty()) \
					throw InterpreterException("Not enough arguments: "#signature" <arg>"); \
			\
				if (!std::holds_alternative<Numeric>(solving.back())) \
					throw InterpreterException("Argument must be numeric: "#signature" <arg>"); \
			\
				Real value = std::get<Numeric>(solving.back()).value; \
			\
				if (value < bottom || value > top) \
					throw InterpreterException("Argument must be within the range: [" + std::to_string(bottom) + ", " + std::to_string(top) + "]"); \
			\
				solving.pop_back(); \
				solving.push_back(Numeric{ func(value) }); \
			} \
			break;
			
			FUNC(sin, SIN, Sin, Numeric::MIN, Numeric::MAX)
			FUNC(cos, COS, Cos, Numeric::MIN, Numeric::MAX)
			FUNC(tan, TAN, Tan, Numeric::MIN, Numeric::MAX)

			FUNC(asin, ARCSIN, ArcSin, -1.0, 1.0)
			FUNC(acos, ARCCOS, ArcCos, -1.0, 1.0)
			FUNC(atan, ARCTAN, ArcTan, -3.145926535 * 0.5, 3.145926535 * 0.5)

			FUNC(log10, LOG, Log, Numeric::EPS, Numeric::MAX)
			FUNC(exp, EXP, Exp, Numeric::MIN, Numeric::MAX)
			FUNC(fabs, ABS, Abs, Numeric::MIN, Numeric::MAX)
			FUNC(REAL_SIGN, SIGN, Sign, Numeric::MIN, Numeric::MAX)
			FUNC(trunc, INT, Int, Numeric::MIN, Numeric::MAX)

			case Token::Type::Keyword_Random:
				solving.push_back(Numeric{ (Real)rand() / (Real)RAND_MAX });
			break;

			}

		}

	#undef UnwrapValue

		if (solving.empty())
			return std::make_pair(Object(), token);

		Object obj = solving.back();

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
		if (m_LineOffset >= tokens.size())
		{
			m_LineOffset = 0;
			return -1;
		}

		m_NextLine = -1;
		m_EndIter = tokens.end();

        bool newStmt = true;

    #define NEW_STMT if (!newStmt) throw InterpreterException("Expected : before new statement")

		m_Token = tokens.begin() + m_LineOffset;

		while (m_Token != tokens.end())
		{
			switch (m_Token->type)
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

				node.posInLine = std::distance(tokens.begin(), m_Token);
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

			default:
			{
				if (m_Token->type == Token::Type::Colon)
                {
                    newStmt = true;
					++m_Token;
                }
                else
                    newStmt = false;

				auto [_, end] = ParseExpression(m_Token);

				if (m_Token == end)
				{
					m_NextLine = line;
					m_LineOffset = std::distance(tokens.begin(), m_Token);
					goto out1;
				}

				m_Token = end;
			}

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
			++m_Token;

			// <?expr>
			auto [res, end] = ParseExpression(m_Token);

			if (m_Token == end)
				std::cout << std::endl;
			else
			{
				m_Token = end;

				std::visit([](const auto& obj)
					{
						std::cout << obj.value;
					}, res);
			}
		}
		while (m_Token != m_EndIter && m_Token->type == Token::Type::Semicolon);
	}

	// INPUT <?question>; <arg>
	void Interpreter::HandleInput()
	{
		// INPUT
		++m_Token;

		// <question> / <arg>
		auto [expr, end] = ParseExpression(m_Token);

		if (end != m_EndIter && end->type == Token::Type::Semicolon)
		{
			// ;
			++end;

			if (std::holds_alternative<String>(expr))
				std::cout << std::get<String>(expr).value;

			// <arg>
			std::tie(expr, end) = ParseExpression(end);
		}

		m_Token = end;

		if (std::holds_alternative<Symbol>(expr))
		{
			std::string name = std::get<Symbol>(expr).value;

			std::string line;
			std::getline(std::cin, line);

			m_Variables.Set(name, String{ line });
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

		++m_Token;
	}

	// LET <name> = <expr>
	void Interpreter::HandleLet()
	{
		// LET
		++m_Token;

		// <name>
		if (m_Token->type != Token::Type::Symbol)
			throw InterpreterException("Expected variable name: LET <name> = <expr>");

		std::string name = m_Token->value;
		++m_Token;

		// =
		if (m_Token->type != Token::Type::Operator)
			throw InterpreterException("Expected =: LET <name> = <expr>");

		// <expr>
		auto [res, end] = ParseExpression(m_Token + 1);

		m_Variables.Set(name, res);

		m_Token = end;
	}

	// GOTO <line>
	void Interpreter::HandleGoto()
	{
		// GOTO
		++m_Token;

		// <line>
		auto [res, end] = ParseExpression(m_Token);

		if (!std::holds_alternative<Numeric>(res))
			throw InterpreterException("Expected line number to be numeric: GOTO <line>");

		m_NextLine = (int)std::get<Numeric>(res).value;
		m_Token = end;
	}

	// IF <expr> THEN <stmt> ELSE <stmt>
	void Interpreter::HandleIf()
	{
		// IF
		++m_Token;

		// <expr>
		auto [res, iter] = ParseExpression(m_Token);

		if (!std::holds_alternative<Numeric>(res))
			throw InterpreterException("Expected expression result to be numeric: IF <expr> ...");

		// THEN
		if (iter->type != Token::Type::Keyword_Then)
			throw InterpreterException("Expected THEN after IF: IF <expr> THEN ...");

		++iter;

		// if <expr>=0 then move to else block if it exists
		if (std::get<Numeric>(res).value == 0.0)
		{
			int elseBalancer = 0;

			// We need to find ELSE block and execute it
			m_SkipElse = false;

			// Searching for the corresponding ELSE block
			while (iter != m_EndIter)
			{
				if (iter->type == Token::Type::Keyword_If)
					++elseBalancer;

				if (iter->type == Token::Type::Keyword_Else)
				{	
					if (elseBalancer == 0)
					{
						// Found corresponding ELSE block so just stop here
						m_Token = iter + 1;
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

		m_Token = iter;
	}

	void Interpreter::HandleElse()
	{
		if (m_SkipElse)
		{
			// We need to skip the corresponding ELSE block so just move to the end of the line
			m_Token = m_EndIter;
		}
		else
		{
			m_SkipElse = true;

			// ELSE
			++m_Token;
		}
	}

	// FOR <var>=<expr> TO <expr> ? STEP <expr>
	void Interpreter::HandleFor()
	{
		ForNode node;

		Object res;

		// FOR
		++m_Token;

		// <var>
		if (m_Token->type != Token::Type::Symbol)
			throw InterpreterException("Expected variable name: FOR <var>=...");

		node.varName = m_Token->value;
		++m_Token;

		// =
		if (m_Token->type != Token::Type::Operator)
			throw InterpreterException("Expected =: FOR <var>=...");

		++m_Token;

		// <expr>
		try
		{
			std::tie(res, m_Token) = ParseExpression(m_Token);

			if (!std::holds_alternative<Numeric>(res))
				throw InterpreterException("Expected <expr> result to be numeric: FOR <var>=<expr> ...");

			node.startValue = std::get<Numeric>(res).value;
		}
		catch (InterpreterException& _)
		{
			std::cerr << "[Interpreter exception] Expected expression after =: FOR <var>=<expr> ..." << std::endl;
		}

		// TO
		if (m_Token->type != Token::Type::Keyword_To)
			throw InterpreterException("Expected TO after <expr>: FOR <var>=<expr> TO ...");

		++m_Token;

		// <expr>
		try
		{
			std::tie(res, m_Token) = ParseExpression(m_Token);

			if (!std::holds_alternative<Numeric>(res))
				throw InterpreterException("Expected <expr> result to be numeric: FOR <var>=<expr> TO <expr> ...");

			node.endValue = std::get<Numeric>(res).value;
		}
		catch (InterpreterException& _)
		{
			std::cerr << "[Interpreter exception] Expected expression after TO: FOR <var>=<expr> TO <expr> ..." << std::endl;
		}

		// STEP
		if (m_Token != m_EndIter && m_Token->type == Token::Type::Keyword_Step)
		{
			++m_Token;

			// <expr>
			try
			{
				std::tie(res, m_Token) = ParseExpression(m_Token);

				if (!std::holds_alternative<Numeric>(res))
					throw InterpreterException("Expected <expr> result to be numeric: FOR <var>=<expr> TO <expr> STEP <expr> ...");

				node.step = std::get<Numeric>(res).value;
			}
			catch (InterpreterException& _)
			{
				std::cerr << "[Interpreter exception] Expected expression after STEP: FOR <var>=<expr> TO <expr> STEP <expr>" << std::endl;
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
		// NEXT
		++m_Token;

		if (m_ForStack.empty())
			throw InterpreterException("NEXT without FOR");

        ForNode& node = m_ForStack.back();

        // <var>
        if (m_Token != m_EndIter && m_Token->type == Token::Type::Symbol)
        {
            // TODO: Add ability to jump to FOR loop with specified var name

            if (m_Token->value != node.varName)
                throw InterpreterException("<var> in NEXT <var> statement isn't same as var name of inner FOR");

            ++m_Token;
        }

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
		++m_Token;

		// <expr>
		auto [res, end] = ParseExpression(m_Token);

		if (std::holds_alternative<Numeric>(res))
		{
			int secs = std::get<Numeric>(res).value;

			if (secs < 0)
                throw InterpreterException("Can't SLEEP < 0 milliseconds");

            std::this_thread::sleep_for(std::chrono::milliseconds(secs));
		}
	}
}
