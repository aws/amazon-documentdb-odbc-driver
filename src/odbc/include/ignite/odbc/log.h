/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _IGNITE_ODBC_LOG
#define _IGNITE_ODBC_LOG

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "ignite/common/utils.h"
#include "ignite/odbc/common/common.h"
#include "ignite/odbc/common/concurrent.h"
#include "ignite/odbc/log_level.h"

using ignite::odbc::common::concurrent::CriticalSection;

// Todo: implement log file using user-provided log path
// https://bitquill.atlassian.net/browse/AD-697

#if defined(_WIN32)
#define DEFAULT_LOG_PATH \
  std::string(getenv("TEMP")) // Windows
// In Windows, DEFAULT_LOG_PATH is used to pre-populate the log path field in 
// the DSN Configuration Window. 
#else
#define DEFAULT_LOG_PATH "."  // unix
// In the ODBC driver, DEFAULT_LOG_PATH is only used for testing in unix platforms.
// It is expected for the user to enter a valid log path during DSN connection in unix platforms. 
#endif

#define WRITE_LOG_MSG(param, logLevel) \
  WRITE_MSG_TO_STREAM(param, logLevel, (std::ostream*)nullptr)

#define WRITE_MSG_TO_STREAM(param, logLevel, logStream)                       \
  {                                                                           \
    std::shared_ptr< ignite::odbc::Logger > p =                               \
        ignite::odbc::Logger::GetLoggerInstance();                            \
    if (p->GetLogLevel() <= logLevel && (p->IsEnabled() || p->EnableLog())) { \
      std::ostream* prevStream = p.get()->GetLogStream();                     \
      if (logStream != nullptr) {                                             \
        /* Override the stream temporarily */                                 \
        p.get()->SetLogStream(logStream);                                     \
      }                                                                       \
      std::unique_ptr< ignite::odbc::LogStream > lstream(new ignite::odbc::LogStream(p.get()));    \
      std::string msg_prefix;                                                 \
      switch (logLevel) {                                                     \
        case ignite::odbc::LogLevel::Type::DEBUG_LEVEL:                       \
          msg_prefix = "DEBUG MSG: ";                                         \
          break;                                                              \
        case ignite::odbc::LogLevel::Type::INFO_LEVEL:                        \
          msg_prefix = "INFO MSG: ";                                          \
          break;                                                              \
        case ignite::odbc::LogLevel::Type::ERROR_LEVEL:                       \
          msg_prefix = "ERROR MSG: ";                                         \
          break;                                                              \
        default:                                                              \
          msg_prefix = "";                                                    \
      }                                                                       \
      char tStr[1000];                                                        \
      time_t curTime = time(NULL);                                            \
      struct tm* locTime = localtime(&curTime);                               \
      strftime(tStr, 1000, "%T %x ", locTime);                                \
      /* Write the formatted message to the stream */                         \
      *lstream << "TID: " << std::this_thread::get_id() << " "                \
               << tStr << msg_prefix << __FUNCTION__ << ": " << param;        \
      /* This will trigger the write to stream */                             \
      lstream = nullptr;                                                      \
      if (logStream != nullptr) {                                             \
        /* Restore the stream if it was set */                                \
        p.get()->SetLogStream(prevStream);                                    \
      }                                                                       \
    }                                                                         \
  }

// TODO replace and remove LOG_MSG
// https://bitquill.atlassian.net/browse/AD-703
// @Deprecated
#define LOG_MSG(param)                                                                                   \
  LOG_INFO_MSG(param)

// Debug messages are messages that are useful for debugging
#define LOG_DEBUG_MSG(param) \
  WRITE_LOG_MSG(param, ignite::odbc::LogLevel::Type::DEBUG_LEVEL)

#define LOG_DEBUG_MSG_TO_STREAM(param, logStream) \
  WRITE_MSG_TO_STREAM(param, ignite::odbc::LogLevel::Type::DEBUG_LEVEL, logStream)

// Info messages are messages that document the application flow
#define LOG_INFO_MSG(param) \
  WRITE_LOG_MSG(param, ignite::odbc::LogLevel::Type::INFO_LEVEL)

#define LOG_INFO_MSG_TO_STREAM(param, logStream) \
  WRITE_MSG_TO_STREAM(param, ignite::odbc::LogLevel::Type::INFO_LEVEL, logStream)

// Error messages display errors.
#define LOG_ERROR_MSG(param) \
  WRITE_LOG_MSG(param, ignite::odbc::LogLevel::Type::ERROR_LEVEL)

#define LOG_ERROR_MSG_TO_STREAM(param, logStream) \
  WRITE_MSG_TO_STREAM(param, ignite::odbc::LogLevel::Type::ERROR_LEVEL, logStream)

namespace ignite {
namespace odbc {
/* Forward declaration */
class Logger;

/**
 * Helper object providing stream operations for single log line.
 * Writes resulting string to Logger object upon destruction.
 */
class LogStream : public std::basic_ostream< char > {
 public:
  /**
   * Constructor.
   * @param parent pointer to Logger.
   */
  LogStream(Logger* parent);

  /**
   * Conversion operator helpful to determine if log is enabled
   * @return True if logger is enabled
   */
  bool operator()() const;

  /**
   * Destructor.
   */
  virtual ~LogStream();

 private:
  IGNITE_NO_COPY_ASSIGNMENT(LogStream);

  /** String buffer. */
  std::basic_stringbuf< char > strbuf;

  /** Parent logger object */
  Logger* logger;
};

/**
 * Logging facility.
 */
class Logger {
 public:
  /**
   * Destructor.
   */
  ~Logger() = default;

  /**
   * Set the logger's set log level.
   */
  void SetLogLevel(LogLevel::Type level);

  /**
   * Set the logger's set log path.
   * Once a log path is set, it cannot be changed
   */
  void SetLogPath(const std::string& path);

  /** 
   * Sets the stream to use for logging.
   */
  void SetLogStream(std::ostream* stream);

  /**
   * Gets the current stream to use for logging.
   */
  std::ostream* GetLogStream() {
    return stream;
  }

  /**
   * Get singleton instance of Logger.
   * If there is no instance, create new instance.
   * @return Logger instance.
   */
  static std::shared_ptr< Logger > GetLoggerInstance() {
    // TODO add locks to prevent 2 or more instances being
    // created at once
    // [AD-716](https://bitquill.atlassian.net/browse/AD-716)

    if (!_logger)
      _logger = std::shared_ptr< Logger >(new Logger());

    return _logger;
  }

  /**
   * Get the logger's set log level.
   * @return logLevel.
   */
  LogLevel::Type GetLogLevel() const;

  /**
   * Get the logger's set log path.
   * @return logPath.
   */
  std::string& GetLogPath();

  /**
   * Checks if file stream is opened.
   * @return True, if file stream is opened.
   */
  bool IsFileStremOpen() const;

  /**
   * Checks if logging is enabled.
   * @return True, if logging is enabled.
   */
  bool IsEnabled() const;

  /**
   * Enable logging if log path is set and log level is not off.
   * @return True, if logging is enabled. False is logging cannot be enabled
   */
  bool EnableLog();

  /**
   * Outputs the message to log file
   * @param message The message to write
   */
  void WriteMessage(std::string const& message);

 private:
  static std::shared_ptr< Logger > _logger;  // a singleton instance

  /**
   * Constructor.
   */
  Logger() = default;

  /**
   * Creates the log file name based on date and time
   * Log file format: docdb_odbc_YYYYMMDD_HHMMSS.log
   */
  std::string CreateFileName() const;

  IGNITE_NO_COPY_ASSIGNMENT(Logger);

  /** Mutex for writes synchronization. */
  CriticalSection mutex;

  /** File stream. */
  std::ofstream fileStream;

  /** Reference to logging stream */
  std::ostream* stream = nullptr;

  /** Log path */
  std::string logPath;

  /** Log Level */
  LogLevel::Type logLevel = LogLevel::Type::OFF;
};
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_LOG
