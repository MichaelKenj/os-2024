#pragma once
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <sstream>
#include <string>
#include <iomanip>

enum logLevel
{
    INFO,
    WARNING,
    ERROR,
    FATAL,
    DEBUG
};

#define LOG_INFO(message) global_logger().Log(logLevel::INFO, __LINE__, __func__, message)
#define LOG_ERROR(message) global_logger().Log(logLevel::ERROR, __LINE__, __func__, message)
#define LOG_WARNING(message) global_logger().Log(logLevel::WARNING, __LINE__, __func__, message)
#define LOG_FATAL(message) global_logger().Log(logLevel::FATAL, __LINE__, __func__, message)
#define LOG_DEBUG(message) global_logger().Log(logLevel::DEBUG, __LINE__, __func__, message)

class Logger
{
private:
    std::ostream& _out;
    bool _colored;
    std::ofstream _fileStream;  
public:
    explicit Logger(std::ostream& out, bool colored = false)
    : _out(out)
    , _colored(colored)
    {
    	_fileStream.open("log.txt", std::ios::out | std::ios::app);
        if (!_fileStream.is_open())
        {
            std::cerr << "Failed to open log file!" << std::endl;
        }
    }

    ~Logger()
    {
    	_fileStream.close();
    }

    static std::string LogLevelStr(logLevel level)
    {
        switch (level)
        {
            case INFO: return "INFO";
            case WARNING: return "WARNING";
            case ERROR: return "ERROR";
            case FATAL: return "FATAL";
            case DEBUG: return "DEBUG";
            default: return "UNKNOWN";
        }
    }
    void Log(logLevel level, int LINE, const std::string& func, const std::string& message)
    {
        std::time_t now = std::time(nullptr);
        std::tm timeInfo = *std::localtime(&now);

        // Log to console
        if (_colored)
        {
            _out << getColor(level) << "[" << std::put_time(&timeInfo, "%T") << "] "
                 << "[" << func << ":" << LINE << "]"
                 << " [" << LogLevelStr(level) << "] " << message << getColor(logLevel::INFO)
                 << std::endl;
        }
        else
        {
            _out << "[" << std::put_time(&timeInfo, "%T") << "] "
                 << "[" << func << ":" << LINE << "]"
                 << " [" << LogLevelStr(level) << "] " << message << std::endl;
        }

        // Log to file
        if (_fileStream.is_open())
        {
            _fileStream << "[" << std::put_time(&timeInfo, "%T") << "] "
                        << "[" << func << ":" << LINE << "]"
                        << " [" << LogLevelStr(level) << "] " << message << std::endl;
        }

        if (level == logLevel::FATAL)
        {
            _out.flush();
            std::exit(EXIT_FAILURE);
        }
    }
    static const char* getColor(logLevel level)
    {
        switch(level)
        {
            case ERROR:
                return "\033[31m";
            case WARNING:
                return "\033[33m";
            case DEBUG:
                return "\033[34m";
            default:
                return "\u001b[0m";
        }
    }
};

inline Logger& global_logger()
{
	static Logger log{std::cout};
	return log;
}
