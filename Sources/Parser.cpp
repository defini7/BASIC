#include "Include/Parser.hpp"

namespace def
{
	void String_ToLower(std::string& s)
	{
		for (char& c : s)
		{
			if (c <= 'Z' && c >= 'A')
				c += 'z' - 'Z';
		}
	}

	int Parser::Tokenise(std::string_view input, std::vector<Token>& tokens)
	{
		State stateNow = State::NewToken;
		State stateNext = State::NewToken;

		Token token;

		auto currentChar = input.begin();

		// If they remain 0 at the end then ok
		int parenthesesBalancer = 0;
		int quotesBalancer = 0;

		auto StartToken = [&](Token::Type type, State nextState = State::CompleteToken, bool push = true)
			{
				token.type = type;

				if (push)
					token.value.push_back(*currentChar);

				stateNext = nextState;
			};

		auto AppendChar = [&](State nextState)
			{
				token.value += *currentChar;
				stateNext = nextState;
				currentChar++;
			};

		while (currentChar != input.end())
		{
			// FDA - First Digit Analysis

			if (stateNow == State::NewToken)
			{
				// Skip whitespace
				if (guard::Whitespaces[*currentChar])
				{
					stateNext = State::NewToken;
					currentChar++;
					continue;
				}

				// Check for a digit
				if (guard::DecDigits[*currentChar])
					StartToken(Token::Type::Literal_NumericBase10, State::Literal_NumericBase10);

				else if (*currentChar == '&')
					StartToken(Token::Type::Literal_NumericBaseUnknown, State::Literal_NumericBaseUnknown, false);

				// Check for an operator
				else if (guard::Operators[*currentChar])
					StartToken(Token::Type::Operator, State::Operator);

				// Check for a string literal
				else if (guard::Quotes[*currentChar])
				{
					StartToken(Token::Type::Literal_String, State::Literal_String, false);
					quotesBalancer++;
				}

				// Check for a symbol (e.g. abc123, something_with_underscore)
				else if (guard::Symbols[*currentChar])
					StartToken(Token::Type::Symbol, State::Symbol);

				else if (*currentChar == '(')
				{
					StartToken(Token::Type::Parenthesis_Open);
					parenthesesBalancer++;
				}

				else if (*currentChar == ')')
				{
					StartToken(Token::Type::Parenthesis_Close);
					parenthesesBalancer--;
				}

				else if (*currentChar == ':')
					StartToken(Token::Type::Colon);
				
				else if (*currentChar == ';')
					StartToken(Token::Type::Semicolon);

				else
					throw ParserException(std::string("Unexpected character: ") + *currentChar);

				currentChar++;
			}
			else
			{
				// Perform something based on the current state

				switch (stateNow)
				{
				case State::Literal_NumericBaseUnknown:
				{
				#define Prepare(base) do { token.type = Token::Type::Literal_NumericBase##base; stateNext = State::Literal_NumericBase##base; } while (0)

					if (guard::Prefixes[*currentChar])
					{
						// Determine base of numeric literal

						if (*currentChar == 'h' || *currentChar == 'H') Prepare(16);
						else if (*currentChar == 'o' || *currentChar == 'O') Prepare(8);

						currentChar++;
					}
					else
					{
						// If there is no H or O after the & then number must be treated as a binary
						
						if (*currentChar == '0' || *currentChar == '1')
						{
							Prepare(2);
							AppendChar(State::Literal_NumericBase2);
						}
						else
							throw ParserException("Unknown prefix for numeric literal");
					}

				#undef Prepare
				}
				break;

			#define CASE_LITERAL_NUMERIC_BASE(base, name) \
				case State::Literal_NumericBase##base: \
				{ \
					if (guard::name##Digits[*currentChar]) \
						AppendChar(State::Literal_NumericBase##base); \
					else \
					{ \
						if (guard::Symbols[*currentChar]) \
							throw ParserException("Invalid numeric literal or symbol"); \
						stateNext = State::CompleteToken; \
					} \
				} \
				break;

				CASE_LITERAL_NUMERIC_BASE(16, Hex)
				CASE_LITERAL_NUMERIC_BASE(10, Dec)
				CASE_LITERAL_NUMERIC_BASE(8, Oct)
				CASE_LITERAL_NUMERIC_BASE(2, Bin)

				case State::Literal_String:
				{
					// Read string

					if (guard::Quotes[*currentChar])
					{
						quotesBalancer--;
						stateNext = State::CompleteToken;
						currentChar++;
					}
					else
						AppendChar(State::Literal_String);
				}
				break;

				case State::Operator:
				{
					if (guard::Operators[*currentChar])
					{
						// If we found an operator then continue searching for a longer operator
						if (s_Operators.contains(token.value + *currentChar))
							AppendChar(State::Operator);
						else
						{
							// If we don't have an operator with the currently appended character then
							// proceed with the current operator

							if (s_Operators.contains(token.value))
								stateNext = State::CompleteToken;
							else
							{
								// If on the current stage we still can't find an operator
								// then probably the operator is not completed yet
								// so continue appending characters

								AppendChar(State::Operator);
							}
						}
					}
					else
					{
						// If current character is not a part of the operator characters
						// and current text is a valid operator say that it's done

						if (s_Operators.contains(token.value))
							stateNext = State::CompleteToken;
						else
							throw ParserException("Invalid operator was found: " + token.value);
					}
				}
				break;

				case State::Symbol:
				{
					// Note: we treat all invalid symbols (e.g. 123abc) in the Literal_Numeric state

					if (guard::Symbols[*currentChar] || guard::DecDigits[*currentChar])
						AppendChar(State::Symbol);
					else
					{
					parse_keyword:
						String_ToLower(token.value);

					#define Keyword(signature, name) if (token.value == signature) token.type = Token::Type::Keyword_##name

						Keyword("print", Print);
						Keyword("input", Input);
						Keyword("cls", Cls);
						Keyword("let", Let);
						Keyword("rem", Rem);
						Keyword("goto", Goto);
						Keyword("if", If);
						Keyword("then", Then);
						Keyword("else", Else);
						Keyword("for", For);
						Keyword("to", To);
						Keyword("step", Step);
						Keyword("next", Next);
						Keyword("sleep", Sleep);
						Keyword("sin", Sin);
						Keyword("cos", Cos);
						Keyword("tan", Tan);
						Keyword("arcsin", ArcSin);
						Keyword("arccos", ArcCos);
						Keyword("arctan", ArcTan);
						Keyword("sqr", Sqrt);
						Keyword("log", Log);
						Keyword("exp", Exp);
						Keyword("abs", Abs);
						Keyword("sgn", Sign);
						Keyword("int", Int);
                        Keyword("rnd", Random);
						Keyword("list", List);
						Keyword("run", Run);
						Keyword("new", New);

						if (currentChar == input.end())
							goto complete_token;

					#undef Keyword

						stateNext = State::CompleteToken;
					}
				}
				break;

				case State::CompleteToken:
				{
				complete_token:
					stateNext = State::NewToken;
					tokens.push_back(token);

					token.type = Token::Type::None;
					token.value.clear();
				}
				break;

				}
			}

			stateNow = stateNext;
		}

		if (stateNext == State::Symbol)
			goto parse_keyword;

		if (parenthesesBalancer != 0)
			throw ParserException("Parentheses were not balanced");

		if (quotesBalancer != 0)
			throw ParserException("Quotes were not balanced");

		// Drain out the last token
		if (!token.value.empty())
			tokens.push_back(token);
		
		// In the original BASIC I guess "2.5 + 2" must be treated as "2 .5 + 2" but for now let's treat it as "2.5 + 2"
		if (tokens.empty() || tokens.front().type != Token::Type::Literal_NumericBase10 || tokens.front().value.find('.') != std::string::npos)
			return -1;

		int line = std::stoi(tokens.front().value);
		tokens.erase(tokens.begin());

		return line;
	}

	std::unordered_map<std::string, Operator> Parser::s_Operators =
	{
		{"=", { Operator::Type::Assign, 0, 2 } },
		{"==", { Operator::Type::Equals, 1, 2 } },
		{"<>", { Operator::Type::NotEquals, 1, 2 } },
		{"<", { Operator::Type::Less, 1, 2 } },
		{">", { Operator::Type::Greater, 1, 2 } },
		{"<=", { Operator::Type::LessEquals, 1, 2 } },
		{">=", { Operator::Type::GreaterEquals, 1, 2 } },
		{"-", { Operator::Type::Subtraction, 2, 2 } },
		{"+", { Operator::Type::Addition, 2, 2 } },
		{"*", { Operator::Type::Multiplication, 3, 2 } },
		{"/", { Operator::Type::Division, 3, 2 } },

		// Unary operators (in that way they are easier to handle)
		{"u-", { Operator::Type::Subtraction, Operator::MAX_PRECEDENCE, 1 } },
		{"u+", { Operator::Type::Addition, Operator::MAX_PRECEDENCE, 1 } },
	};
}
