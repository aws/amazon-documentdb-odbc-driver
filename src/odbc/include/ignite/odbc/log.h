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
#include <sstream>
#include <string>
#include <memory>

#include "ignite/odbc/common/common.h"
#include "ignite/odbc/common/concurrent.h"
#include "ignite/odbc/log_level.h"

// Todo: implement log file using user-provided log path
// https://bitquill.atlassian.net/browse/AD-697

// todo remove pre-fix when making PR ready for review
#define WRITE_MSG(param, logLevel)                            \
  {                                                           \
    std::shared_ptr< ignite::odbc::Logger > p =               \
        ignite::odbc::Logger::getLoggerInstance();            \
    if (p->IsEnabled() && p->getLogLevel() <= logLevel) {     \
      ignite::odbc::LogStream lstream(p.get());               \
      std::string msg_prefix;                                 \
      switch (logLevel) {                                     \
        case ignite::odbc::LogLevel::Type::DEBUG_LEVEL:       \
          msg_prefix = "DEBUG MSG: ";                         \
          break;                                              \
        case ignite::odbc::LogLevel::Type::INFO_LEVEL:        \
          msg_prefix = "INFO MSG: ";                          \
          break;                                              \
        case ignite::odbc::LogLevel::Type::ERROR_LEVEL:       \
          msg_prefix = "ERROR MSG: ";                         \
          break;                                              \
        default:                                              \
          msg_prefix = "wrong level passed!!!!!!!";           \
      }                                                       \
      lstream << msg_prefix << __FUNCTION__ << ": " << param; \
    }                                                         \
    static_assert(true, "");                                  \
  }

// TODO replace and remove LOG_MSG
// https://bitquill.atlassian.net/browse/AD-703
// @Deprecated
#define LOG_MSG(param) {                                                \
  std::shared_ptr< ignite::odbc::Logger > p =                           \
        ignite::odbc::Logger::getLoggerInstance();                      \
  if (p->IsEnabled()) {                                                 \
    ignite::odbc::LogStream lstream(p.get());                           \
    lstream << __FUNCTION__ << ": " << param;                           \
  }                                                                     \
  static_assert(true, ""); }
// \ is used as line continuation here.
// to debug the LOG_MSG macro functions, need to navigate to a place where it is used 
// and use solar lint to check and make sure it doesn't have red squiggles 

// todo remove extra "xxx msg" in front -AL- // the purpose of adding the 
// "DEBUG MSG:" is for debugging my logging implementation.
#define LOG_DEBUG_MSG(param) \
  WRITE_MSG(param, ignite::odbc::LogLevel::Type::DEBUG_LEVEL)

// todo remove extra "xxx msg" in front -AL-
#define LOG_INFO_MSG(param) \
  WRITE_MSG(param, ignite::odbc::LogLevel::Type::INFO_LEVEL)

// todo remove extra "xxx msg" in front -AL-
#define LOG_ERROR_MSG(param) \
  WRITE_MSG(param, ignite::odbc::LogLevel::Type::ERROR_LEVEL)

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
  bool operator()();

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
   * Destructor. // -AL- made public so shared_ptr can remove the object when program exits
   */
  ~Logger();

  /**
   * Set the logger's set log level.
   */
  void setLogLevel(LogLevel::Type level);

  /**
   * Set the logger's set log path.
   * Once a log path is set, it cannot be changed
   */
  void setLogPath(std::string path);

  /**
   * Get singleton instance of Logger.
   * If there is no instance, create new instance.
   * @return Logger instance.
   */
  static std::shared_ptr<Logger> getLoggerInstance() {
      // -AL- Todo make jira ticket for adding locks for logger instance (or can do it myself if it is easy)
    if (!_logger)
        _logger = std::shared_ptr< Logger >(new Logger());

    return _logger;
  }

  /**
   * Get the logger's set log level.
   * @return logLevel.
   */
  LogLevel::Type getLogLevel();

  /**
   * Get the logger's set log path.
   * @return logPath.
   */
  std::string getLogPath();

  /**
   * Checks if logging is enabled.
   * @return True, if logging is enabled.
   */
  bool IsEnabled() const;

  /**
   * Outputs the message to log file
   * @param message The message to write
   */
  void WriteMessage(std::string const& message);

 private:
  static std::shared_ptr< Logger > _logger;  //a singleton instance

  /**
   * Constructor.
   */
  Logger()
      : mutex(), stream(), logLevel(), logPath(){ 
    // no-op
  }

  IGNITE_NO_COPY_ASSIGNMENT(Logger);

  /** Mutex for writes synchronization. */
  odbc::common::concurrent::CriticalSection mutex;

  /** File stream. */
  std::ofstream stream;

  /** Log path */
  std::string logPath;

  /** Log Level */
  LogLevel::Type logLevel;
};

}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_LOG
