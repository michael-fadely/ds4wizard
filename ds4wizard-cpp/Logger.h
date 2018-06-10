#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <enum.h>
#include "EventHandler.h"

BETTER_ENUM(LogLevel, int, info, warning, error);

/// <summary>
/// Event arguments for <see cref="Logger.LineLogged"/>.
/// </summary>
class LineLoggedEventArgs
{
public:
	/// <summary>
	/// The time that this line was submitted.
	/// </summary>
	const std::chrono::system_clock::time_point time;

	/// <summary>
	/// Error level.
	/// </summary>
	/// <seealso cref="LogLevel"/>
	const LogLevel level;

	/// <summary>
	/// The line of text submitted.
	/// </summary>
	const std::string line;

	LineLoggedEventArgs(LogLevel level, std::string line);
};

class Logger
{
	inline static std::recursive_mutex sync_lock;

public:
	static inline EventHandler<LineLoggedEventArgs> lineLogged;

	/// <summary>
	/// Submits a line to the logger and notifies all registered events.
	/// </summary>
	/// <param name="level">Error level.</param>
	/// <param name="line">Line of text to be submitted.</param>
	static void writeLine(LogLevel level, const std::string& line);

	/// <summary>
	/// Submits a line (with context) to the logger and notifies all registered events.
	/// The submitted line will be in the format "[context] line".
	/// </summary>
	/// <param name="level">Error level.</param>
	/// <param name="context">Context of this line.</param>
	/// <param name="line">Line of text to be submitted.</param>
	static void writeLine(LogLevel level, const std::string& context, const std::string& line);

private:
	static void onLineLogged(LogLevel level, const std::string& line);
};