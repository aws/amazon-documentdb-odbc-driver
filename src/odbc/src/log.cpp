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

#ifdef __linux__
#include <pwd.h>
#include <unistd.h>
#endif

#include <cstdlib>

#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/log_level.h"

using ignite::odbc::Logger;
using ignite::odbc::common::concurrent::CsLockGuard;
using ignite::odbc::config::Configuration;

#if defined(_WIN32)
#define SLASH "\\"  // Windows
#else
#define SLASH "/"  // unix
#endif

// logger_ pointer will  initialized in first call to GetLoggerInstance
std::shared_ptr< Logger > Logger::logger_;

namespace ignite {
namespace odbc {
LogStream::LogStream(Logger* parent)
    : std::basic_ostream< char >(0), strbuf(), logger(parent) {
  init(&strbuf);
}

bool LogStream::operator()() const {
  return logger != nullptr;
}

LogStream::~LogStream() {
  if (logger) {
    logger->WriteMessage(strbuf.str());
  }
}

std::string Logger::GetDefaultLogPath() {
  std::string defPath;
#ifdef unix
  struct passwd* pwd = getpwuid(getuid());
  if (pwd) {
    defPath = pwd->pw_dir;
  } else {
    // try the $HOME environment variable (note: $HOME is not defined in OS X)
    defPath = common::GetEnv("HOME");
  }
#elif defined(_WIN32)
  defPath = common::GetEnv("USERPROFILE");
  if (defPath.empty()) {
    const std::string homeDirectory = common::GetEnv("HOMEDRIVE");
    const std::string homepath = common::GetEnv("HOMEPATH");
    defPath = homeDirectory + homepath;
  }
#endif

  if (defPath.empty()) {
    // couldn't find home directory, fall back to current working directory
    std::cout << "warning: couldn't find home directory, the default log path "
                 "is set as the current working directory"
              << '\n';
    defPath = ".";
  }

  return defPath;
}

std::string Logger::CreateFileName() const {
  char tStr[1000];
  time_t curTime = time(nullptr);
  struct tm* locTime = localtime(&curTime);
  strftime(tStr, 1000, "%Y%m%d", locTime);
  std::string dateTime(tStr, std::find(tStr, tStr + 1000, '\0'));
  std::string fileName("docdb_odbc_" + dateTime + ".log");
  return fileName;
}

void Logger::SetLogPath(const std::string& path) {
  if (logPath == path) {
    LOG_DEBUG_MSG(
        "WARNING: SetLogPath is called with the existing path string. "
        "SetLogPath should only be called once in normal circumstances aside "
        "from testing");
    return;
  }
  std::string oldLogPath = logPath;
  std::string oldLogFileName = logFileName;
  logPath = path;
  if (IsEnabled() && logLevel != LogLevel::Type::OFF && !logPath.empty()) {
    LOG_INFO_MSG("Reset log path: Log path is changed to " + logPath + SLASH
                 + logFileName);
    fileStream.close();
    LOG_INFO_MSG("Previously logged information is stored in log file "
                 + oldLogPath + SLASH + oldLogFileName);
  }
  SetLogStream(&fileStream);
}

void Logger::SetLogStream(std::ostream* logStream) {
  stream = logStream;
}

void Logger::SetLogLevel(LogLevel::Type level) {
  logLevel = level;
}

bool Logger::IsFileStreamOpen() const {
  return fileStream.is_open();
}

bool Logger::IsEnabled() const {
  return stream != nullptr && (stream != &fileStream || IsFileStreamOpen());
}

bool Logger::EnableLog() {  // stream is nullptr, which makes enable log called.
  if (!IsEnabled() && logLevel != LogLevel::Type::OFF && !logPath.empty()
      && stream == &fileStream) {
    if (logFileName.empty())
      logFileName = CreateFileName();
    fileStream.open(logPath + SLASH + logFileName, std::ios_base::app);
  }
  return IsEnabled();
}

void Logger::WriteMessage(std::string const& message) {
  if (IsEnabled()) {
    CsLockGuard guard(mutex);
    *stream << message << std::endl;
  }
}

LogLevel::Type Logger::GetLogLevel() const {
  return logLevel;
}

std::string& Logger::GetLogPath() {
  return logPath;
}

}  // namespace odbc
}  // namespace ignite
