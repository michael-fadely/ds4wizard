#pragma once

#include <deque>
#include <functional>

class EventArgs
{
public:
	static inline EventArgs const* empty = nullptr;
};

template <typename sender_t, typename args_t>
class EventHandler
{
public:
	using callback_t = std::function<void(sender_t* sender, std::shared_ptr<args_t> args)>;

private:
	std::deque<callback_t> callbacks;

public:
	EventHandler& operator+=(callback_t callback);
	EventHandler& operator-=(callback_t callback);

	void invoke(sender_t* sender, std::shared_ptr<args_t> args) const;
	void invoke(sender_t* sender) const;
};

template <typename sender_t, typename args_t>
EventHandler<sender_t, args_t>& EventHandler<sender_t, args_t>::operator+=(callback_t callback)
{
	callbacks.emplace_back(std::move(callback));
	return *this;
}

template <typename sender_t, typename args_t>
EventHandler<sender_t, args_t>& EventHandler<sender_t, args_t>::operator-=(callback_t callback)
{
	auto it = callbacks.find(callback);

	if (it != callbacks.end())
	{
		callbacks.erase(it);
	}

	return *this;
}

template <typename sender_t, typename args_t>
void EventHandler<sender_t, args_t>::invoke(sender_t* sender, std::shared_ptr<args_t> args) const
{
	for (auto& callback : callbacks)
	{
		callback(sender, args);
	}
}

template <typename sender_t, typename args_t>
void EventHandler<sender_t, args_t>::invoke(sender_t* sender) const
{
	invoke(sender, nullptr);
}
