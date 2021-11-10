/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include "DatabaseQueryRequest.h"

/**
 */
class DescribeEndpointsRequest
    : public DatabaseQueryRequest {
   public:
    DescribeEndpointsRequest();

    // Service request name is the Operation name which will send this request
    // out, each operation should has unique request name, so that we can get
    // operation's name from this request. Note: this is not true for response,
    // multiple operations may have the same response name, so we can not get
    // operation's name from response.
    inline virtual const char* GetServiceRequestName() const override {
        return "DescribeEndpoints";
    }

    std::string SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;
};