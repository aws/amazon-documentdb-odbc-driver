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

#ifndef _DOCUMENTDB_ODBC_SQL_SQL_PARSER
#define _DOCUMENTDB_ODBC_SQL_SQL_PARSER

#include <documentdb/odbc/sql/sql_command.h>
#include <documentdb/odbc/sql/sql_lexer.h>

#include <memory>
#include <string>

namespace documentdb {
namespace odbc {
/**
 * SQL parser.
 */
class SqlParser {
 public:
  /**
   * Default constructor.
   *
   * @param sql SQL request.
   */
  SqlParser(const std::string& sql);

  /**
   * Destructor.
   */
  ~SqlParser();

  /**
   * Get next command.
   *
   * @return Parsed command on success and null on failure.
   */
  std::shared_ptr< SqlCommand > GetNextCommand();

 private:
  /**
   *
   */
  std::shared_ptr< SqlCommand > ProcessCommand();

  /** SQL lexer. */
  SqlLexer lexer;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_SQL_SQL_PARSER
