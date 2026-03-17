#pragma once

#include <array>
#include <string>

namespace Basic
{
	namespace Guard
	{
        constexpr size_t SIZE = 128;
    
		// Uses every char from string as index and sets it to true
		// so we can use it later to check if character corresponds to
		// the specific group of symbols, e.g. DecDigits['.'] == true
		static constexpr std::array<bool, SIZE> Create(const std::string& availableCharacters)
		{
			std::array<bool, SIZE> chars{ false };

			for (auto c : availableCharacters)
				chars[c] = true;

			return chars;
		}

		constexpr auto DecDigits = Create(".0123456789");
		constexpr auto HexDigits = Create("0123456789ABCDEFabcdef");
		constexpr auto OctDigits = Create("01234567");
		constexpr auto BinDigits = Create("01");
        constexpr auto Prefixes = Create("hoHO");
		constexpr auto Whitespaces = Create(" \t\n\r\v");
		constexpr auto Symbols = Create("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789_.$?");
        constexpr auto Operators = Create("+-*/=<>^");
		constexpr auto Quotes = Create("'\"");
	}
}
