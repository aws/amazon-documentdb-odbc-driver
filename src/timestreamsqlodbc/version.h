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

#ifndef __VERSION_H__
#define __VERSION_H__

/*
 *	BuildAll may pass DRIVERVERSION, RESOURCE_VERSION
 *	and DRVFILE_VERSION via winbuild/timestreamodbc.vcxproj.
 */
#ifdef TS_ODBC_VERSION

#ifndef TIMESTREAMDRIVERVERSION
#define TIMESTREAMDRIVERVERSION TS_ODBC_VERSION
#endif
#ifndef TIMESTREAM_RESOURCE_VERSION
#define TIMESTREAM_RESOURCE_VERSION TIMESTREAMDRIVERVERSION
#endif
#ifndef TS_DRVFILE_VERSION
#define TS_DRVFILE_VERSION TS_ODBC_DRVFILE_VERSION
#endif

#endif  // TS_ODBC_VERSION

#endif
