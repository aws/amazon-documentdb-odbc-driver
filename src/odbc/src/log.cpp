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

// initalize pointer to NULL so that it can be initialized in first call to
// getLoggerInstance
Logger* Logger::_logger = NULL;

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

// -AL- to do add: log error somehow when program is trying to set path TWICE or
// more
void Logger::setLogPath(std::string path) {
  logPath = path; // -AL- todo create function to deal with duplicate code???
  if (!IsEnabled() && logLevel != LogLevel::Type::OFF && !logPath.empty()) {
    stream.open(logPath);
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

/*
Logger::Logger(std::string path, LogLevel::Type level)
    : mutex(), stream(), logLevel(level) {
  if (logLevel != LogLevel::Type::OFF && !path.empty()) {
    stream.open(path);
  }
}

// -AL- todo remove later
Logger::Logger(const char* path, const char* level) : mutex(), stream(), logLevel() {
  if (level) {
    logLevel = LogLevel::FromString(
        level, LogLevel::Type::INFO_LEVEL);  // -AL- draft code
    // todo add default level here // APR-11: for now, have INFO level
  }
  if (logLevel != LogLevel::Type::OFF && path) {
    stream.open(path);
  }

}
*/

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

std::string Logger::getLogPath() {
  return logPath;
}

}  // namespace odbc
}  // namespace ignite
