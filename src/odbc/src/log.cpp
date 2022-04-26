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
//#include <Windows.h> // -AL- use macros and only include this if it is win32 defined.
#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/log_level.h"

using ignite::odbc::Logger;
using ignite::odbc::config::Configuration;
using ignite::odbc::common::concurrent::CsLockGuard;

#if defined(_WIN32)
#define SLASH "\\"  // Windows
#else
#define SLASH "/"  // unix
#endif

// _logger pointer will  initialized in first call to GetLoggerInstance
std::shared_ptr< Logger > Logger::_logger;

namespace ignite {
namespace odbc {
LogStream::LogStream(Logger* parent)
    : std::basic_ostream< char >(0), strbuf(), logger(parent) {
  init(&strbuf);
}

bool LogStream::operator()() const{
  return logger != nullptr;
}

LogStream::~LogStream() {
  if (logger) {
    logger->WriteMessage(strbuf.str());
  }
}

std::string Logger::CreateFileName() const{
// get process id
  std::ostringstream tmpStream;
  // tmpStream << GetCurrentProcessId();
  tmpStream << getpid();
  std::string pid = tmpStream.str();

  char tStr[1000];
  time_t curTime = time(nullptr);
  struct tm* locTime = localtime(&curTime);
  //strftime(tStr, 1000, "_%Y%m%d_%H%M%S", locTime);
  strftime(tStr, 1000, "_%Y%m%d_", locTime);
  std::string dateTime(tStr, std::find(tStr, tStr + 1000, '\0'));
  std::string fileName("docdb_odbc" + dateTime + pid
                        + ".log");
  return fileName;
}

void Logger::SetLogPath(const std::string& path) {
  if (logPath == path) {
    LOG_DEBUG_MSG(
        "WARNING: SetLogPath is called with the existing path string. "
        "SetLogPath should only be called once in normal circumstances aside from testing");
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

bool Logger::IsFileStremOpen() const {
  return fileStream.is_open();
}

bool Logger::IsEnabled() const {
  return stream != nullptr && (stream != &fileStream || IsFileStremOpen());
}

bool Logger::EnableLog() { // stream is nullptr, which makes enable log called. 
  if (!IsEnabled() && logLevel != LogLevel::Type::OFF && !logPath.empty()
      && stream == &fileStream) {
    std::cout << IsFileStremOpen() << '\n';
    if (logFileName.empty())
        logFileName = CreateFileName();
    fileStream.open(logPath + SLASH + logFileName, std::ios_base::app);
    std::cout << logFileName << '\n';
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
