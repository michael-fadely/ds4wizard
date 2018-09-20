#pragma once

#include <deque>
#include <functional>

// TODO: senderless version
// TODO: rename

template <typename sender_t, typename... args_t>
class EventHandler
{
public:
	using callback_t = std::function<void(sender_t* sender, args_t... args)>;

private:
	std::deque<callback_t> callbacks;

public:
	EventHandler& operator+=(callback_t callback)
	{
		callbacks.emplace_back(std::move(callback));
		return *this;
	}

	EventHandler& operator-=(callback_t callback)
	{
		auto it = callbacks.find(callback);

		if (it != callbacks.end())
		{
			callbacks.erase(it);
		}

		return *this;
	}

	void invoke(sender_t* sender, args_t... args) const
	{
		for (auto& callback : callbacks)
		{
			callback(sender, args...);
		}
	}
};
