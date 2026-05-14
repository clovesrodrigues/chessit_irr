#pragma once

#include <mutex>
#include <string>

namespace chessit {

enum class LogLevel { Info, Warning, Error };

class Logger {
public:
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);

private:
    static void Write(LogLevel level, const std::string& message);
    static std::mutex mutex_;
};

} // namespace chessit
