#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <mutex>

/**
 * \brief An event token to be used with \c Event.
 * When the \c EventToken is destroyed, it is unregistered from the event automatically.
 */
using EventToken = std::shared_ptr<void>;

/**
 * \brief An object which notifies listeners of an event.
 * \tparam sender_t The type of the sender.
 * \tparam args_t Arguments that will be passed to the listeners.
 */
template <typename sender_t, typename... args_t>
class Event
{
public:
	using callback_t = std::function<void(sender_t* sender, args_t... args)>;

private:
	using weak_callback = std::weak_ptr<callback_t>;

	std::deque<weak_callback> callbacks;
	std::recursive_mutex mutex;

public:
	/**
	 * \brief Registers a listener callback function with the event.
	 * \param callback Function to execute.
	 * \return An \c EventToken to maintain connection to the event.
	 */
	[[nodiscard]] EventToken add(callback_t callback)
	{
		std::lock_guard lock(mutex);

		auto token = std::make_shared<callback_t>(std::move(callback));
		callbacks.push_back(token);
		return token;
	}

	/**
	 * \brief Unregisters a listener from the event.
	 * \param token The \c EventToken to unregister from the event.
	 */
	void remove(EventToken token)
	{
		std::lock_guard lock(mutex);

		callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), [token](auto ptr) -> bool
		{
			return ptr.lock() == token;
		}));
	}

	/**
	 * \brief Raises the event and notifies all listeners.
	 * \param sender The object invoking the event, or \c nullptr.
	 * \param args The arguments to pass to the listeners.
	 */
	void invoke(sender_t* sender, args_t... args)
	{
		std::lock_guard lock(mutex);

		auto predicate = [](weak_callback t) -> bool
		{
			return t.expired();
		};

		callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), predicate), callbacks.end());

		// Copied so that callbacks can register new callbacks or unregister callbacks.
		auto callbacksCopy = callbacks;

		for (auto weak : callbacksCopy)
		{
			if (auto shared = weak.lock())
			{
				(*shared)(sender, args...);
			}
		}
	}
};
