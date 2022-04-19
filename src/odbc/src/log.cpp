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

#include "ignite/odbc/log.h"

#include <cstdlib>

#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/log_level.h"

using ignite::odbc::Logger;
using ignite::odbc::config::Configuration;

// _logger pointer will  initialized in first call to getLoggerInstance
std::shared_ptr< Logger > Logger::_logger;

namespace ignite {
namespace odbc {
LogStream::LogStream(Logger* parent)
    : std::basic_ostream< char >(0), strbuf(), logger(parent) {
  init(&strbuf);
}

bool LogStream::operator()() {
  return logger != 0;
}

LogStream::~LogStream() {
  if (logger) {
    logger->WriteMessage(strbuf.str());
  }
}

void Logger::setLogPath(std::string path) {
  if (logPath == path) {
    LOG_DEBUG_MSG(
        "WARNING: setLogPath is called again with the same path string. "
        "setLogPath should only be called once");
    return;
  }
  std::string oldLogPath = logPath;
  logPath = path;
  if (logLevel != LogLevel::Type::OFF && !logPath.empty()) {
    if (!IsEnabled()) {
      stream.open(logPath);
    } else {
      LOG_INFO_MSG(
          "reset log path: Log path is changed to " + logPath);
      stream.close();
      stream.open(logPath);
      LOG_INFO_MSG("Previously logged information is stored in log file " + oldLogPath);
    }
  }
}

void Logger::setLogLevel(LogLevel::Type level) {
  logLevel = level;
  if (!IsEnabled() && logLevel != LogLevel::Type::OFF && !logPath.empty()) {
    stream.open(logPath);
  } else if (IsEnabled() && logLevel == LogLevel::Type::OFF) {
    stream.close();
  }
}

Logger::~Logger() {
}

bool Logger::IsEnabled() const {
  return stream.is_open();
}

void Logger::WriteMessage(std::string const& message) {
  if (IsEnabled()) {
    common::concurrent::CsLockGuard guard(mutex);
    stream << message << std::endl;
  }
}

LogLevel::Type Logger::getLogLevel() {
  return logLevel;
}

std::string& Logger::getLogPath() {
  return logPath;
}

}  // namespace odbc
}  // namespace ignite
