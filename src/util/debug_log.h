#ifndef KINDLE_UTIL_DEBUG_LOG_H
#define KINDLE_UTIL_DEBUG_LOG_H

#include <iostream>
#include <string>

/**
 * @brief Utility for formatting structured debug traces.
 */
class DebugLog {
public:
    /**
     * @brief Formats a debug message with a subsystem tag.
     * @param tag Subsystem tag identifying the message source.
     * @param message Debug message text.
     * @return A string in the form "[DEBUG][tag] message".
     */
    static std::string format(const std::string& tag, const std::string& message) {
        return "[DEBUG][" + tag + "] " + message;
    }
};

/**
 * @brief Emits a tagged debug message when debug logging is enabled.
 * @param tag Subsystem tag identifying the message source.
 * @param message Debug message text.
 *
 * When DEBUG is not defined, this macro expands to a no-op and does not emit
 * anything to the process log.
 */
#ifdef DEBUG
#define DEBUG_LOG(tag, message)                                                \
    do {                                                                       \
        std::clog << DebugLog::format((tag), (message)) << '\n';               \
    } while (false)
#else
#define DEBUG_LOG(tag, message) do {} while (false)
#endif

#endif
