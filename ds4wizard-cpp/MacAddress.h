#pragma once

#include <array>

constexpr size_t macAddressLength = 6;
using MacAddress = std::array<uint8_t, macAddressLength>;
