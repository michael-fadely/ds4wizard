#pragma once

#include <Windows.h>

class Handle
{
public:
	bool owner = false;
	HANDLE nativeHandle = INVALID_HANDLE_VALUE;

	Handle() = default;
	Handle(const Handle& other) = delete;

	Handle(Handle&& other) noexcept;
	explicit Handle(HANDLE h, bool owner = false);
	~Handle();

	[[nodiscard]] bool isValid() const;

	void close();

	[[nodiscard]] bool operator==(const Handle& rhs) const;
	[[nodiscard]] bool operator!=(const Handle& rhs) const;

	Handle& operator=(const Handle& rhs) = delete;
	Handle& operator=(Handle&& rhs) noexcept;
};
