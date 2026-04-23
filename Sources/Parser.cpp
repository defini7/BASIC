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

		auto ClassifyKeyword = [&]()
		{
			String_ToUpper(token.value);

			static const std::unordered_map<std::string, Token::Type> s_KeywordMap =
			{
				{"PRINT", Token::Type::Keyword_Print},
				{"INPUT", Token::Type::Keyword_Input},
				{"CLS", Token::Type::Keyword_Cls},
				{"LET", Token::Type::Keyword_Let},
				{"REM", Token::Type::Keyword_Rem},
				{"GOTO", Token::Type::Keyword_Goto},
				{"IF", Token::Type::Keyword_If},
				{"THEN", Token::Type::Keyword_Then},
				{"ELSE", Token::Type::Keyword_Else},
				{"FOR", Token::Type::Keyword_For},
				{"TO", Token::Type::Keyword_To},
				{"STEP", Token::Type::Keyword_Step},
				{"NEXT", Token::Type::Keyword_Next},
				{"SLEEP", Token::Type::Keyword_Sleep},
				{"SIN", Token::Type::Keyword_Sin},
				{"COS", Token::Type::Keyword_Cos},
				{"TAN", Token::Type::Keyword_Tan},
				{"ARCSIN", Token::Type::Keyword_ArcSin},
				{"ARCCOS", Token::Type::Keyword_ArcCos},
				{"ARCTAN", Token::Type::Keyword_ArcTan},
				{"SQR", Token::Type::Keyword_Sqrt},
				{"LN", Token::Type::Keyword_Ln},
				{"LOG", Token::Type::Keyword_Log},
				{"EXP", Token::Type::Keyword_Exp},
				{"ABS", Token::Type::Keyword_Abs},
				{"SGN", Token::Type::Keyword_Sign},
				{"INT", Token::Type::Keyword_Int},
				{"RND", Token::Type::Keyword_Random},
				{"END", Token::Type::Keyword_End},
				{"GOSUB", Token::Type::Keyword_GoSub},
				{"RETURN", Token::Type::Keyword_Return},
				{"VAL", Token::Type::Keyword_Val},
				{"LIST", Token::Type::Keyword_List},
				{"RUN", Token::Type::Keyword_Run},
				{"NEW", Token::Type::Keyword_New},
				{"LOAD", Token::Type::Keyword_Load},
				{"DIM", Token::Type::Keyword_Dim},
				{"AND", Token::Type::Operator},
				{"OR", Token::Type::Operator}
			};

			auto it = s_KeywordMap.find(token.value);

			if (it != s_KeywordMap.end())
				token.type = it->second;
		};

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
            
            if (*currentChar < 0 || *currentChar >= Guard::SIZE)
                throw Exception(input, (int)std::distance(input.begin(), currentChar), std::string("Unexpected character ") + *currentChar);

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

				else if (*currentChar == '[')
					StartToken(Token::Type::Bracket_Open);

				else if (*currentChar == ']')
					StartToken(Token::Type::Bracket_Close);

				else if (*currentChar == ':')
					StartToken(Token::Type::Colon);
				
				else if (*currentChar == ';')
					StartToken(Token::Type::Semicolon);

				else if (*currentChar == ',')
					StartToken(Token::Type::Comma);

				else
				{
					// In MSX BASIC you can write any symbols in comments (i.e. after REM keyword)
					// but because comments are handled at interpreter stage you can't do the same thing here
                    throw Exception(input, (int)std::distance(input.begin(), currentChar), std::string("Unexpected character ") + *currentChar);
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
					auto SetNumericBase = [&](int base)
					{
						switch (base)
						{
						case 2:
							token.type = Token::Type::Literal_NumericBase2;
							stateNext = State::Literal_NumericBase2;
							break;
						case 8:
							token.type = Token::Type::Literal_NumericBase8;
							stateNext = State::Literal_NumericBase8;
							break;
						case 10:
							token.type = Token::Type::Literal_NumericBase10;
							stateNext = State::Literal_NumericBase10;
							break;
						case 16:
							token.type = Token::Type::Literal_NumericBase16;
							stateNext = State::Literal_NumericBase16;
							break;
						}
					};

					if (Guard::Prefixes[*currentChar])
					{
						// Determine base of numeric literal

						if (*currentChar == 'h' || *currentChar == 'H')
							SetNumericBase(16);
						else if (*currentChar == 'o' || *currentChar == 'O')
							SetNumericBase(8);

						currentChar++;
					}
					else
					{
						// If there is no H or O after the & then number must be treated as a binary
						
						if (*currentChar == '0' || *currentChar == '1')
						{
							SetNumericBase(2);
							AppendChar(State::Literal_NumericBase2);
						}
						else
							throw Exception(input, (int)std::distance(input.begin(), currentChar), "Unknown prefix for numeric literal");
					}
				}
				break;

				case State::Literal_NumericBase16:
				{
					if (Guard::HexDigits[*currentChar])
						AppendChar(State::Literal_NumericBase16);
					else
					{
						if (Guard::Symbols[*currentChar])
							throw Exception(input, (int)std::distance(input.begin(), currentChar), "Invalid numeric literal or symbol");
						stateNext = State::CompleteToken;
					}
				}
				break;

				case State::Literal_NumericBase10:
				{
					if (Guard::DecDigits[*currentChar])
						AppendChar(State::Literal_NumericBase10);
					else
					{
						if (Guard::Symbols[*currentChar])
							throw Exception(input, (int)std::distance(input.begin(), currentChar), "Invalid numeric literal or symbol");
						stateNext = State::CompleteToken;
					}
				}
				break;

				case State::Literal_NumericBase8:
				{
					if (Guard::OctDigits[*currentChar])
						AppendChar(State::Literal_NumericBase8);
					else
					{
						if (Guard::Symbols[*currentChar])
							throw Exception(input, (int)std::distance(input.begin(), currentChar), "Invalid numeric literal or symbol");
						stateNext = State::CompleteToken;
					}
				}
				break;

				case State::Literal_NumericBase2:
				{
					if (Guard::BinDigits[*currentChar])
						AppendChar(State::Literal_NumericBase2);
					else
					{
						if (Guard::Symbols[*currentChar])
							throw Exception(input, (int)std::distance(input.begin(), currentChar), "Invalid numeric literal or symbol");
						stateNext = State::CompleteToken;
					}
				}
				break;

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
                            throw Exception(input, (int)std::distance(input.begin(), currentChar), "Invalid operator was found: " + token.value);
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
						ClassifyKeyword();

						if (currentChar == input.end())
						{
							// To make sure that the last token is pushed if we reached the end of the string
							currentChar = input.end() - 1;
						}

						stateNext = State::CompleteToken;
					}
				}
				break;

				case State::CompleteToken:
				{
					stateNext = State::NewToken;
					tokens.push_back(token);

					token.type = Token::Type::None;
					token.value.clear();
				}
				break;
                        
                default: /* Unreachable */ break;

				}
			}

			stateNow = stateNext;
		}

		if (stateNow == State::Symbol)
			ClassifyKeyword();

		if (parenthesesBalancer != 0)
            throw Exception(input, (int)std::distance(input.begin(), currentChar), "Parentheses were not balanced");

		if (quotesBalancer != 0)
            throw Exception(input, (int)std::distance(input.begin(), currentChar), "Quotes were not balanced");

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
