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

#ifndef _DOCUMENTDB_ODBC_LOG
#define _DOCUMENTDB_ODBC_LOG

#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "documentdb/odbc/common/common.h"
#include "documentdb/odbc/common/concurrent.h"
#include "documentdb/odbc/log_level.h"

using documentdb::odbc::common::concurrent::CriticalSection;

#define REDACTED_STRING "****"

#define DEFAULT_LOG_PATH documentdb::odbc::Logger::GetDefaultLogPath()

#define WRITE_LOG_MSG(param, logLevel, msgPrefix) \
  WRITE_MSG_TO_STREAM(param, logLevel, msgPrefix, (std::ostream*)nullptr)

#define WRITE_MSG_TO_STREAM(param, logLevel, msgPrefix, logStream)            \
  {                                                                           \
    std::shared_ptr< documentdb::odbc::Logger > p =                           \
        documentdb::odbc::Logger::GetLoggerInstance();                        \
    if (p->GetLogLevel() <= logLevel && (p->IsEnabled() || p->EnableLog())) { \
      std::ostream* prevStream = p.get()->GetLogStream();                     \
      if (logStream) {                                                        \
        /* Override the stream temporarily */                                 \
        p.get()->SetLogStream(logStream);                                     \
      }                                                                       \
      std::unique_ptr< documentdb::odbc::LogStream > lstream(                 \
          new documentdb::odbc::LogStream(p.get()));                          \
      auto now = std::chrono::system_clock::now();                            \
      auto now_time_t = std::chrono::system_clock::to_time_t(now);            \
      auto locTime = std::localtime(&now_time_t);                             \
      std::ostringstream fmt_time;                                            \
      fmt_time << std::put_time(locTime, "%T %x ");                           \
      /* Write the formatted message to the stream */                         \
      *lstream << "TID: " << std::this_thread::get_id() << " "                \
               << fmt_time.str() << msgPrefix << __FUNCTION__ << ": "         \
               << param;                                                      \
      /* This will trigger the write to stream */                             \
      lstream = nullptr;                                                      \
      if (logStream) { /* Restore the stream if it was set */                 \
        p.get()->SetLogStream(prevStream);                                    \
      }                                                                       \
    }                                                                         \
  }

// TODO replace and remove LOG_MSG
// https://github.com/aws/amazon-documentdb-odbc-driver/issues/174
// @Deprecated
#define LOG_MSG(param) LOG_INFO_MSG(param)

// Debug messages are messages that are useful for debugging
#define LOG_DEBUG_MSG(param)                                          \
  WRITE_LOG_MSG(param, documentdb::odbc::LogLevel::Type::DEBUG_LEVEL, \
                "DEBUG MSG: ")

#define LOG_DEBUG_MSG_TO_STREAM(param, logStream)                           \
  WRITE_MSG_TO_STREAM(param, documentdb::odbc::LogLevel::Type::DEBUG_LEVEL, \
                      "DEBUG MSG: ", logStream)

// Info messages are messages that document the application flow
#define LOG_INFO_MSG(param)                                          \
  WRITE_LOG_MSG(param, documentdb::odbc::LogLevel::Type::INFO_LEVEL, \
                "INFO MSG: ")

#define LOG_INFO_MSG_TO_STREAM(param, logStream)                           \
  WRITE_MSG_TO_STREAM(param, documentdb::odbc::LogLevel::Type::INFO_LEVEL, \
                      "INFO MSG: ", logStream)

// Error messages display errors.
#define LOG_ERROR_MSG(param)                                          \
  WRITE_LOG_MSG(param, documentdb::odbc::LogLevel::Type::ERROR_LEVEL, \
                "ERROR MSG: ")

#define LOG_ERROR_MSG_TO_STREAM(param, logStream)                           \
  WRITE_MSG_TO_STREAM(param, documentdb::odbc::LogLevel::Type::ERROR_LEVEL, \
                      "ERROR MSG: ", logStream)

namespace documentdb {
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
  DOCUMENTDB_NO_COPY_ASSIGNMENT(LogStream);

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
   * Get default log path.
   * @return Logger default path.
   */
  static std::string GetDefaultLogPath();

  /**
   * Get singleton instance of Logger.
   * If there is no instance, create new instance.
   * @return Logger instance.
   */
  static std::shared_ptr< Logger > GetLoggerInstance() {
    // TODO add locks to prevent 2 or more instances being
    // created at once
    // [Add locks on getLoggerInstance](https://github.com/aws/amazon-documentdb-odbc-driver/issues/175)

    if (!logger_)
      logger_ = std::shared_ptr< Logger >(new Logger());

    return logger_;
  }

/**
 * Will redact the message if the log level is not DEBUG
 * 
 * @param message 
 * @return std::string 
 */
  static std::string RedactMessage(std::string const message) {
    return GetLoggerInstance()->GetLogLevel() == LogLevel::Type::DEBUG_LEVEL ? message
               : REDACTED_STRING;
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
  bool IsFileStreamOpen() const;

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
  static std::shared_ptr< Logger > logger_;  // a singleton instance

  /**
   * Constructor.
   */
  Logger() = default;

  /**
   * Creates the log file name based on date
   * Log file format: docdb_odbc_YYYYMMDD.log
   */
  std::string CreateFileName() const;

  DOCUMENTDB_NO_COPY_ASSIGNMENT(Logger);

  /** Mutex for writes synchronization. */
  CriticalSection mutex;

  /** File stream. */
  std::ofstream fileStream;

  /** Reference to logging stream */
  std::ostream* stream = nullptr;

  /** Log folder path */
  std::string logPath = DEFAULT_LOG_PATH;

  /** Log Level */
  LogLevel::Type logLevel = LogLevel::Type::ERROR_LEVEL;

  /** Log file name */
  std::string logFileName;

  /** Log file path */
  std::string logFilePath;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_LOG
