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
#ifndef _DOCUMENTDB_ODBC_COMMON_COMMON
#define _DOCUMENTDB_ODBC_COMMON_COMMON

#if defined(__unix__) || defined(__unix) \
    || (defined(__APPLE__) && defined(__MACH__))
#define PREDEF_PLATFORM_UNIX_OR_APPLE 1
#endif

#define DOCUMENTDB_EXPORT __declspec(dllexport)
#define DOCUMENTDB_IMPORT __declspec(dllimport)
#define DOCUMENTDB_CALL __stdcall

#define DOCUMENTDB_IMPORT_EXPORT DOCUMENTDB_EXPORT

#include <iostream>

#define DOCUMENTDB_TRACE_ALLOC(addr)                                  \
  std::cout << "ALLOC " << __FILE__ << "(" << __LINE__ << "): 0x" \
            << (void*)addr << std::endl;

/**
 * Common construction to disable copy constructor and assignment for class.
 */
#define DOCUMENTDB_NO_COPY_ASSIGNMENT(cls) \
  cls(const cls& src);                 \
  cls& operator=(const cls& other);

#if (__cplusplus >= 201103L)
#define DOCUMENTDB_NO_THROW noexcept
#else
#define DOCUMENTDB_NO_THROW throw()
#endif

#define DOCUMENTDB_UNUSED(x) ((void)x)

#endif  //_DOCUMENTDB_ODBC_COMMON_COMMON
