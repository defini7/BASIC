#pragma once

#include <limits>

namespace def
{
	typedef unsigned char Uint8;

	struct Operator
	{
		enum class Type
		{
			Subtraction,
			Addition,
			Multiplication,
			Division,
			Equals,
			NotEquals,
			Less,
			Greater,
			LessEquals,
			GreaterEquals,
            Assign,
            Power
		};

		Type type;
		Uint8 precedence;
		Uint8 arguments;

		static constexpr Uint8 MAX_PRECEDENCE = std::numeric_limits<Uint8>::max();
	};
}
