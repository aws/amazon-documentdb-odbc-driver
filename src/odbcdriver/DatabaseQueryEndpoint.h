/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/core/Region.h>
#include <aws/core/utils/memory/stl/AWSString.h>

namespace DatabaseQueryEndpoint 
{
Aws::String ForRegion(const Aws::String& regionName, bool useDualStack = false);
} // namespace DatabaseQueryEndpoint