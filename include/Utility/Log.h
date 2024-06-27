#pragma once
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

// Forward declarations for Windows console functions
extern "C"
{
    __declspec(dllimport) void* __stdcall GetStdHandle(int);
    __declspec(dllimport) int __stdcall SetConsoleTextAttribute(void*, unsigned short);
}

// Console text color constants
const unsigned short FOREGROUND_GREEN     = 0x2;
const unsigned short FOREGROUND_BLUE      = 0x1;
const unsigned short FOREGROUND_RED       = 0x4;
const unsigned short FOREGROUND_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
const unsigned short FOREGROUND_INTENSITY = 0x8;
const unsigned short FOREGROUND_WHITE     = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

const unsigned short LOG_GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const unsigned short LOG_BLUE  = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
const unsigned short LOG_YELLOW = FOREGROUND_YELLOW | FOREGROUND_INTENSITY;
const unsigned short LOG_RED   = FOREGROUND_RED | FOREGROUND_INTENSITY;

inline std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%X"); // %X is the locale's time representation (H:M:S)
    return ss.str();
}

#define Log(x) do { \
    void* _log_hConsole = GetStdHandle(-11); \
    SetConsoleTextAttribute(_log_hConsole, FOREGROUND_WHITE); \
    std::cout << "[" << getCurrentTimeString() << "] LOG:: " << x << std::endl; \
    SetConsoleTextAttribute(_log_hConsole, FOREGROUND_WHITE); \
} while (0)

#define LogColor(x, color) do { \
    void* _logColor_hConsole = GetStdHandle(-11); \
    SetConsoleTextAttribute(_logColor_hConsole, color); \
    std::cout << "[" << getCurrentTimeString() << "] LOG:: " << x << std::endl; \
    SetConsoleTextAttribute(_logColor_hConsole, FOREGROUND_WHITE); \
} while (0)


/* LINUX VERSION
// ANSI escape codes for console text colors
inline const char* ANSI_RESET = "\033[0m";
inline const char* LOG_GREEN = "\033[32m";
inline const char* LOG_BLUE = "\033[34m";
inline const char* LOG_YELLOW = "\033[33m";
inline const char* LOG_RED = "\033[31m";
inline const char* ANSI_WHITE = "\033[37m";
inline const char* ANSI_INTENSITY = "\033[1m";

#define Log(x) do { \
    std::cout << ANSI_WHITE << "[" << getCurrentTimeString() << "] LOG:: " << x << ANSI_RESET << std::endl; \
} while (0)

#define LogColor(x, color) do { \
    std::cout << color << "[" << getCurrentTimeString() << "] LOG:: " << x << ANSI_RESET << std::endl; \
} while (0)
*/


// BASIC VERSION
//#define Log(x) std::cout << "LOG:: "<< x << std::endl