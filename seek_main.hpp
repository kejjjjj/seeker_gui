#pragma once

#include <variant>
#include <regex>
#include <string>

enum struct ESearchType : char { standard, regex };

struct RegexData
{
	std::regex regex;
	std::size_t patternLength;
};

struct CSearchData
{
	std::variant<std::string, RegexData> variant;
	ESearchType type{};
};
