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
#ifdef _WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>

#include "ignite/odbc/meta/column_meta.h"
#include "ignite/odbc/impl/binary/binary_common.h"

#include "odbc_test_suite.h"

using ignite::odbc::OdbcTestSuite;
using ignite::odbc::meta::ColumnMeta;
using ignite::odbc::meta::Nullability;

BOOST_AUTO_TEST_CASE(TestGetAttribute) {
  using namespace ignite::odbc::impl::binary;

  std::string schema("database");
  std::string table("table");
  std::string column("column");
  ColumnMeta columnMeta(schema, table, column, JDBC_TYPE_VARCHAR,
                        Nullability::NULLABLE);

  SQLLEN intVal;
  std::string resVal;
  bool found;

  // test SQL_DESC_LABEL
  found = columnMeta.GetAttribute(SQL_DESC_LABEL, resVal);
  BOOST_CHECK(found);

  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_TYPE
  found = columnMeta.GetAttribute(SQL_DESC_TYPE, intVal);
  BOOST_CHECK(found);

  BOOST_CHECK_EQUAL(intVal, SQL_VARCHAR);



}


