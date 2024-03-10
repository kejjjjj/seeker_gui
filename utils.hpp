#pragma once

#include "seeker_gui.hpp"

template<typename T>
concept arithmetic = requires(T v) { std::is_arithmetic<T>(); };

template<arithmetic T>
struct rect
{
	T x = {};
	T y = {};
	T w = {};
	T h = {};
};

template<arithmetic T>
struct vec2
{
	T x = {};
	T y = {};
};

using irect = rect<int>;
using frect = rect<float>;
using drect = rect<double>;

using ivec2 = vec2<int>;
using fvec2 = vec2<float>;
using dvec2 = vec2<double>;

irect adjust_from_640x480(const irect& target);
irect adjust_to_640x480(const irect& target);
int adjust_from_640(const int x);
int adjust_from_480(const int y);
int adjust_to_640(const int x);
int adjust_to_480(const int y);

irect get_parent_dimensions(HWND parent);
constexpr ivec2 get_rect_center(const irect& target)
{
	return { target.x + target.w / 2, target.y + target.h / 2 };
}
constexpr irect make_rect_centered_around_point(const ivec2& p, const irect& target)
{
	auto diff = ivec2{ p.x - target.x, p.y - target.y };
	return { target.x + diff.x, target.y + diff.y, target.w + diff.x, target.h + diff.y };
}

template<arithmetic T> 
constexpr T ce_abs(const T v) noexcept
{
	return v < 0 ? -v : v;
}
#include <locale>

inline std::wstring convertToWideString(const std::string& string) {

	std::locale loc;
	
	const auto str = string.data();

	auto len = strlen(str);

	std::wstring wide_str;
	wide_str.resize(len);

	std::use_facet<std::ctype<wchar_t>>(loc).widen(str, str + len, wide_str.data());

	return wide_str;
}