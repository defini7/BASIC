#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <optional>

namespace def
{
	template <class T>
	struct Type
	{
		T value;
	};

	struct Numeric : Type<long double>
	{
		static constexpr long double MIN = std::numeric_limits<long double>::min();
		static constexpr long double MAX = std::numeric_limits<long double>::max();
		static constexpr long double EPS = std::numeric_limits<long double>::epsilon();
	};

	struct String : Type<std::string> {};
	struct Symbol : Type<std::string> {};

	using Object = std::variant<Numeric, String, Symbol>;

	class VarStorage
	{
	public:
		VarStorage() = default;

	public:
		void Set(const std::string& name, const Object& value);

		std::optional<std::reference_wrapper<Object>> Get(const std::string& name);

	private:
		std::unordered_map<std::string, Object> m_Values;

	};
}
