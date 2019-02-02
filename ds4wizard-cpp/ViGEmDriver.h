#pragma once

#include <ViGEm/Client.h>

namespace vigem
{
	class XInputTarget;

	class Driver
	{
		friend class XInputTarget;
		PVIGEM_CLIENT client = nullptr;

	public:
		Driver() = default;
		Driver(const Driver&) = delete;
		Driver(Driver&& rhs) noexcept;
		~Driver();

		VIGEM_ERROR open();
		bool isOpen() const;
		void close();

		Driver& operator=(const Driver&) = delete;
		Driver& operator=(Driver&& rhs) noexcept;
	};
}
