#ifndef __LOG_WRITER_H__
#define __LOG_WRITER_H__

#include <string>
#include <iostream>
//#include <stdexcept>

enum LogLevel {
    NONE = 0,
    DEBUG = 10,
    INFO = 20,
    PRINT = 25,
    WARNING = 30,
    THROW = 35,         //those below can throw 
    ERROR = 40,
    CRITICAL = 50,
    ASSERT_FAIL = 60
};
#define LOG(msg, level) do { std::cout << msg << std::endl; } while (0)
#define THROW_(msg, level)     LOG(msg, level)

#define THROW_ERROR(msg)      THROW_(msg, ERROR)
#define THROW_CRITICAL(msg)   THROW_(msg, CRITICAL)
#define LOG_PRINT(msg)          LOG(msg, PRINT)
#define LOG_INFO(msg)       LOG(msg, INFO)
#define LOG_ERROR(msg)       LOG(msg, ERROR)
#define LOG_WARNING(msg)    LOG(msg, WARNING)
#define LOG_DEBUG(msg)      LOG(msg, DEBUG)

#endif // __LOG_WRITER_H__

