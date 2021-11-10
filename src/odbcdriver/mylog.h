/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#ifndef __MYLOG_H__
#define __MYLOG_H__

#undef DLL_DECLARE
#ifdef WIN32
#ifdef _MYLOG_FUNCS_IMPLEMENT_
#define DLL_DECLARE _declspec(dllexport)
#else
#ifdef _MYLOG_FUNCS_IMPORT_
#define DLL_DECLARE _declspec(dllimport)
#else
#define DLL_DECLARE
#endif /* _MYLOG_FUNCS_IMPORT_ */
#endif /* _MYLOG_FUNCS_IMPLEMENT_ */
#else
#define DLL_DECLARE
#endif /* WIN32 */

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GNUC__
#define __attribute__(x)
#endif

#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"
// This will basically disable all warnings.
// There is no way to disable a warning that is coming out in Amazon Linux 2 so we need to blanket them off.
#pragma GCC system_header 
#endif  // __linux__


DLL_DECLARE int mylog(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
DLL_DECLARE int myprintf(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));

const char *po_basename(const char *path);

#define PREPEND_FMT "%20.20s[%s]%d: "
#define PREPEND_ITEMS , po_basename(__FILE__), __func__, __LINE__

#if defined(__GNUC__) && !defined(__APPLE__)
#define MYLOG(level, fmt, ...)                                                          \
    do {                                                                                \
        _Pragma("GCC diagnostic push");                                                 \
        _Pragma("GCC diagnostic ignored \"-Wformat=\"");                                \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"");                               \
        (level < get_mylog() ? mylog(PREPEND_FMT fmt PREPEND_ITEMS, ##__VA_ARGS__) : 0);\
        _Pragma("GCC diagnostic pop");                                                  \
    } while (0)

#define MYPRINTF(level, fmt, ...)                                                           \
    do {                                                                                    \
        _Pragma("GCC diagnostic push");                                                     \
        _Pragma("GCC diagnostic ignored \"-Wformat=\"");                                    \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"");                                   \
        (level < get_mylog() ? myprintf((fmt), ##__VA_ARGS__) : 0);                         \
        _Pragma("GCC diagnostic pop");                                                      \
    } while (0)
#elif defined WIN32 /* && _MSC_VER > 1800 */
#define MYLOG(level, fmt, ...)                               \
    ((int)level <= get_mylog()                               \
         ? mylog(PREPEND_FMT fmt PREPEND_ITEMS, __VA_ARGS__) \
         : (printf || printf((fmt), __VA_ARGS__)))
#define MYPRINTF(level, fmt, ...)                           \
    ((int)level <= get_mylog() ? myprintf(fmt, __VA_ARGS__) \
                               : (printf || printf((fmt), __VA_ARGS__)))
#else
#define MYLOG(level, ...)                                                \
    do {                                                                 \
        _Pragma("clang diagnostic push");                                \
        _Pragma("clang diagnostic ignored \"-Wformat-pedantic\"");       \
        (level < get_mylog()                                             \
             ? (mylog(PREPEND_FMT PREPEND_ITEMS), myprintf(__VA_ARGS__)) \
             : 0);                                                       \
        _Pragma("clang diagnostic pop");                                 \
    } while (0)
#define MYPRINTF(level, ...)                                       \
    do {                                                           \
        _Pragma("clang diagnostic push");                          \
        _Pragma("clang diagnostic ignored \"-Wformat-pedantic\""); \
        (level < get_mylog() ? myprintf(__VA_ARGS__) : 0);         \
        _Pragma("clang diagnostic pop");                           \
    } while (0)
#endif /* __GNUC__ */

enum LogLevel {
    LOG_OFF = 0,
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE,
    LOG_ALL
};

//int get_qlog(void);
int get_mylog(void);

int getGlobalDebug();
int setGlobalDebug(int val);
int getGlobalCommlog();
int setGlobalCommlog(int val);
int writeGlobalLogs();
int getLogDir(char *dir, int dirmax);
int setLogDir(const char *dir);

void InitializeLogging(void);
void FinalizeLogging(void);

#ifdef __linux__
#pragma GCC diagnostic pop
#endif  // __linux__

#ifdef __cplusplus
}
#endif
#endif /* __MYLOG_H__ */