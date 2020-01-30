#pragma once

#include <array>
#include <mutex>

#include "Ds4Color.h"

/**
 * \brief Class for managing automatic light color selection for a \c Ds4Device
 * \sa Ds4Device
 */
class Ds4AutoLightColor
{
	struct Pair
	{
		const Ds4Color color;
		ptrdiff_t references = 0;

		explicit Pair(const Ds4Color& color);
	};

	inline static std::recursive_mutex lock;

	static std::array<Pair, 17> colors;

public:
	/**
	 * \brief Get an automatically assigned color.
	 * \param index Assigned color index.
	 * \return Automatically assigned color.
	 */
	static Ds4Color getColor(ptrdiff_t& index);

	/**
	 * \brief Release an assigned color by index.
	 * \param index The index to release.
	 */
	static void releaseColor(ptrdiff_t index);
};
