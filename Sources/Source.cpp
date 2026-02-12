#include <iostream>
#include <map>

#include "Include/Interpreter.hpp"

Basic::Exception GenerateException(const std::vector<Basic::Token>& tokens, const std::string& input, const Basic::Exception_Iter& exception)
{
    int pos = 0;

    for (auto it = tokens.begin(); it != exception.iterator; ++it)
        pos += it->value.length() + 1;

    return Basic::Exception(input, pos, exception.message);
}

int main()
{
	Basic::Parser parser;
	Basic::Interpreter interpreter;

	std::string input;

	std::map<int, std::vector<Basic::Token>> programm;

	do
    {
        std::getline(std::cin, input);

		try
		{
			std::vector<Basic::Token> tokens;

            try
            {
                int line = parser.Tokenise(input, tokens);

                if (line == -1 && !tokens.empty())
                {
                    // No line was specified but the first argument is one of CMD commands
                    // but be sure that there are no other tokens after the first one

                    if (tokens[0].type == Basic::Token::Type::Keyword_Run)
                    {
                        if (tokens.size() > 1)
                            throw Basic::Exception(input, tokens[0].value.length(), "Expected nothing after RUN");
                        else
                        {
                            auto line = programm.begin();

                            while (line != programm.end())
                            {
                                try
                                {
                                    int nextLine = interpreter.Execute(line->second, line->first);

                                    if (nextLine == Basic::Interpreter::Result_Terminate)
                                        line = programm.end();
                                    else if (nextLine == Basic::Interpreter::Result_NextLine)
                                        line++;
                                    else
                                    {
                                        line = programm.find(nextLine);

                                        if (line == programm.end())
                                            throw "Undefined line number: " + std::to_string(nextLine);
                                    }
                                }
                                catch (const Basic::Exception_Iter& e)
                                {
                                    throw GenerateException(line->second, TokensToString(line->second), e);
                                }
                            }
                        }
                    }
                    else if (tokens[0].type == Basic::Token::Type::Keyword_List)
                    {
                        if (tokens.size() > 1)
                            throw Basic::Exception(input, tokens[0].value.length(), "Expected nothing after LIST");
                        else
                        {
                            // Print full programm
                            for (const auto& [line, tokens] : programm)
                                std::cout << line << TokensToString(tokens) << std::endl;
                        }
                    }
                    else if (tokens[0].type == Basic::Token::Type::Keyword_New)
                    {
                        if (tokens.size() > 1)
                            throw Basic::Exception(input, tokens[0].value.length(), "Expected nothing after NEW");
                        else
                            programm.clear();
                    }
                    else
                    {
                        // Not a CMD command so just execute the line and don't save it anywhere
                        try
                        {
                            interpreter.Execute(tokens);
                        }
                        catch (const Basic::Exception_Iter& e)
                        {
                            throw GenerateException(tokens, input, e);
                        }
                    }
                }

                if (line == -1)
                    std::cout << "Ok" << std::endl;
                else
                    programm[line] = tokens;
            }
            catch (const Basic::Exception& e)
            {
                throw e;
            }
		}
        catch (const Basic::Exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
        catch (const std::string& s)
        {
            std::cerr << s << std::endl;
        }

        Basic::String_ToLower(input);
	}
	while (input != "quit");

	return 0;
}
