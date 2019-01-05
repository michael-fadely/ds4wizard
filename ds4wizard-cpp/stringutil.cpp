#include "pch.h"

#include <cwctype>
#include <cctype>
#include <locale>

#include "stringutil.h"

bool iequalsw(const wchar_t& a, const wchar_t& b)
{
	return std::towupper(a) == std::towupper(b);
}

bool iequals(const std::wstring& a, const std::wstring& b)
{
	return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), &iequalsw);
}

bool iequalsc(const char& a, const char& b)
{
	return std::toupper(a) == std::toupper(b);
}

bool iequals(const std::string& a, const std::string& b)
{
	return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), &iequalsc);
}

void triml(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
	{
		return !std::isspace(ch);
	}));
}

void trimr(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
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
