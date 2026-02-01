#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <optional>

namespace def
{
	using Real = long double;

	template <class T>
	struct Type
	{
		T value;
	};

	struct Numeric : Type<Real>
	{
		static constexpr Real MIN = std::numeric_limits<Real>::lowest();
		static constexpr Real MAX = std::numeric_limits<Real>::max();
		static constexpr Real EPS = std::numeric_limits<Real>::epsilon();
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
