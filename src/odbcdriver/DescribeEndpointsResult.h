/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <vector>
#include <utility>

#include "Endpoint.h"

namespace Aws {
template < typename RESULT_TYPE >
class AmazonWebServiceResult;

namespace Utils {
namespace Json {
class JsonValue;
}  // namespace Json
}  // namespace Utils
}  // namespace Aws

class DescribeEndpointsResult {
   public:
    DescribeEndpointsResult();
    DescribeEndpointsResult(
        const Aws::AmazonWebServiceResult< Aws::Utils::Json::JsonValue >&
            result);
    DescribeEndpointsResult& operator=(
        const Aws::AmazonWebServiceResult< Aws::Utils::Json::JsonValue >&
            result);

    /**
     * <p>An <code>Endpoints</code> object is returned when a
     * <code>DescribeEndpoints</code> request is made.</p>
     */
    inline const std::vector< Endpoint >& GetEndpoints() const {
        return m_endpoints;
    }

    /**
     * <p>An <code>Endpoints</code> object is returned when a
     * <code>DescribeEndpoints</code> request is made.</p>
     */
    inline void SetEndpoints(const std::vector< Endpoint >& value) {
        m_endpoints = value;
    }

    /**
     * <p>An <code>Endpoints</code> object is returned when a
     * <code>DescribeEndpoints</code> request is made.</p>
     */
    inline void SetEndpoints(std::vector< Endpoint >&& value) {
        m_endpoints = std::move(value);
    }

    /**
     * <p>An <code>Endpoints</code> object is returned when a
     * <code>DescribeEndpoints</code> request is made.</p>
     */
    inline DescribeEndpointsResult& WithEndpoints(
        const std::vector< Endpoint >& value) {
        SetEndpoints(value);
        return *this;
    }

    /**
     * <p>An <code>Endpoints</code> object is returned when a
     * <code>DescribeEndpoints</code> request is made.</p>
     */
    inline DescribeEndpointsResult& WithEndpoints(
        std::vector< Endpoint >&& value) {
        SetEndpoints(std::move(value));
        return *this;
    }

    /**
     * <p>An <code>Endpoints</code> object is returned when a
     * <code>DescribeEndpoints</code> request is made.</p>
     */
    inline DescribeEndpointsResult& AddEndpoints(const Endpoint& value) {
        m_endpoints.push_back(value);
        return *this;
    }

    /**
     * <p>An <code>Endpoints</code> object is returned when a
     * <code>DescribeEndpoints</code> request is made.</p>
     */
    inline DescribeEndpointsResult& AddEndpoints(Endpoint&& value) {
        m_endpoints.push_back(std::move(value));
        return *this;
    }

   private:
    std::vector< Endpoint > m_endpoints;
};