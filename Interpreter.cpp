#include "Interpreter.hpp"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#define LD_SIGN(v) (long double)((0.0l < v) - (v < 0.0l))

namespace def
{
	// Returns result of expression and position of next token after end of expression
	std::pair<Object, Interpreter::TokenIter> Interpreter::ParseExpression(TokenIter iter)
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
			case Token::Type::Literal_NumericBase16: solving.push_back(Numeric{ (long double)std::stoll(token.value, nullptr, 16) }); break;
			case Token::Type::Literal_NumericBase2:  solving.push_back(Numeric{ (long double)std::stoll(token.value, nullptr, 2) }); break;

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
					long double number = UnwrapValue(Numeric, arguments[0], "Can't apply unary operator to non-numeric value");

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

						if (op.type != Operator::Type::Semicolon)
							throw InterpreterException("Can perform only concatenation (;) with strings: " + lhs);

						const std::string rhs = UnwrapValue(String, arguments[0], "Can only concatenate string with another string: " + lhs);

						object = String{ lhs + rhs };
					}
					else
					{
						switch (op.type)
						{
						case Operator::Type::Equals:
						{
							auto CheckTypes = [&]<class T>(const std::string & name)
							{
								if (std::holds_alternative<T>(arguments[1]) || std::holds_alternative<Symbol>(arguments[1]))
								{
									const auto lhs = UnwrapValue(T, arguments[1], "");
									const auto rhs = UnwrapValue(T, arguments[0], "Can only compare a " + name + " with another " + name);

									object = Numeric{ (long double)(lhs == rhs) };
									return true;
								}

								return false;
							};

							if (CheckTypes.operator()<Numeric>("number"));
							else if (CheckTypes.operator()<String>("string"));
							else
								throw InterpreterException("Can't compare 2 values");
						}
						break;

						case Operator::Type::Assign:
						{
							if (!std::holds_alternative<Symbol>(arguments[1]))
								throw InterpreterException("Can't create a variable with an invalid name");

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
				long double value = std::get<Numeric>(solving.back()).value; \
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
			FUNC(LD_SIGN, SIGN, Sign, Numeric::MIN, Numeric::MAX)
			FUNC(trunc, INT, Int, Numeric::MIN, Numeric::MAX)

			case Token::Type::Keyword_Random:
				solving.push_back(Numeric{ (long double)rand() / (long double)RAND_MAX });
			break;

			}

		}

	#undef UnwrapValue

		Object obj = solving.back();

		if (std::holds_alternative<Symbol>(obj))
		{
			auto value = m_Variables.Get(std::get<Symbol>(obj).value);

			if (value)
				obj = *value;
		}

		return std::make_pair(obj, token);
	}

	int Interpreter::Execute(bool isTerminalMode, const std::vector<Token>& tokens)
	{
		m_NextLine = -1;
		bool newStmt = true;

		m_EndIter = tokens.end();

	#define NEW_STMT if (!newStmt) throw InterpreterException("Expected : before new statement");

		for (auto token = tokens.begin(); token != tokens.end();)
		{
			switch (token->type)
			{
			case Token::Type::Keyword_Print: NEW_STMT token = HandlePrint(token); newStmt = false; break;
			case Token::Type::Keyword_Input: NEW_STMT token = HandleInput(token); newStmt = false; break;
			case Token::Type::Keyword_Cls: NEW_STMT token = HandleCls(token); newStmt = false; break;
			case Token::Type::Keyword_Let: NEW_STMT token = HandleLet(token); newStmt = false; break;
			case Token::Type::Keyword_Rem: NEW_STMT goto out1;
			case Token::Type::Keyword_Goto: NEW_STMT token = HandleGoto(token); goto out1;
			case Token::Type::Keyword_If: NEW_STMT token = HandleIf(token); newStmt = false; break;
			case Token::Type::Keyword_Else: NEW_STMT token = HandleElse(token); newStmt = false; break;
			case Token::Type::Keyword_For: NEW_STMT token = HandleFor(token); newStmt = false; break;
			case Token::Type::Keyword_Next: NEW_STMT token = HandleNext(token); newStmt = false; break;
			case Token::Type::Keyword_Sleep: NEW_STMT token = HandleSleep(token); newStmt = false; break;

			default:
			{
				if (token->type == Token::Type::Colon)
					newStmt = true;
				else
					newStmt = false;

				/*TokenIter end;
				ParseExpression(token, end);*/
				++token;
			}

			}
		}

		out1:

		return m_NextLine;
	}

	// PRINT <expr>
	Interpreter::TokenIter Interpreter::HandlePrint(TokenIter iter)
	{
		// PRINT
		++iter;

		// <arg>
		auto [res, end] = ParseExpression(iter);

		std::visit([](const auto& obj)
			{
				std::cout << obj.value << std::endl;
			}, res);

		return end;
	}

	// INPUT <arg>
	Interpreter::TokenIter Interpreter::HandleInput(TokenIter iter)
	{
		// INPUT
		++iter;

		// <arg>
		auto [sym, end] = ParseExpression(iter);

		if (std::holds_alternative<Symbol>(sym))
		{
			std::string name = std::get<Symbol>(sym).value;

			std::string line;
			std::getline(std::cin, line);

			m_Variables.Set(name, String{ line });
		}

		return end;
	}

	// CLS
	Interpreter::TokenIter Interpreter::HandleCls(TokenIter iter)
	{
	#ifdef _WIN32
		system("cls");
	#else
		system("clear");
	#endif

		return iter + 1;
	}

	// LET <name> = <expr>
	Interpreter::TokenIter Interpreter::HandleLet(TokenIter iter)
	{
		// LET
		++iter;

		// <name>
		if (iter->type != Token::Type::Symbol)
			throw InterpreterException("Expected variable name: LET <name> = <expr>");

		std::string name = iter->value;
		++iter;

		// =
		if (iter->type != Token::Type::Operator)
			throw InterpreterException("Expected =: LET <name> = <expr>");

		// <expr>
		auto [res, end] = ParseExpression(iter + 1);

		m_Variables.Set(name, res);

		return end;
	}

	// GOTO <line>
	Interpreter::TokenIter Interpreter::HandleGoto(TokenIter iter)
	{
		// GOTO
		++iter;

		// <line>
		auto [res, end] = ParseExpression(iter);

		if (!std::holds_alternative<Numeric>(res))
			throw InterpreterException("Expected numeric value as a line number: GOTO <line>");

		m_NextLine = (int)std::get<Numeric>(res).value;
		return end;
	}

	Interpreter::TokenIter Interpreter::HandleIf(TokenIter iter)
	{
		return iter + 1;
	}

	Interpreter::TokenIter Interpreter::HandleElse(TokenIter iter)
	{
		return iter + 1;
	}

	Interpreter::TokenIter Interpreter::HandleFor(TokenIter iter)
	{
		return iter + 1;
	}

	Interpreter::TokenIter Interpreter::HandleNext(TokenIter iter)
	{
		return iter + 1;
	}

	// SLEEP <arg>
	Interpreter::TokenIter Interpreter::HandleSleep(TokenIter iter)
	{
		// SLEEP
		++iter;

		// <expr>
		auto [res, end] = ParseExpression(iter);

		if (std::holds_alternative<Numeric>(res))
		{
			int secs = std::get<Numeric>(res).value;

			if (secs < 0)
				throw InterpreterException("Can't SLEEP < 0 seconds");

			std::this_thread::sleep_for(std::chrono::seconds(secs));
		}

		return end;
	}
}
