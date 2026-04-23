#include "../Include/Interpreter.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>

namespace Basic
{
    bool operator||(const std::string& s1, const std::string& s2)
    {
        return !s1.empty() || !s2.empty();
    }

    bool operator&&(const std::string& s1, const std::string& s2)
    {
        return !s1.empty() && !s2.empty();
    }

	// Returns result of expression and position of next token after end of expression
    std::pair<Object, Token::Iter> Interpreter::ParseExpression(Token::Iter iter)
	{
		// Using Shunting yard algorithm

		std::deque<Token> holding, output;
		std::deque<Object> solving;

		Token prev(Token::Type::None);

		auto token = iter;
		bool stop = false;

		for (; token != m_End; ++token)
		{
			switch (token->type)
			{
			case Token::Type::Literal_NumericBase10:
			case Token::Type::Literal_NumericBase16:
			case Token::Type::Literal_NumericBase2:
			case Token::Type::Literal_String:
                holding.push_back(*token);
                break;

            case Token::Type::Symbol:
            {
                std::string name = token->value;

                auto next = std::next(token);

                // Check if it's an array access
                if (next != m_End && next->type == Token::Type::Parenthesis_Open)
                {
                    Token::Iter it = next;

                    int index = ParseArrayIndex(it);

                    const auto value = m_Variables.Get(name);

                    if (!value || !std::holds_alternative<Array>(value.value().get()))
                        throw Exception_Iter(iter, "Variable is not an array");

                    Array& arr = std::get<Array>(value.value().get());

                    if (index < 0 || index >= (int)arr.value.size())
                        throw Exception_Iter(iter, "Array index out of bounds");

                    solving.push_back(Object(arr.value[index]));

                    token = it - 1;
                }
                else
                    solving.push_back(Object(Symbol{ name }));
            }
			break;

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
            case Token::Type::Keyword_List:
            case Token::Type::Keyword_Run:
            case Token::Type::Keyword_New:
            case Token::Type::Keyword_Load:
		    case Token::Type::Keyword_Dim:
		    case Token::Type::Semicolon:
		    case Token::Type::Colon:
		    case Token::Type::Comma:
            case Token::Type::Bracket_Close:
				stop = true;
				break;

			case Token::Type::Keyword_Sin:
			case Token::Type::Keyword_Cos:
			case Token::Type::Keyword_Tan:
			case Token::Type::Keyword_ArcSin:
			case Token::Type::Keyword_ArcCos:
			case Token::Type::Keyword_ArcTan:
			case Token::Type::Keyword_Sqrt:
			case Token::Type::Keyword_Log:
            case Token::Type::Keyword_Ln:
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
				// Check if there's a matching open paren in the holding stack
				bool hasMatchingOpenParen = false;

				for (auto it = holding.rbegin(); it != holding.rend(); ++it)
				{
					if (it->type == Token::Type::Parenthesis_Open)
					{
						hasMatchingOpenParen = true;
						break;
					}
				}

				if (!hasMatchingOpenParen)
				{
					// No matching open paren, this closing paren is for array indexing
					stop = true;
				}
				else
				{
					// Drain the holding stack out until an open parenthesis
					while (!holding.empty() && holding.back().type != Token::Type::Parenthesis_Open)
					{
						output.push_back(holding.back());
						holding.pop_back();
					}

					// And remove the parenthesis by itself
					if (!holding.empty())
					{
						holding.pop_back();

						// If the token before the open parenthesis was a function
						// then add it to the output now
						if (!holding.empty() && holding.back().IsFunction())
						{
							output.push_back(holding.back());
							holding.pop_back();
						}
					}
				}
			}
			break;

			// Bracket_Open and Bracket_Close are NOT handled here!
			// They are handled by HandleDim, HandleLet, etc.

			default: /* Unreachable */ break;

			}

			if (stop)
				break;

			prev = *token;
		}

		// Drain out the holding stack at the end
		while (!holding.empty())
		{
			output.push_back(holding.back());
			holding.pop_back();
		}

        auto ApplyFunc = [&](std::function<long double(long double)> func, const std::string& signature, Real bottom, Real top)
            {
                if (solving.empty())
                    throw Exception_Iter(iter, "Not enough arguments: " + signature + " <arg>");

                if (!std::holds_alternative<Numeric>(solving.back()))
                    throw Exception_Iter(iter, "Argument must be numeric: " + signature + " <arg>");

                Real value = std::get<Numeric>(solving.back()).value;

                if (value < bottom || value > top)
                    throw Exception_Iter(iter, "Argument must be within the range: [" + std::to_string(bottom) + ", " + std::to_string(top) + "]");

                solving.pop_back();
                solving.push_back(Numeric{ func(value) });
            };

		for (const auto& token : output)
		{
			switch (token.type)
			{
            case Token::Type::Literal_NumericBase10: solving.push_back(Object(Numeric{ std::stold(token.value) })); break;
			case Token::Type::Literal_NumericBase16: solving.push_back(Object(Numeric{ (Real)std::stoll(token.value, nullptr, 16) })); break;
			case Token::Type::Literal_NumericBase2:  solving.push_back(Object(Numeric{ (Real)std::stoll(token.value, nullptr, 2) })); break;

			case Token::Type::Literal_String:
				solving.push_back(Object(String{ token.value }));
			break;

			case Token::Type::Symbol:
				solving.push_back(Object(Symbol{ token.value }));
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
                    default: /* Unreachable */ break;
                    }
				}
				break;

				case 2:
				{
					// Handle binary operators

                    switch (op.type)
                    {
                    case Operator::Type::Equals:
                    case Operator::Type::NotEquals:
                    case Operator::Type::Less:
                    case Operator::Type::Greater:
                    case Operator::Type::LessEquals:
                    case Operator::Type::GreaterEquals:
                    {
                        auto Compare = [&](auto comparator)
                        {
                            auto CheckTypes = [&]<class T>(const std::string& name)
                            {
                                if (std::holds_alternative<T>(arguments[1]) || std::holds_alternative<Symbol>(arguments[1]))
                                {
                                    std::string error = "Expected " + name;

                                    const auto lhs = UnwrapValue<T>(iter, arguments[1], error);
                                    const auto rhs = UnwrapValue<T>(iter, arguments[0], error);

                                    object = Numeric{ (Real)comparator(lhs, rhs) };
                                    return true;
                                }

                                return false;
                            };

                            if (CheckTypes.template operator()<String>("string") || CheckTypes.template operator()<Numeric>("number"))
                                return;

                            throw Exception_Iter(iter, "Can't compare 2 values");
                        };

                        if (op.type == Operator::Type::Equals)
                            Compare(std::equal_to<>());
                        else if (op.type == Operator::Type::NotEquals)
                            Compare(std::not_equal_to<>());
                        else if (op.type == Operator::Type::Less)
                            Compare(std::less<>());
                        else if (op.type == Operator::Type::Greater)
                            Compare(std::greater<>());
                        else if (op.type == Operator::Type::LessEquals)
                            Compare(std::less_equal<>());
                        else if (op.type == Operator::Type::GreaterEquals)
                            Compare(std::greater_equal<>());
                    }
                    break;

                    case Operator::Type::And:
                    {
                        auto lhs = UnwrapValue<Numeric>(iter, arguments[1], "Expected number");
                        auto rhs = UnwrapValue<Numeric>(iter, arguments[0], "Expected number");
                        object = Numeric{ (Real)((lhs != 0) && (rhs != 0)) };
                    }
                    break;

                    case Operator::Type::Or:
                    {
                        auto lhs = UnwrapValue<Numeric>(iter, arguments[1], "Expected number");
                        auto rhs = UnwrapValue<Numeric>(iter, arguments[0], "Expected number");
                        object = Numeric{ (Real)((lhs != 0) || (rhs != 0)) };
                    }
                    break;

                    case Operator::Type::Assign:
                    {
                        if (!std::holds_alternative<Symbol>(arguments[1]))
                            throw Exception_Iter(iter, "Can't create variable with invalid name");

                        Object& value = arguments[0];

                        if (std::holds_alternative<Symbol>(arguments[0]))
                        {
                            auto var = m_Variables.Get(std::get<Symbol>(arguments[0]).value);

                            if (var)
                                value = *var;
                        }

                        m_Variables.Set(
                            std::get<Symbol>(arguments[1]).value,
                            value
                        );

                        object = value;
                    }
                    break;

                    default:
                    {
                        const auto lhs = UnwrapValue(iter, arguments[1]);
                        const auto rhs = UnwrapValue(iter, arguments[0]);

                        if (std::holds_alternative<Numeric>(lhs) && std::holds_alternative<Numeric>(rhs))
                        {
                            Real lhsVal = std::get<Numeric>(lhs).value;
                            Real rhsVal = std::get<Numeric>(rhs).value;

                            switch (op.type)
                            {
                            case Operator::Type::Subtraction:    object = Numeric{ lhsVal - rhsVal };     break;
                            case Operator::Type::Addition:       object = Numeric{ lhsVal + rhsVal };     break;
                            case Operator::Type::Multiplication: object = Numeric{ lhsVal * rhsVal };     break;
                            case Operator::Type::Division:       object = Numeric{ lhsVal / rhsVal };     break;
                            case Operator::Type::Power:          object = Numeric{ pow(lhsVal, rhsVal) }; break;
                            default: /* Unreachable */ break;
                            }
                        }
                        else if (std::holds_alternative<String>(lhs) && std::holds_alternative<String>(rhs))
                        {
                            std::string lhsVal = std::get<String>(lhs).value;
                            std::string rhsVal = std::get<String>(rhs).value;

                            if (op.type == Operator::Type::Addition) object = String{ lhsVal + rhsVal };
                            else throw Exception_Iter(iter+1, "Can only concatenate strings");
                        }
                        else
                            throw Exception_Iter(iter+1, "Can't perform binary operations on values with different types");
                    }

                    };

				}
				}

                solving.push_back(object);
			}
			break;

            case Token::Type::Keyword_Sin:     ApplyFunc(static_cast<Real(*)(Real)>(&sin), "SIN", Numeric::MIN, Numeric::MAX); break;
            case Token::Type::Keyword_Cos:     ApplyFunc(static_cast<Real(*)(Real)>(&cos), "COS", Numeric::MIN, Numeric::MAX); break;
            case Token::Type::Keyword_Tan:     ApplyFunc(static_cast<Real(*)(Real)>(&tan), "TAN", Numeric::MIN, Numeric::MAX); break;

            case Token::Type::Keyword_ArcSin:  ApplyFunc(static_cast<Real(*)(Real)>(&asin), "ARCSIN", -1.0, 1.0); break;
            case Token::Type::Keyword_ArcCos:  ApplyFunc(static_cast<Real(*)(Real)>(&acos), "ARCCOS", -1.0, 1.0); break;
            case Token::Type::Keyword_ArcTan:  ApplyFunc(static_cast<Real(*)(Real)>(&atan), "ARCTAN", -3.145926535 * 0.5, 3.145926535 * 0.5); break;

            case Token::Type::Keyword_Log:     ApplyFunc(static_cast<Real(*)(Real)>(&log10), "LOG", Numeric::EPS, Numeric::MAX); break;
            case Token::Type::Keyword_Ln:      ApplyFunc(static_cast<Real(*)(Real)>(&log), "LN", Numeric::EPS, Numeric::MAX); break;
            case Token::Type::Keyword_Exp:     ApplyFunc(static_cast<Real(*)(Real)>(&exp), "EXP", Numeric::MIN, Numeric::MAX); break;
            case Token::Type::Keyword_Abs:     ApplyFunc(static_cast<Real(*)(Real)>(&fabs), "ABS", Numeric::MIN, Numeric::MAX); break;
            case Token::Type::Keyword_Sign:    ApplyFunc(Real_Sign, "SIGN", Numeric::MIN, Numeric::MAX); break;
            case Token::Type::Keyword_Int:     ApplyFunc(static_cast<Real(*)(Real)>(&trunc), "INT", Numeric::MIN, Numeric::MAX); break;

            case Token::Type::Keyword_Val:
            {
                if (solving.empty())
                    throw Exception_Iter(iter + 1, "Not enough arguments: VAL <arg>");

                std::string value = UnwrapValue<String>(iter + 1, solving.back(), "Argument must be string: VAL <arg>");

                solving.pop_back();
                solving.push_back(Numeric { std::stold(value) });
            }
            break;

			case Token::Type::Keyword_Random:
				solving.push_back(Numeric{ (Real)rand() / (Real)RAND_MAX });
			break;
                    
            default: /* Unreachable */ break;

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
            std::string name = std::get<Symbol>(obj).value;

			auto value = m_Variables.Get(name);

            if (value)
                obj = *value;
            else
                throw Exception_Iter(iter, "No such variable \"" + name + "\"");
		}

		return std::make_pair(obj, token);
	}

    void Interpreter::Reset()
    {
        m_NextLine = -1;
        m_LineOffset = 0;

        m_Cursor = Token::Iter();
        m_End = Token::Iter();

        m_ForStack.clear();
        m_SkipElse = true;
    }

    Exception GenerateException(const std::vector<Basic::Token>& tokens, const std::string& input, const Basic::Exception_Iter& exception)
    {
        int pos = 0;

        for (auto it = tokens.begin(); it != exception.iterator; ++it)
            pos += it->value.length() + 1;

        return Basic::Exception(input, pos + 1, exception.message);
    }

    bool Interpreter::RunLine(const std::vector<Token>& tokens, int lineNumber)
    {
        if (tokens.empty())
            return false;

        bool programmMode = lineNumber > 0;

        // If the first token in a string is decimal then it must be a line number
        // so we don't want to execute our programm, we just want to save the line
        if (!programmMode && m_LineOffset == 0 && tokens[0].type == Token::Type::Literal_NumericBase10)
        {
            int line = std::stoi(tokens[0].value);

            if (line < 0)
                throw Exception_Iter(tokens.begin() + 1, "Invalid line number");

            m_Programm[line] = std::vector<Token>(tokens.begin() + 1, tokens.end());

            return true;
        }

        if (m_LineOffset >= (int)tokens.size())
		{
			m_LineOffset = 0;
            m_NextLine = Result_NextLine;

            return false;
		}

        m_NextLine = Result_NextLine;
		m_End = tokens.end();

		m_Cursor = tokens.begin() + m_LineOffset;
        m_LineOffset = 0;

		bool newStmt = true;

        auto EnsureNewStatement = [&]()
		{
			if (!newStmt)
				throw Exception_Iter(m_Cursor, "Expected : before new statement");
		};

		while (m_Cursor != tokens.end())
		{
            try
            {
                switch (m_Cursor->type)
                {
                case Token::Type::Keyword_Print: EnsureNewStatement(); HandlePrint(); newStmt = false; break;
                case Token::Type::Keyword_Input: EnsureNewStatement(); HandleInput(); newStmt = false; break;
                case Token::Type::Keyword_Cls: EnsureNewStatement(); HandleCls(); newStmt = false; break;
                case Token::Type::Keyword_Let: EnsureNewStatement(); HandleLet(); newStmt = false; break;
                case Token::Type::Keyword_Dim: EnsureNewStatement(); HandleDim(); newStmt = false; break;
                case Token::Type::Keyword_Rem: EnsureNewStatement(); m_NextLine = Result_NextLine; return programmMode;
                case Token::Type::Keyword_Goto: EnsureNewStatement(); HandleGoto(); return programmMode;
                case Token::Type::Keyword_If: EnsureNewStatement(); HandleIf(); newStmt = true; break;
                case Token::Type::Keyword_Else: HandleElse(); newStmt = false; break;

                case Token::Type::Keyword_For:
                {
                    EnsureNewStatement();
                    HandleFor();
                    newStmt = false;

                    ForNode& node = m_ForStack.back();

                    node.posInLine = (int)std::distance(tokens.begin(), m_Cursor);
                    node.line = lineNumber;
                }
                break;

                case Token::Type::Keyword_Next:
                {
                    EnsureNewStatement();
                    HandleNext();
                    newStmt = false;

                    if (m_NextLine != -1)
                    {
                        ForNode& node = m_ForStack.back();
                        m_LineOffset = node.posInLine + 1;

                        return programmMode;
                    }
                }
                break;

                case Token::Type::Keyword_Sleep: EnsureNewStatement(); HandleSleep(); newStmt = false; break;
                case Token::Type::Keyword_End: EnsureNewStatement(); m_NextLine = Result_Terminate; return programmMode;
                case Token::Type::Keyword_GoSub:
                {
                    EnsureNewStatement();
                    HandleGoSub();

                    m_SubStack.push_back(
                        SubNode{
                            .line = lineNumber,
                            .posInLine = (int)std::distance(tokens.begin(), m_Cursor)
                        });

                    return programmMode;
                }

                case Token::Type::Keyword_Return: EnsureNewStatement(); HandleReturn(); return programmMode;
                case Token::Type::Keyword_List: EnsureNewStatement(); HandleList(); return programmMode;
                case Token::Type::Keyword_New: EnsureNewStatement(); HandleNew(); return programmMode;
                case Token::Type::Keyword_Load: EnsureNewStatement(); HandleLoad(); return programmMode;

                case Token::Type::Keyword_Run:
                {
                    EnsureNewStatement();
                    HandleRun();

                    m_NextLine = Result_Terminate;

                    return programmMode;
                }
                break;

                default:
                {
                    const auto next = std::next(m_Cursor);

                    if (m_Cursor->type == Token::Type::Symbol && next != tokens.end() && next->type == Token::Type::Parenthesis_Open)
                    {
                        std::string name = m_Cursor->value;
                        ++m_Cursor;

                        int index = ParseArrayIndex(m_Cursor);

                        if (m_Cursor->type != Token::Type::Operator || m_Cursor->value != "=")
                            throw Exception_Iter(m_Cursor, "Expected = after array index");

                        try
                        {
                            auto [res, end] = ParseExpression(m_Cursor + 1);
                            auto value = m_Variables.Get(name);

                            if (!value || !std::holds_alternative<Array>(value.value().get()))
                                throw Exception_Iter(m_Cursor, "Variable is not an array");

                            Array& arr = std::get<Array>(value.value().get());

                            if (index < 0 || index >= (int)arr.value.size())
                                throw Exception_Iter(m_Cursor, "Array index out of bounds");

                            if (!std::holds_alternative<Numeric>(res))
                                throw Exception_Iter(m_Cursor, "Can only assign numeric values to array elements");

                            arr.value[index] = std::get<Numeric>(res);

                            if (m_Cursor == end)
                                ++m_Cursor;
                            else
                                m_Cursor = end;

                        }
                        catch (const Exception_Iter& e)
                        {
                            throw e;
                        }
                    }
                    else if (m_Cursor->type == Token::Type::Colon)
                    {
                        newStmt = true;
                        ++m_Cursor;
                    }
                    else
                        newStmt = false;

                    auto [_, end] = ParseExpression(m_Cursor);

                    if (m_Cursor == end)
                    {
                        m_NextLine = lineNumber;
                        m_LineOffset = (int)std::distance(tokens.begin(), m_Cursor);

                        return programmMode;
                    }
                    else
                        m_Cursor = end;
                }
                }
            }
            catch (const Exception_Iter& e)
            {
                throw e;
            }
		}

        return programmMode;
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
                    std::visit(
                        std::overloaded
                        {
                            [&](const Array& arr)
                            {
                                throw Exception_Iter(m_Cursor, "Can't print array");
                            },
                            [](const auto& obj)
                            {
                                std::cout << obj.value;
                            },
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

    // INPUT <?question>; <variable>
    void Interpreter::HandleInput()
	{
		// INPUT
		++m_Cursor;

        try
        {
            // <question>
            if (m_Cursor != m_End && m_Cursor->type == Token::Type::Literal_String)
            {
                std::cout << m_Cursor->value;

                // <question>
                ++m_Cursor;

                if (m_Cursor == m_End || m_Cursor->type != Token::Type::Semicolon)
                    throw Exception_Iter(m_Cursor, "Expected ;");

                // ;
                ++m_Cursor;
            }

            if (m_Cursor != m_End && m_Cursor->type == Token::Type::Symbol)
            {
                std::string line;
                std::getline(std::cin >> std::ws, line);

                m_Variables.Set(m_Cursor->value, String { line });

                // <variable>
                ++m_Cursor;
            }
            else
                throw Exception_Iter(m_Cursor, "Expected variable name");
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

	int Interpreter::ParseArrayIndex(Token::Iter& iter)
	{
		if (iter->type != Token::Type::Parenthesis_Open)
			throw Exception_Iter(iter, "Expected (");

		++iter;

		try
		{
			auto [res, end] = ParseExpression(iter);

			if (!std::holds_alternative<Numeric>(res))
				throw Exception_Iter(iter, "Array index must be numeric");

			int index = (int)std::get<Numeric>(res).value;

			if (end == m_End || end->type != Token::Type::Parenthesis_Close)
				throw Exception_Iter(end, "Expected )");

			++end;
			iter = end;

			return index;
		}
		catch (const Exception_Iter& e)
		{
			throw e;
		}
	}

	// DIM <name>(<size>) [, <name>(<size>) , ...]
	void Interpreter::HandleDim()
	{
		// DIM
		++m_Cursor;

		while (true)
		{
			// <name>
			if (m_Cursor->type != Token::Type::Symbol)
				throw Exception_Iter(m_Cursor, "Expected array name");

			std::string arrayName = m_Cursor->value;
			++m_Cursor;

			// (<size>)
			if (m_Cursor->type != Token::Type::Parenthesis_Open)
				throw Exception_Iter(m_Cursor, "Expected ( after array name");

			try
			{
				int size = ParseArrayIndex(m_Cursor);

				if (size <= 0)
					throw Exception_Iter(m_Cursor, "Array size must be positive");

                Array arr{ std::vector<Numeric>(size, Numeric{ 0.0 }) };
				m_Variables.Set(arrayName, arr);
			}
			catch (const Exception_Iter& e)
			{
				throw e;
			}

			// Check for comma to continue or end
			if (!IsEnd() && m_Cursor->type == Token::Type::Comma)
			{
				++m_Cursor;
				continue;
			}
			else
				break;
		}
	}

    // RETURN
    void Interpreter::HandleReturn()
    {
        // RETURN
        ++m_Cursor;

        if (m_SubStack.empty())
            throw Exception_Iter(m_Cursor, "RETURN without GOSUB");

        SubNode& node = m_SubStack.back();

        m_NextLine = node.line;
        m_LineOffset = node.posInLine;

        m_SubStack.pop_back();
    }

    // LIST
    void Interpreter::HandleList()
    {
        // LIST
        ++m_Cursor;

        for (const auto& [line, tokens] : m_Programm)
            std::cout << line << TokensToString(tokens) << std::endl;
    }

    // NEW
    void Interpreter::HandleNew()
    {
        // NEW
        ++m_Cursor;

        m_Programm.clear();
        m_Variables.Clear();
    }

    // RUN
    void Interpreter::HandleRun()
    {
        // RUN
        ++m_Cursor;

        Reset();

        auto line = m_Programm.begin();

        while (line != m_Programm.end())
        {
            try
            {
                RunLine(line->second, line->first);

                if (m_NextLine == Result_Terminate)
                    line = m_Programm.end();
                else if (m_NextLine == Result_NextLine)
                    line++;
                else
                    line = m_Programm.find(m_NextLine);
            }
            catch (const Exception_Iter& e)
            {
                throw GenerateException(line->second, TokensToString(line->second), e);
            }
        }

        throw 0;
    }

    // LOAD
    void Interpreter::HandleLoad()
    {
        // LOAD
        ++m_Cursor;

        // <path>
        if (m_Cursor->type != Token::Type::Literal_String)
            throw Exception_Iter(m_Cursor, "Expected file path");

        std::ifstream ifs(m_Cursor->value);

        if (!ifs.is_open())
            throw Exception_Iter(m_Cursor, "Can't open file");

        m_Programm.clear();

        // Save state

        int nextLine = m_NextLine;
        int lineOffset = m_LineOffset;

        Token::Iter cursor = m_Cursor;
        Token::Iter end = m_End;

        auto forStack = m_ForStack;
        bool skipElse = m_SkipElse;

        Parser parser;

        while (!ifs.eof())
        {
            std::string buf;
            std::getline(ifs, buf);

            std::vector<Token> tokens;
            parser.Tokenise(buf, tokens);

            RunLine(tokens);
        }

        ifs.close();

        // Restore state

        m_NextLine = nextLine;
        m_LineOffset = lineOffset;

        // <path>
        m_Cursor = cursor + 1;
        m_End = end;

        m_ForStack = forStack;

        m_SkipElse = skipElse;
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
            {
                if (end != m_End && end != m_Cursor)
                    throw Exception_Iter(std::prev(end), "Expected line number to be numeric");
                else
                    throw Exception_Iter(m_Cursor, "Expected line number to be numeric");
            }

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
            {
                if (iter != m_End && iter != m_Cursor)
                    throw Exception_Iter(std::prev(iter), "Expected expression result to be numeric");
                else
                    throw Exception_Iter(m_Cursor, "Expected expression result to be numeric");
            }

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
                while (iter != m_End)
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

	// FOR <var> = <expr> TO <expr> [ STEP <expr> ]
	void Interpreter::HandleFor()
	{
		// FOR
		++m_Cursor;

		// <var>
		if (m_Cursor->type != Token::Type::Symbol)
            throw Exception_Iter(m_Cursor, "Expected variable name");

		std::string varName = m_Cursor->value;
		++m_Cursor;

		// =
		if (m_Cursor->type != Token::Type::Operator || m_Cursor->value != "=")
            throw Exception_Iter(m_Cursor, "Expected =");

		++m_Cursor;

		try
		{
			// <start expr>
			auto [startRes, startEnd] = ParseExpression(m_Cursor);

			if (!std::holds_alternative<Numeric>(startRes))
                throw Exception_Iter(m_Cursor, "Start value must be numeric");

			// TO
			if (startEnd->type != Token::Type::Keyword_To)
                throw Exception_Iter(startEnd, "Expected TO");

			auto iter = startEnd + 1;

			// <end expr>
			auto [endRes, endEnd] = ParseExpression(iter);

			if (!std::holds_alternative<Numeric>(endRes))
                throw Exception_Iter(iter, "End value must be numeric");

			Real step = 1.0;

			// STEP ?
			if (endEnd->type == Token::Type::Keyword_Step)
			{
				iter = endEnd + 1;

				auto [stepRes, stepEnd] = ParseExpression(iter);

				if (!std::holds_alternative<Numeric>(stepRes))
                    throw Exception_Iter(iter, "Step value must be numeric");

				step = std::get<Numeric>(stepRes).value;

				m_Cursor = stepEnd;
			}
			else
				m_Cursor = endEnd;

			// Create and push a new for node
			m_ForStack.push_back(ForNode {
				.varName = varName,
				.line = -1,
				.posInLine = -1,
				.startValue = std::get<Numeric>(startRes).value,
				.endValue = std::get<Numeric>(endRes).value,
				.step = step
			});

			// Set the variable to start value
			m_Variables.Set(varName, startRes);
		}
        catch (const Exception_Iter& e)
        {
            throw e;
        }
	}

	// NEXT [ <var> ]
	void Interpreter::HandleNext()
	{
		// NEXT
		++m_Cursor;

		if (m_ForStack.empty())
            throw Exception_Iter(std::prev(m_Cursor), "NEXT without FOR");

		ForNode& node = m_ForStack.back();

		// <?var>
		if (m_Cursor != m_End && m_Cursor->type == Token::Type::Symbol)
		{
			if (m_Cursor->value != node.varName)
                throw Exception_Iter(m_Cursor, "Variable name mismatch");

			++m_Cursor;
		}

		// Increment the variable
		auto value = m_Variables.Get(node.varName);

		if (value)
		{
			Object& obj = value.value().get();
			if (std::holds_alternative<Numeric>(obj))
			{
				Real curValue = std::get<Numeric>(obj).value;
				curValue += node.step;

				m_Variables.Set(node.varName, Numeric{ curValue });

				if ((node.step > 0 && curValue > node.endValue) ||
				    (node.step < 0 && curValue < node.endValue))
				{
					// Loop is finished
					m_ForStack.pop_back();
                    m_NextLine = -1;
				}
				else
				{
					// Continue loop
					m_NextLine = node.line;
				}
			}
			else
                throw Exception_Iter(std::prev(m_Cursor), "For loop variable is not numeric");
		}
		else
            throw Exception_Iter(std::prev(m_Cursor), "For loop variable is not numeric");
	}

	// SLEEP <milliseconds>
	void Interpreter::HandleSleep()
	{
		// SLEEP
		++m_Cursor;

        try
        {
            // <milliseconds>
            auto [res, end] = ParseExpression(m_Cursor);

            if (!std::holds_alternative<Numeric>(res))
                throw Exception_Iter(m_Cursor, "Sleep time must be numeric");

            std::this_thread::sleep_for(std::chrono::milliseconds((long long)std::get<Numeric>(res).value));

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
            {
                if (end != m_End && end != m_Cursor)
                    throw Exception_Iter(std::prev(end), "Expected line number to be numeric");
                else
                    throw Exception_Iter(m_Cursor, "Expected line number to be numeric");
            }

            m_NextLine = (int)std::get<Numeric>(res).value;
            m_Cursor = end;
        }
        catch (const Exception_Iter& e)
        {
            throw e;
        }
	}
}
