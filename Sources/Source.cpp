#include <iostream>
#include <map>

#include "Include/Interpreter.hpp"

def::Exception GenerateException(const std::vector<def::Token>& tokens, const std::string& input, const def::Exception_Iter& exception)
{
    int pos = 0;

    for (auto it = tokens.begin(); it != exception.iterator; ++it)
        pos += it->value.length() + 1;

    return def::Exception(input, pos, exception.message);
}

int main()
{
	def::Parser parser;
	def::Interpreter interpreter;

	std::string input;

	std::map<int, std::vector<def::Token>> programm;

	do
    {
        std::getline(std::cin, input);

		try
		{
			std::vector<def::Token> tokens;

            try
            {
                int line = parser.Tokenise(input, tokens);

                if (line == -1 && !tokens.empty())
                {
                    // No line was specified but the first argument is one of CMD commands
                    // but be sure that there are no other tokens after the first one

                    if (tokens[0].type == def::Token::Type::Keyword_Run)
                    {
                        if (tokens.size() > 1)
                            throw def::Exception(input, tokens[0].value.length(), "Expected nothing after RUN");
                        else
                        {
                            auto line = programm.begin();

                            while (line != programm.end())
                            {
                                try
                                {
                                    int nextLine = interpreter.Execute(line->second, line->first);

                                    if (nextLine == def::Interpreter::Result_Terminate)
                                        line = programm.end();
                                    else if (nextLine == def::Interpreter::Result_NextLine)
                                        line++;
                                    else
                                        line = programm.find(nextLine);
                                }
                                catch (const def::Exception_Iter& e)
                                {
                                    throw GenerateException(line->second, TokensToString(line->second), e);
                                }
                            }
                        }
                    }
                    else if (tokens[0].type == def::Token::Type::Keyword_List)
                    {
                        if (tokens.size() > 1)
                            throw def::Exception(input, tokens[0].value.length(), "Expected nothing after LIST");
                        else
                        {
                            // Print full programm
                            for (const auto& [line, tokens] : programm)
                                std::cout << line << TokensToString(tokens) << std::endl;
                        }
                    }
                    else if (tokens[0].type == def::Token::Type::Keyword_New)
                    {
                        if (tokens.size() > 1)
                            throw def::Exception(input, tokens[0].value.length(), "Expected nothing after NEW");
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
                        catch (const def::Exception_Iter& e)
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
            catch (const def::Exception& e)
            {
                throw e;
            }
		}
        catch (const def::Exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
	while (input != "quit");

	return 0;
}
