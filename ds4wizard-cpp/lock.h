#pragma once

#include <mutex>

#define lock(NAME) std::lock_guard<std::mutex> NAME ## _guard(NAME ## _lock)
