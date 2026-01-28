#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>

// Inline function to prevent multiple definition errors
inline void LOG(std::string msg) {
    // Print to console
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::cout << std::put_time(std::localtime(&now_time), "[%Y-%m-%d %H:%M:%S -") << msg << std::endl;

    // Append to exec.log
    std::ofstream errorLog("exec.log", std::ios::app);
    if (errorLog.is_open()) {
        errorLog << std::put_time(std::localtime(&now_time), "[%Y-%m-%d %H:%M:%S -") << msg << std::endl;
        errorLog.close();
    } else {
        std::cerr << "Failed to open error.log" << std::endl;
    }
}

// Macro for simplified logging
//#define LOG(msg) logMessage

#endif // LOG_H
