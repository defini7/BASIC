#include <iostream>
#include <map>

#include "Interpreter.hpp"

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
						auto pc = programm.begin();

						while (pc != programm.end())
						{
							int newPc = interpreter.Execute(false, pc->second);

							if (newPc == -1)
								pc++;
							else
								pc = programm.find(newPc);
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
								std::cout << ' ' << token.value;

							std::cout << std::endl;
						}
					}
				}
				else
				{
					// Not a CMD command so just execute the line and don't save it anywhere
					interpreter.Execute(true, tokens);
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
