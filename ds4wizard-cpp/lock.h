#pragma once

#include <mutex>

#define MAKE_GUARD(M) std::lock_guard<std::recursive_mutex> M ## _guard(M)

// Create a std::lock_guard for a symbol that has an associated *_lock
#define LOCK(NAME) MAKE_GUARD(NAME ## _lock)
