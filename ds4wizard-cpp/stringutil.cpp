#include "pch.h"

#include <cwctype>
#include <cctype>
#include <locale>
#include <ranges>

#include "stringutil.h"

bool iequalsw(const wchar_t& a, const wchar_t& b)
{
	return std::towupper(a) == std::towupper(b);
}

bool iequals(const std::wstring& a, const std::wstring& b)
{
	return a.size() == b.size() && std::ranges::equal(a, b, &iequalsw);
}

bool iequalsc(const char& a, const char& b)
{
	return std::toupper(a) == std::toupper(b);
}

bool iequals(const std::string& a, const std::string& b)
{
	return a.size() == b.size() && std::ranges::equal(a, b, &iequalsc);
}

void triml(std::string& s)
{
	s.erase(s.begin(), std::ranges::find_if(s, [](int ch)
	{
		return !std::isspace(ch);
	}));
}

void trimr(std::string& s)
{
	std::ranges::reverse_view rs(s);
	s.erase(std::ranges::find_if(rs, [](int ch)
	{
		return !std::isspace(ch);
	}).base(), s.end());
}

void trim(std::string& s)
{
	triml(s);
	trimr(s);
}

std::string triml_copy(std::string s)
{
	triml(s);
	return s;
}

std::string trimr_copy(std::string s)
{
	trimr(s);
	return s;
}

std::string trim_copy(std::string s)
{
	trim(s);
	return s;
}
