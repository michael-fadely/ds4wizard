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
		/**
		 * \brief Explicitly disallow copying.
		 */
		Driver(const Driver&) = delete;

		Driver() = default;
		Driver(Driver&& rhs) noexcept;
		~Driver();

		/**
		 * \brief Opens a handle to the ViGEm driver.
		 * \return \c VIGEM_ERROR_NONE on success.
		 */
		VIGEM_ERROR open();

		/**
		 * \brief Indicates the opened state of the ViGEm driver handle.
		 * \return \c true if the handle is open.
		 */
		bool isOpen() const;

		/**
		 * \brief If open, closes the handle to the ViGEm driver.
		 */
		void close();

		/**
		 * \brief Explicitly disallow copying.
		 */
		Driver& operator=(const Driver&) = delete;
		Driver& operator=(Driver&& rhs) noexcept;
	};
}
