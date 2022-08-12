#include "hid_handle.h"
#include <utility>

Handle::Handle(Handle&& other) noexcept
	: owner(std::exchange(other.owner, false)),
	  nativeHandle(std::exchange(other.nativeHandle, INVALID_HANDLE_VALUE))
{
}

Handle::Handle(HANDLE h, bool owner)
	: owner(owner),
	  nativeHandle(h)
{
}

Handle::~Handle()
{
	close();
}

bool Handle::isValid() const
{
	return nativeHandle && nativeHandle != INVALID_HANDLE_VALUE;
}

void Handle::close()
{
	if (owner && isValid())
	{
		CloseHandle(nativeHandle);
		nativeHandle = INVALID_HANDLE_VALUE;
	}
}

bool Handle::operator==(const Handle& rhs) const
{
	return nativeHandle == rhs.nativeHandle;
}

bool Handle::operator!=(const Handle& rhs) const
{
	return !(*this == rhs);
}

Handle& Handle::operator=(Handle&& rhs) noexcept
{
	if (this != &rhs && *this != rhs)
	{
		close();

		owner        = std::exchange(rhs.owner, false);
		nativeHandle = std::exchange(rhs.nativeHandle, INVALID_HANDLE_VALUE);
	}

	return *this;
}
