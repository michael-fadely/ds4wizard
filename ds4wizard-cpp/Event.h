#pragma once

#include <deque>
#include <functional>
#include <memory>

using EventToken = std::shared_ptr<void>;

template <typename sender_t, typename... args_t>
class Event
{
public:
	using callback_t = std::function<void(sender_t* sender, args_t... args)>;

private:
	using weak_callback = std::weak_ptr<callback_t>;
	using shared_callback = std::shared_ptr<callback_t>;

	std::deque<weak_callback> callbacks;

public:
	[[nodiscard]] EventToken add(callback_t callback)
	{
		auto token = std::make_shared<callback_t>(std::move(callback));
		callbacks.push_back(token);
		return token;
	}

	void remove(EventToken token)
	{
		std::remove(callbacks.begin(), callbacks.end(), token);
	}

	void invoke(sender_t* sender, args_t... args)
	{
		auto predicate = [](weak_callback t) -> bool
		{
			return t.expired();
		};

		callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), predicate), callbacks.end());

		auto callbacks_ = callbacks;

		for (auto weak : callbacks_)
		{
			if (auto shared = weak.lock())
			{
				(*shared)(sender, args...);
			}
		}
	}
};
