#pragma once

#include <deque>
#include <functional>

class EventHandlerArgs {};

template <typename args_t>
class EventHandler
{
public:
	using callback_t = std::function<void(void* sender, args_t* args)>;

private:
	std::deque<callback_t> callbacks;

public:
	EventHandler& operator+=(callback_t callback);
	EventHandler& operator-=(callback_t callback);

	void invoke(void* sender, args_t* args);
};

template <typename args_t>
EventHandler<args_t>& EventHandler<args_t>::operator+=(callback_t callback)
{
	callbacks.emplace_back(std::move(callback));
	return *this;
}

template <typename args_t>
EventHandler<args_t>& EventHandler<args_t>::operator-=(callback_t callback)
{
	auto it = callbacks.find(callback);

	if (it != callbacks.end())
	{
		callbacks.erase(it);
	}

	return *this;
}

template <typename args_t>
void EventHandler<args_t>::invoke(void* sender, args_t* args)
{
	for (auto& callback : callbacks)
	{
		callback(sender, args);
	}
}
