/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/core/Region.h>
#include <string>

namespace DatabaseQueryEndpoint 
{
std::string ForRegion(const std::string& regionName, bool useDualStack = false);
} // namespace DatabaseQueryEndpoint