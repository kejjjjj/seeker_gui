#include "utils.hpp"

irect adjust_from_640x480(const irect& t)
{
	auto real_w = GetSystemMetrics(SM_CXSCREEN);
	auto real_h = GetSystemMetrics(SM_CYSCREEN);

	auto scaleX = real_w / 640.f;
	auto scaleY = real_h / 480.f;

	return
	{
		static_cast<int>(static_cast<float>(t.x) * scaleX),
		static_cast<int>(static_cast<float>(t.y) * scaleY),
		static_cast<int>(static_cast<float>(t.w) * scaleX),
		static_cast<int>(static_cast<float>(t.h) * scaleY),
	};

}
irect adjust_to_640x480(const irect& t)
{
	auto real_w = GetSystemMetrics(SM_CXSCREEN);
	auto real_h = GetSystemMetrics(SM_CYSCREEN);

	auto scaleX = 640.f / real_w;
	auto scaleY = 480.f / real_h;

	return
	{
		static_cast<int>(static_cast<float>(t.x) * scaleX),
		static_cast<int>(static_cast<float>(t.y) * scaleY),
		static_cast<int>(static_cast<float>(t.w) * scaleX),
		static_cast<int>(static_cast<float>(t.h) * scaleY),
	};
}
int adjust_from_640(const int x)
{
	auto real_w = GetSystemMetrics(SM_CXSCREEN);
	auto scaleX = real_w / 640.f;
	return static_cast<int>(static_cast<float>(x) * scaleX);

}
int adjust_from_480(const int y)
{
	auto real_h = GetSystemMetrics(SM_CYSCREEN);
	auto scaleY = real_h / 480.f;
	return static_cast<int>(static_cast<float>(y) * scaleY);
}
int adjust_to_640(const int x)
{
	auto real_w = GetSystemMetrics(SM_CXSCREEN);
	auto scaleX = 640.f / real_w;
	return static_cast<int>(static_cast<float>(x) * scaleX);

}
int adjust_to_480(const int y)
{
	auto real_h = GetSystemMetrics(SM_CYSCREEN);
	auto scaleY = 480.f / real_h;
	return static_cast<int>(static_cast<float>(y) * scaleY);
}
irect get_parent_dimensions(HWND parent)
{
	RECT r;
	GetClientRect(parent, &r);
	return adjust_to_640x480({ r.left, r.top, r.right - r.left, r.bottom - r.top });
}