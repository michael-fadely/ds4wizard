#include "hid_handle.h"

Handle::Handle(const Handle& other)
{
	*this = other;
}

Handle::Handle(Handle&& rhs) noexcept
{
	*this = rhs;
}

Handle::Handle(HANDLE h, bool owner)
{
	nativeHandle = h;
	this->owner  = owner;
}

Handle::~Handle()
{
	close();
}

void Handle::close()
{
	if (owner && isValid())
	{
		CloseHandle(nativeHandle);
		nativeHandle = nullptr;
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

Handle& Handle::operator=(const Handle& rhs)
{
	close();

	nativeHandle = rhs.nativeHandle;
	owner        = rhs.owner;

	return *this;
}

Handle& Handle::operator=(Handle&& rhs) noexcept
{
	close();

	nativeHandle = rhs.nativeHandle;
	owner        = rhs.owner;

	rhs.nativeHandle = nullptr;
	rhs.owner        = false;

	return *this;
}

bool Handle::isValid() const
{
	return nativeHandle && nativeHandle != INVALID_HANDLE_VALUE;
}
