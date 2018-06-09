#pragma once

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
