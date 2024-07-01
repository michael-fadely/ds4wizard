#include "pch.h"

#include <algorithm>

#include "pathutil.h"

// TODO: separate function for validating Windows paths. See: https://docs.microsoft.com/en-us/windows/desktop/FileIO/naming-a-file

static const std::string INVALID_CHARS(R"(\/:*?"<>|)");

static char replace_invalid(char c)
{
	if (std::ranges::find(INVALID_CHARS, c) != INVALID_CHARS.cend())
	{
		return '_';
	}

	return c;
}

void makeValidFileName(std::string& str)
{
	std::ranges::transform(str, str.begin(), replace_invalid);
}

std::string validatedFileName(std::string str)
{
	makeValidFileName(str);
	return str;
}
