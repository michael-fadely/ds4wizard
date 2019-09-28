#include "pch.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include "Logger.h"

using namespace std::chrono;

LineLoggedEventArgs::LineLoggedEventArgs(LogLevel level, std::string line)
	: time(system_clock::now()),
	  level(level),
	  line(std::move(line))
{
}

void Logger::writeLine(LogLevel level, const std::string& line)
{
	const auto now = system_clock::now();
	std::time_t t = system_clock::to_time_t(now);

	std::stringstream message;
	message << std::put_time(std::localtime(&t), "%F %T") << " [" << level._to_string() << "] " << line;

	qDebug() << message.str().c_str();
	onLineLogged(level, line);
}

void Logger::writeLine(LogLevel level, const std::string& context, const std::string& line)
{
	std::stringstream message;
	message << "[" << context << "] " << line;

	writeLine(level, message.str());
}

void Logger::onLineLogged(LogLevel level, const std::string& line)
{
	LOCK(sync);

	auto args = std::make_shared<LineLoggedEventArgs>(level, line);
	lineLogged.invoke(nullptr, args);
}
