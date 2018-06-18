#pragma once

#include <algorithm>
#include <string>
#include <cwctype>
#include <cctype>

inline bool iequalsw(const wchar_t& a, const wchar_t& b)
{
	return std::towupper(a) == std::towupper(b);
}

inline bool iequals(const std::wstring& a, const std::wstring& b)
{
	return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), &iequalsw);
}

inline bool iequalsc(const char& a, const char& b)
{
	return std::toupper(a) == std::toupper(b);
}

inline bool iequals(const std::string& a, const std::string& b)
{
	return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), &iequalsc);
}
