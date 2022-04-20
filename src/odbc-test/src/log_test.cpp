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
#include <boost/test/unit_test.hpp>
#include <ignite/odbc/log_level.h>
#include <ignite/odbc/log.h>
#include <string>

#include "odbc_test_suite.h"

using namespace ignite::odbc::common;
using namespace boost::unit_test;

using boost::unit_test::precondition;
using ignite::odbc::OdbcTestSuite;
using ignite::odbc::LogLevel;
using ignite::odbc::Logger;


// TODO enable the log file unit test after logging is properly
// supported in test suites
// https://bitquill.atlassian.net/browse/AD-712
BOOST_AUTO_TEST_CASE(TestLogFileCreated) {
  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::DEBUG_LEVEL;

  std::shared_ptr< Logger > logger = Logger::getLoggerInstance();
  // set log level and log path
  logger->setLogLevel(logLevel);
  logger->setLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->getLogLevel();
  BOOST_CHECK(logLevel == loggerLogLevel);

  // check log path
  std::string loggerLogPath = logger->getLogPath();
  BOOST_CHECK_EQUAL(logPath, loggerLogPath);

  LOG_DEBUG_MSG("test");

  // check that the file stream is open
  bool loggerEnabled = logger->IsEnabled();
  BOOST_CHECK(loggerEnabled);
}
// move out of connection test and into log_test.cpp
// can test for setting log path and log level
// can test unknown log level gives error level

// try to swap out the file stream with in-memory stream (only for testing),
// but it is okay to write the file then look at it