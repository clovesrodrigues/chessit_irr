#include "Core/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace chessit {

std::mutex Logger::mutex_;

void Logger::Info(const std::string& message) { Write(LogLevel::Info, message); }
void Logger::Warning(const std::string& message) { Write(LogLevel::Warning, message); }
void Logger::Error(const std::string& message) { Write(LogLevel::Error, message); }

void Logger::Write(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);

    const char* label = "INFO";
    if (level == LogLevel::Warning) label = "WARN";
    if (level == LogLevel::Error) label = "ERROR";

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    out << '[' << std::put_time(&tm, "%H:%M:%S") << "] [" << label << "] " << message << std::endl;
}

} // namespace chessit
