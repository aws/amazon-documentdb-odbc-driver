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

// -AL- idea: duplicate the same tests for different log levels.

// TODO enable the log file unit test after logging is properly
// supported in test suites
// https://bitquill.atlassian.net/browse/AD-712
BOOST_AUTO_TEST_CASE(TestLogStreamCreatedOnDefaultInstance) {
  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::DEBUG_LEVEL;

  std::shared_ptr< Logger > logger = Logger::getLoggerInstance();
  // set log level and stream
  logger->setLogLevel(logLevel);
  logger->setLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->getLogLevel();
  BOOST_CHECK(logLevel == loggerLogLevel);

  // check log path
  std::string loggerLogPath = logger->getLogPath();
  BOOST_CHECK_EQUAL(logPath, loggerLogPath);

  std::stringstream stringStream;
  std::string testData;
  testData = "test" + std::to_string(std::rand());

  // Write to log file.
  LOG_DEBUG_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStremOpen());
  BOOST_CHECK(logger->IsEnabled());
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find_last_of(testData));

  // Write to stream.
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Chekc that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find_last_of(testData));
}
// move out of connection test and into log_test.cpp
// can test for setting log path and log level
// can test unknown log level gives error level

// try to swap out the file stream with in-memory stream (only for testing),
// but it is okay to write the file then look at it

/*
BOOST_AUTO_TEST_CASE(TestLogStreamWithInfoLevel) {
  LogLevel::Type logLevel = LogLevel::Type::INFO_LEVEL;

  std::shared_ptr< Logger > logger = Logger::getLoggerInstance();
  // set log level and stream
  logger->setLogLevel(logLevel);

  // check log level
  LogLevel::Type loggerLogLevel = logger->getLogLevel();
  BOOST_CHECK(logLevel == loggerLogLevel);

  std::stringstream stringStream;
  std::string testData1;
  testData1 = "test2" + std::to_string(std::rand());

  // Write to log file.
  LOG_INFO_MSG(testData1);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStremOpen());
  BOOST_CHECK(logger->IsEnabled());
  BOOST_CHECK_EQUAL(std::string::npos,
                    stringStream.str().find_last_of(testData1));

  // Attempt to write debug log to log file, which should fail
  testData1 = "test3" + std::to_string(std::rand());
  LOG_DEBUG_MSG(testData1);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos,
                    stringStream.str().find_last_of(testData1));

  testData1 = "test4" + std::to_string(std::rand());
  // Write to stream.
  LOG_INFO_MSG_TO_STREAM(testData1, &stringStream);

  // Chekc that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find_last_of(testData1));

  // Attempt to write debug log to log stream, which should fail
  testData1 = "test5" + std::to_string(std::rand());
  LOG_DEBUG_MSG_TO_STREAM(testData1, &stringStream);

  // Check that the debug log is not logged
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find_last_of(testData1));
}
*/