/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <string>

#include <utility>

namespace Aws {
template < typename RESULT_TYPE >
class AmazonWebServiceResult;

namespace Utils {
namespace Json {
class JsonValue;
}  // namespace Json
}  // namespace Utils
}  // namespace Aws

class CancelQueryResult {
   public:
    CancelQueryResult();
    CancelQueryResult(
        const Aws::AmazonWebServiceResult< Aws::Utils::Json::JsonValue >&
            result);
    CancelQueryResult& operator=(
        const Aws::AmazonWebServiceResult< Aws::Utils::Json::JsonValue >&
            result);

    /**
     * <p> A <code>CancellationMessage</code> is returned when a
     * <code>CancelQuery</code> request for the query specified by
     * <code>QueryId</code> has already been issued. </p>
     */
    inline const std::string& GetCancellationMessage() const {
        return m_cancellationMessage;
    }

    /**
     * <p> A <code>CancellationMessage</code> is returned when a
     * <code>CancelQuery</code> request for the query specified by
     * <code>QueryId</code> has already been issued. </p>
     */
    inline void SetCancellationMessage(const std::string& value) {
        m_cancellationMessage = value;
    }

    /**
     * <p> A <code>CancellationMessage</code> is returned when a
     * <code>CancelQuery</code> request for the query specified by
     * <code>QueryId</code> has already been issued. </p>
     */
    inline void SetCancellationMessage(std::string&& value) {
        m_cancellationMessage = std::move(value);
    }

    /**
     * <p> A <code>CancellationMessage</code> is returned when a
     * <code>CancelQuery</code> request for the query specified by
     * <code>QueryId</code> has already been issued. </p>
     */
    inline void SetCancellationMessage(const char* value) {
        m_cancellationMessage.assign(value);
    }

    /**
     * <p> A <code>CancellationMessage</code> is returned when a
     * <code>CancelQuery</code> request for the query specified by
     * <code>QueryId</code> has already been issued. </p>
     */
    inline CancelQueryResult& WithCancellationMessage(
        const std::string& value) {
        SetCancellationMessage(value);
        return *this;
    }

    /**
     * <p> A <code>CancellationMessage</code> is returned when a
     * <code>CancelQuery</code> request for the query specified by
     * <code>QueryId</code> has already been issued. </p>
     */
    inline CancelQueryResult& WithCancellationMessage(std::string&& value) {
        SetCancellationMessage(std::move(value));
        return *this;
    }

    /**
     * <p> A <code>CancellationMessage</code> is returned when a
     * <code>CancelQuery</code> request for the query specified by
     * <code>QueryId</code> has already been issued. </p>
     */
    inline CancelQueryResult& WithCancellationMessage(const char* value) {
        SetCancellationMessage(value);
        return *this;
    }

   private:
    std::string m_cancellationMessage;
};
