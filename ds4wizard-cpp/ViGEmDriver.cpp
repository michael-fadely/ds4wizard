#include "pch.h"

#include <ViGEm/Client.h>

#include "ViGEmDriver.h"

namespace vigem
{
	Driver::Driver(Driver&& rhs) noexcept
		: client(rhs.client)
	{
		rhs.client = nullptr;
	}

	Driver::~Driver()
	{
		close();
	}

	VIGEM_ERROR Driver::open()
	{
		if (isOpen())
		{
			return VIGEM_ERROR_NONE;
		}

		auto guard = lock();
		client = vigem_alloc();
		const VIGEM_ERROR result = vigem_connect(client);

		if (!VIGEM_SUCCESS(result))
		{
			vigem_free(client);
			client = nullptr;
		}

		return result;
	}

	bool Driver::isOpen() const
	{
		return client != nullptr;
	}

	void Driver::close()
	{
		if (isOpen())
		{
			auto guard = lock();
			vigem_disconnect(client);
			vigem_free(client);
			client = nullptr;
		}
	}

	Driver& Driver::operator=(Driver&& rhs) noexcept
	{
		close();
		client = rhs.client;
		rhs.client = nullptr;
		return *this;
	}
}
