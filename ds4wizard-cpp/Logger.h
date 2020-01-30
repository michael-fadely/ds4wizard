#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <enum.h>
#include "Event.h"

BETTER_ENUM(LogLevel, int, info, warning, error);

/**
 * \brief Event arguments for \c Logger.LineLogged.
 * \sa Logger.LineLogged
 */
class LineLoggedEventArgs
{
public:
	/**
	 * \brief The time that this line was submitted.
	 */
	const std::chrono::system_clock::time_point time;

	/**
	 * \brief Error level.
	 * \sa LogLevel
	 */
	const LogLevel level;

	/**
	 * \brief The line of text submitted.
	 */
	const std::string line;

	LineLoggedEventArgs(LogLevel level, std::string line);
};

class Logger
{
	inline static std::recursive_mutex lock;

public:
	static inline Event<void, std::shared_ptr<LineLoggedEventArgs>> lineLogged;

	/**
	 * \brief Submits a line to the logger and notifies all registered events.
	 * \param level Error level.
	 * \param line Line of text to be submitted.
	 */
	static void writeLine(LogLevel level, const std::string& line);

	/**
	 * \brief Submits a line (with context) to the logger and notifies all registered events.
	 * The submitted line will be in the format "[context] line".
	 * \param level Error level.
	 * \param context Context of this line.
	 * \param line Line of text to be submitted.
	 */
	static void writeLine(LogLevel level, const std::string& context, const std::string& line);

private:
	static void onLineLogged(LogLevel level, const std::string& line);
};
