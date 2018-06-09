#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

class Handle
{
public:
	bool owner = false;
	HANDLE nativeHandle = nullptr;

	Handle() = default;
	Handle(const Handle& other);

	Handle(Handle&& rhs) noexcept;
	explicit Handle(HANDLE h, bool owner = false);
	~Handle();

	void close();

	bool operator==(const Handle& rhs) const;
	bool operator!=(const Handle& rhs) const;

	Handle& operator=(const Handle& rhs);
	Handle& operator=(Handle&& rhs) noexcept;

	bool isValid() const;
};
