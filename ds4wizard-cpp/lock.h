#pragma once

#include <mutex>

#define lock(NAME) std::lock_guard<std::recursive_mutex> NAME ## _guard(NAME ## _lock)
