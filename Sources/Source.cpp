#include <iostream>
#include <map>

#include "Include/Interpreter.hpp"

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
			int line = parser.Tokenise(input, tokens);

			if (line == -1 && !tokens.empty())
			{
				// No line was specified but the first argument is one of CMD commands
				// but be sure that there are no other tokens after the first one

				if (tokens[0].type == def::Token::Type::Keyword_Run)
				{
					if (tokens.size() > 1)
						std::cerr << "Syntax error" << std::endl;
					else
					{
						auto line = programm.begin();

						while (line != programm.end())
						{
							int nextLine = interpreter.Execute(line->second, line->first);

							if (nextLine == -1)
								line++;
							else
								line = programm.find(nextLine);
						}
					}
				}
				else if (tokens[0].type == def::Token::Type::Keyword_List)
				{
					if (tokens.size() > 1)
						std::cerr << "Syntax error" << std::endl;
					else
					{
						// Print full programm
						for (const auto& [line, tokens] : programm)
						{
							std::cout << line;

							for (const auto& token : tokens)
							{
								if (token.type == def::Token::Type::Literal_String)
									std::cout << ' ' << '"' << token.value << '"';
								else
									std::cout << ' ' << token.value;
							}

							std::cout << std::endl;
						}
					}
				}
				else if (tokens[0].type == def::Token::Type::Keyword_New)
					programm.clear();
				else
				{
					// Not a CMD command so just execute the line and don't save it anywhere
					interpreter.Execute(tokens);
				}
			}

			/*for (const auto& token : tokens)
				std::cout << token.ToString() << std::endl;*/

			if (line != -1)
				programm[line] = tokens;
		}
		catch (const def::ParserException& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
	while (input != "quit");

	return 0;
}
