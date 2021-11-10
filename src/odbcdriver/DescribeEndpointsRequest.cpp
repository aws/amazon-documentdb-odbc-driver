/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/utils/json/JsonSerializer.h>
#include "DescribeEndpointsRequest.h"

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

DescribeEndpointsRequest::DescribeEndpointsRequest() {
}

std::string DescribeEndpointsRequest::SerializePayload() const {
    return "{}";
}

Aws::Http::HeaderValueCollection
DescribeEndpointsRequest::GetRequestSpecificHeaders() const {
    Aws::Http::HeaderValueCollection headers;
    headers.insert(Aws::Http::HeaderValuePair(
        "X-Amz-Target", "Database_20181101.DescribeEndpoints"));
    return headers;
}
