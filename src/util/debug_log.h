#ifndef KINDLE_UTIL_DEBUG_LOG_H
#define KINDLE_UTIL_DEBUG_LOG_H

#include <iostream>
#include <string>

/**
 * Utility for formatting and emitting debug traces.
 */
class DebugLog {
public:
    /**
     * Formats a structured debug message with a subsystem tag.
     *
     * @param tag Subsystem tag name.
     * @param message Debug message text.
     * @return Formatted debug log string.
     */
    static std::string format(const std::string& tag, const std::string& message) {
        return "[DEBUG][" + tag + "] " + message;
    }
};

#ifdef DEBUG
#define DEBUG_LOG(tag, message)                                                \
    do {                                                                       \
        std::clog << DebugLog::format((tag), (message)) << '\n';               \
    } while (false)
#else
#define DEBUG_LOG(tag, message) do {} while (false)
#endif

#endif
