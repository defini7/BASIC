#include "../Include/VarStorage.hpp"

namespace Basic
{
	void VarStorage::Set(const std::string& name, const Object& value)
	{
		m_Values[name] = value;
	}

	std::optional<std::reference_wrapper<Object>> VarStorage::Get(const std::string& name)
	{
		auto it = m_Values.find(name);
		if (it == m_Values.end())
			return std::nullopt;
		return it->second;
	}

	void VarStorage::Clear()
	{
		m_Values.clear();
	}
}
