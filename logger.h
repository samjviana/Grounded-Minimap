//
// Created by PC-SAMUEL on 20/12/2024.
//
// A logging class for file and console logging with multi-threading support.
//

#ifndef GROUNDED_MINIMAP_LOGGER_H
#define GROUNDED_MINIMAP_LOGGER_H

#include <string>
#include <mutex>

namespace grounded_minimap {

/**
 * Provides methods for logging messages to a file or console.\n
 * Supports multi-threaded applications.
 */
class Logger {
public:
    /**
     * Initializes the logger with the specified file path.
     * Closes any previously opened log file.
     *
     * @param logFilePath The path to the log file.
     * @note If the file cannot be opened, an error message will be printed to `stderr`.
     */
    static void Initialize(const std::string& logFilePath);

    /**
     * Enables debug mode, printing log messages not only to the file but also the console.
     * Allocates a console for output if one is not already present.
     *
     * @note Debug mode does not disable file logging.
     */
    static void InitializeDebug();

    /**
     * Logs an informational message.
     *
     * @param message The message to log.
     */
    static void Info(const std::string& message);

    /**
     * Logs a warning message.
     *
     * @param message The message to log.
     */
    static void Warning(const std::string& message);

    /**
     * Logs an error message.
     *
     * @param message The message to log.
     */
    static void Error(const std::string& message);

    /**
     * Shuts down the logger by closing the log file and releasing resources.
     *
     * @note If debug mode is active, the console will also be freed.
     */
    static void Shutdown();

private:
    /**
     * Logs a message with the given severity level.
     * Includes a timestamp and formats the message before logging.
     *
     * @param severity The severity level (e.g.: INFO, WARNING, ERROR).
     * @param message The message to log.
     */
    static void Log(const std::string& severity, const std::string& message);

    /**
     * Sets up a console for output when debug mode is active.
     * Redirects standard streams (stdout, stderr, stdin) to the console.
     *
     * @note If the console allocation fails, an error message will be printed to stderr.
     */
    static void InitializeConsole();

    static std::ofstream logFile_;  // The log file stream.
    static std::mutex logMutex_;    // Mutex for thread safety.
    static bool debugMode_;         // Indicates whether debug mode is active.
};

} // grounded_minimap

#endif //GROUNDED_MINIMAP_LOGGER_H
