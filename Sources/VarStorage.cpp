#include "Include/VarStorage.hpp"

namespace def
{
	void VarStorage::Set(const std::string& name, const Object& value)
	{
		m_Values[name] = value;
	}

	std::optional<std::reference_wrapper<Object>> VarStorage::Get(const std::string& name)
	{
		if (!m_Values.contains(name))
			return std::nullopt;

		return m_Values[name];
	}
}
