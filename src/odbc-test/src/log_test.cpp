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
#include <boost/optional.hpp>
#include <ignite/odbc/log_level.h>
#include <ignite/odbc/log.h>

#include <string>
#include <random>

#include "odbc_test_suite.h"

using namespace ignite::odbc::common;
using namespace boost::unit_test;

using boost::unit_test::precondition;
using ignite::odbc::OdbcTestSuite;
using ignite::odbc::LogLevel;
using ignite::odbc::Logger;

bool SaveLoggerVars(
    std::shared_ptr< Logger > logger, boost::optional< std::string >& origLogPath,
                    boost::optional< LogLevel::Type >& origLogLevel) {
  if (logger->IsEnabled()) {
    origLogPath = logger->GetLogPath();
    origLogLevel = logger->GetLogLevel();

    return true;
  }
  origLogPath = boost::none;
  origLogLevel = boost::none;

  return false;
}

void setLoggerVars(std::shared_ptr< Logger > logger,
                    boost::optional< std::string >& origLogPath,
                    boost::optional< LogLevel::Type >& origLogLevel) {
  // pre-requiste: origLogPath and origLogLevel hold valid values

  logger->SetLogLevel(origLogLevel.get());
  logger->SetLogPath(origLogPath.get());
}

BOOST_AUTO_TEST_CASE(TestLogStreamCreatedOnDefaultInstance) {
  std::minstd_rand randNum;
  randNum.seed(29);

  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::DEBUG_LEVEL;

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // set log level and stream
  logger->SetLogLevel(logLevel);
  logger->SetLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->GetLogLevel();
  BOOST_CHECK_EQUAL(logLevel, loggerLogLevel);

  // check log path
  std::string loggerLogPath = logger->GetLogPath();
  BOOST_CHECK_EQUAL(logPath, loggerLogPath);

  std::stringstream stringStream;
  std::string testData;
  testData = "defTest" + std::to_string(randNum());

  // Write to log file.
  LOG_DEBUG_MSG("TestLogStreamCreatedOnDefaultInstance begins. Log path/level changes are expected.");

  LOG_DEBUG_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStreamOpen());
  BOOST_CHECK(logger->IsEnabled());

  // this boost check means that testData is not in stringStream
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));
  // find means finding the last instance of the string (param)

  // Write to stream.
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Chekc that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  // this boost check means that testData is in stringStream
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  LOG_DEBUG_MSG("TestLogStreamCreatedOnDefaultInstance ends. Log path/level changes are expected.");

  // set the original log level / log path back
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}

BOOST_AUTO_TEST_CASE(TestLogStreamWithInfoLevel) {
  std::minstd_rand randNum;
  randNum.seed(31);

  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::INFO_LEVEL;

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  // save the original log path / log level
  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // set log level and stream
  logger->SetLogLevel(logLevel);
  logger->SetLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->GetLogLevel();
  BOOST_CHECK_EQUAL(logLevel,  loggerLogLevel);

  std::stringstream stringStream;
  std::string testData;
  testData = "infoLvlTest1" + std::to_string(randNum());

  // Write to log file.
  LOG_INFO_MSG("TestLogStreamWithInfoLevel begins. Log path/level changes are expected.");

  LOG_INFO_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStreamOpen());
  BOOST_CHECK(logger->IsEnabled());

  //check that stringStream does not have testData
  BOOST_CHECK_EQUAL(std::string::npos,
                    stringStream.str().find(testData));

  // Attempt to write debug log to log file, which should fail
  testData = "infoLvlTest2" + std::to_string(randNum());
  LOG_DEBUG_MSG(testData);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos,
                    stringStream.str().find(testData));

  testData = "infoLvlTest3" + std::to_string(randNum());
  // Write to stream.
  LOG_INFO_MSG_TO_STREAM(testData, &stringStream);

  // Chekc that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log stream, which should fail
  testData = "infoLvlTest4" + std::to_string(randNum());
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  LOG_INFO_MSG("TestLogStreamWithInfoLevel ends. Log path/level changes are expected.");

  // set the original log level / log path back
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}


BOOST_AUTO_TEST_CASE(TestLogStreamWithErrorLevel) {
  std::minstd_rand randNum;
  randNum.seed(42);

  std::string logPath = DEFAULT_LOG_PATH;
  LogLevel::Type logLevel = LogLevel::Type::ERROR_LEVEL;

  std::shared_ptr< Logger > logger = Logger::GetLoggerInstance();

  // save the original log path / log level
  boost::optional< std::string > origLogPath;
  boost::optional< LogLevel::Type > origLogLevel;
  bool logVarSaved = SaveLoggerVars(logger, origLogPath, origLogLevel);

  // set log level and stream
  logger->SetLogLevel(logLevel);
  logger->SetLogPath(logPath);

  // check log level
  LogLevel::Type loggerLogLevel = logger->GetLogLevel();
  BOOST_CHECK_EQUAL(logLevel,  loggerLogLevel);

  std::stringstream stringStream;
  std::string testData;
  testData = "errLvlTest1" + std::to_string(randNum());

  // Write to log file.
  LOG_ERROR_MSG("(Not an actual error, logged for clarity) TestLogStreamWithErrorLevel begins. Log path/level changes are expected.");

  LOG_ERROR_MSG(testData);

  // Check that log file is working
  BOOST_CHECK(logger->IsFileStreamOpen());
  BOOST_CHECK(logger->IsEnabled());

  // check that stringStream does not have testData
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log file, which should fail
  testData = "errLvlTest2" + std::to_string(randNum());
  LOG_DEBUG_MSG(testData);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write info log to log file, which should fail
  testData = "errLvlTest3" + std::to_string(randNum());
  LOG_INFO_MSG(testData);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  testData = "errLvlTest4" + std::to_string(randNum());
  // Write to stream.
  LOG_ERROR_MSG_TO_STREAM(testData, &stringStream);

  // Chekc that logger is still enabled after writing to stream
  BOOST_CHECK(logger->IsEnabled());

  // Check that log stream is working
  BOOST_CHECK_NE(std::string::npos, stringStream.str().find(testData));

  // Attempt to write debug log to log stream, which should fail
  testData = "errLvlTest5" + std::to_string(randNum());
  LOG_DEBUG_MSG_TO_STREAM(testData, &stringStream);

  // Check that the debug log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  // Attempt to write info log to log stream, which should fail
  testData = "errLvlTest6" + std::to_string(randNum());
  LOG_INFO_MSG_TO_STREAM(testData, &stringStream);

  // Check that the info log is not logged
  BOOST_CHECK_EQUAL(std::string::npos, stringStream.str().find(testData));

  LOG_ERROR_MSG("(Not an actual error, logged for clarity) TestLogStreamWithErrorLevel ends. Log path/level changes are expected.");

  // set the original log level / log path back
  if (logVarSaved)
    setLoggerVars(logger, origLogPath, origLogLevel);
}
