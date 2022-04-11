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
#include "ignite/odbc/log_level.h"

#include <cstdlib>

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

Logger::Logger(const char* path, const char* level) : mutex(), stream(), logLevel() {
  if ((std::string)level != "OFF" && path) {
    stream.open(path);
  }
  if (level) {
    logLevel = LogLevel::FromString(level, LogLevel::Type::DEBUG_LEVEL); // -AL- draft code
    // todo add default level here 
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

LogLevel::Type Logger::GetLogLevel() {
  return logLevel;
}

Logger* Logger::Get() {
  const char* envPathVarName = "DOC_DB_ODBC_LOG_PATH";
  const char* envLvlVarName = "DOC_DB_LOG_LEVEL";
  // -AL- note: after PATH variable is changed, need to reopen VS Code to run debug, but don't need to rebuild, I believe
  // TODO retrieve logging level, only pass in logging level if it is provided 
  static Logger logger(
      getenv(envPathVarName),
      getenv(envLvlVarName));  // TODO -AL- add default level here
      
  // static Logger logger(getenv(envPathVarName), "INFO");  // -AL- for testing purpose. 
  //static Logger logger(getenv(envPathVarName), "ERROR"); //  -AL- for testing purpose. 
  // static Logger logger(getenv(envPathVarName), "OFF"); //  -AL- for testing purpose. 
  return logger.IsEnabled() ? &logger : 0;
}
}  // namespace odbc
}  // namespace ignite
