#pragma once

#include <limits>

namespace Basic
{
	typedef unsigned char Byte;

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
            Power,
			And,
			Or
		};

		Type type;
		Byte precedence;
		Byte arguments;

		static constexpr Byte MAX_PRECEDENCE = std::numeric_limits<Byte>::max();
	};
}
