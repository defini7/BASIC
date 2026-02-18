#include "../Include/Parser.hpp"

namespace Basic
{
    void String_ToUpper(std::string& s)
	{
		for (char& c : s)
		{
            if ('a' <= c && c <= 'z')
                c += 'Z' - 'z';
		}
	}

    void Parser::Tokenise(const std::string& input, std::vector<Token>& tokens)
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
				if (Guard::Whitespaces[*currentChar])
				{
					stateNext = State::NewToken;
					currentChar++;
					continue;
				}

				// Check for a digit
				if (Guard::DecDigits[*currentChar])
					StartToken(Token::Type::Literal_NumericBase10, State::Literal_NumericBase10);

				else if (*currentChar == '&')
					StartToken(Token::Type::Literal_NumericBaseUnknown, State::Literal_NumericBaseUnknown, false);

				// Check for an operator
				else if (Guard::Operators[*currentChar])
					StartToken(Token::Type::Operator, State::Operator);

				// Check for a string literal
				else if (Guard::Quotes[*currentChar])
				{
					StartToken(Token::Type::Literal_String, State::Literal_String, false);
					quotesBalancer++;
				}

				// Check for a symbol (e.g. abc123, something_with_underscore)
				else if (Guard::Symbols[*currentChar])
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
				{
					// In MSX BASIC you can write any symbols in comments (i.e. after REM keyword)
					// but because comments are handled at interpreter stage you can't do the same thing here
                    throw Exception(input, std::distance(input.begin(), currentChar), std::string("Unexpected character ") + *currentChar);
				}

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

					if (Guard::Prefixes[*currentChar])
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
                            throw Exception(input, std::distance(input.begin(), currentChar), "Unknown prefix for numeric literal");
					}

				#undef Prepare
				}
				break;

			#define CASE_LITERAL_NUMERIC_BASE(base, name) \
				case State::Literal_NumericBase##base: \
				{ \
					if (Guard::name##Digits[*currentChar]) \
						AppendChar(State::Literal_NumericBase##base); \
					else \
					{ \
						if (Guard::Symbols[*currentChar]) \
                            throw Exception(input, std::distance(input.begin(), currentChar), "Invalid numeric literal or symbol"); \
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

					if (Guard::Quotes[*currentChar])
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
					if (Guard::Operators[*currentChar])
					{
						// If we found an operator then continue searching for a longer operator
						if (s_Operators.contains(token.value + *currentChar))
							AppendChar(State::Operator);
						else
						{
							// If we don't have an operator with the currently appended character then
							// proceed with the current operator

							if (s_Operators.contains(token.value))
							{
								String_ToUpper(token.value);
								stateNext = State::CompleteToken;
							}
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
                            throw Exception(input, std::distance(input.begin(), currentChar), "Invalid operator was found: " + token.value);
					}
				}
				break;

				case State::Symbol:
				{
					// Note: we treat all invalid symbols (e.g. 123abc) in the Literal_Numeric state

					if (Guard::Symbols[*currentChar] || Guard::DecDigits[*currentChar])
						AppendChar(State::Symbol);
					else
					{
					parse_keyword:
                        String_ToUpper(token.value);

					#define KEYWORD(signature, name) if (token.value == signature) token.type = Token::Type::Keyword_##name

                        KEYWORD("PRINT", Print);
                        KEYWORD("INPUT", Input);
                        KEYWORD("CLS", Cls);
                        KEYWORD("LET", Let);
                        KEYWORD("REM", Rem);
                        KEYWORD("GOTO", Goto);
                        KEYWORD("IF", If);
                        KEYWORD("THEN", Then);
                        KEYWORD("ELSE", Else);
                        KEYWORD("FOR", For);
                        KEYWORD("TO", To);
                        KEYWORD("STEP", Step);
                        KEYWORD("NEXT", Next);
                        KEYWORD("SLEEP", Sleep);
                        KEYWORD("SIN", Sin);
                        KEYWORD("COS", Cos);
                        KEYWORD("TAN", Tan);
                        KEYWORD("ARCSIN", ArcSin);
                        KEYWORD("ARCCOS", ArcCos);
                        KEYWORD("ARCTAN", ArcTan);
                        KEYWORD("SQR", Sqrt);
                        KEYWORD("LOG", Log);
                        KEYWORD("EXP", Exp);
                        KEYWORD("ABS", Abs);
                        KEYWORD("SGN", Sign);
                        KEYWORD("INT", Int);
                        KEYWORD("RND", Random);
                        KEYWORD("END", End);
                        KEYWORD("GOSUB", GoSub);
                        KEYWORD("RETURN", Return);
                        KEYWORD("VAL", Val);
                        KEYWORD("LIST", List);
                        KEYWORD("RUN", Run);
                        KEYWORD("NEW", New);
                        KEYWORD("LOAD", Load);

					#undef KEYWORD

						if (token.value == "AND" || token.value == "OR")
							token.type = Token::Type::Operator;

						if (currentChar == input.end())
							goto complete_token;

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
            throw Exception(input, std::distance(input.begin(), currentChar), "Parentheses were not balanced");

		if (quotesBalancer != 0)
            throw Exception(input, std::distance(input.begin(), currentChar), "Quotes were not balanced");

		// Drain out the last token
		if (!token.value.empty())
			tokens.push_back(token);
	}

	std::unordered_map<std::string, Operator> Parser::s_Operators =
	{
		{"=", { Operator::Type::Assign, 0, 2 } },
		{"AND", { Operator::Type::And, 1, 2 } },
		{"OR", { Operator::Type::Or, 2, 2 } },
		{"==", { Operator::Type::Equals, 3, 2 } },
		{"<>", { Operator::Type::NotEquals, 3, 2 } },
		{"<", { Operator::Type::Less, 3, 2 } },
		{">", { Operator::Type::Greater, 3, 2 } },
		{"<=", { Operator::Type::LessEquals, 3, 2 } },
		{">=", { Operator::Type::GreaterEquals, 3, 2 } },
		{"-", { Operator::Type::Subtraction, 4, 2 } },
		{"+", { Operator::Type::Addition, 4, 2 } },
		{"*", { Operator::Type::Multiplication, 5, 2 } },
		{"/", { Operator::Type::Division, 5, 2 } },
        {"^", { Operator::Type::Power, 6, 2 } },

		{"u-", { Operator::Type::Subtraction, Operator::MAX_PRECEDENCE, 1 } },
		{"u+", { Operator::Type::Addition, Operator::MAX_PRECEDENCE, 1 } },
	};
}
