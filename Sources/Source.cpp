#include <iostream>

#include "Include/Interpreter.hpp"

int main()
{
    std::cout << "MSX-like BASIC version 0.1\n";
    std::cout << "Repository: github.com/defini7/BASIC\n" << std::endl;

	Basic::Parser parser;
	Basic::Interpreter interpreter;

	std::string input;

    while (true)
    {
        std::getline(std::cin, input);

        bool programmMode = false;

		try
		{
			std::vector<Basic::Token> tokens;

            try
            {
                parser.Tokenise(input, tokens);
                programmMode = interpreter.RunLine(tokens);
            }
            catch (const Basic::Exception_Iter& e)
            {
                throw Basic::GenerateException(tokens, input, e);
            }
		}
        catch (const Basic::Exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

        if (!programmMode)
            std::cout << "Ok" << std::endl;
	}

	return 0;
}
