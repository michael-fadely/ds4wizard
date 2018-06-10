#include "stdafx.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include "Logger.h"

using namespace std::chrono;

LineLoggedEventArgs::LineLoggedEventArgs(LogLevel level, std::string line)
	: Time(system_clock::now()),
	  Level(level),
	  Line(std::move(line))
{
}

void Logger::WriteLine(LogLevel level, const std::string& line)
{
	// TODO: Console.WriteLine(Resources.LoggerLineFormat, DateTime.Now.ToString(CultureInfo.InvariantCulture), level, line);

	const auto now = system_clock::now();
	std::time_t t = system_clock::to_time_t(now);

	std::stringstream message;
	message << std::put_time(std::localtime(&t), "%F %T") << " [" << level._to_string() << "] " << line;

	qDebug() << message.str().c_str();
	OnLineLogged(LineLoggedEventArgs(level, line));
}

void Logger::WriteLine(LogLevel level, const std::string& context, const std::string& line)
{
	std::stringstream message;
	message << "[" << context << "] " << line;

	WriteLine(level, message.str());
}

void Logger::OnLineLogged(const LineLoggedEventArgs& e)
{
	lock(sync);
	// TODO: LineLogged?.Invoke(null, e);
}
