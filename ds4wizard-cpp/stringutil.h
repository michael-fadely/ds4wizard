#pragma once

#include <string>

bool iequalsw(const wchar_t& a, const wchar_t& b);

bool iequals(const std::wstring& a, const std::wstring& b);

bool iequalsc(const char& a, const char& b);

bool iequals(const std::string& a, const std::string& b);

// source: https://stackoverflow.com/a/217605

// trim from start (in place)
void triml(std::string& s);

// trim from end (in place)
void trimr(std::string& s);

// trim from both ends (in place)
void trim(std::string& s);

// trim from start (copying)
std::string triml_copy(std::string s);

// trim from end (copying)
std::string trimr_copy(std::string s);

// trim from both ends (copying)
std::string trim_copy(std::string s);
