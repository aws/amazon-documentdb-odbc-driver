/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <string>

#include "DatabaseQueryRequest.h"

#include <utility>

class CancelQueryRequest : public DatabaseQueryRequest {
   public:
    CancelQueryRequest();

    // Service request name is the Operation name which will send this request
    // out, each operation should has unique request name, so that we can get
    // operation's name from this request. Note: this is not true for response,
    // multiple operations may have the same response name, so we can not get
    // operation's name from response.
    inline virtual const char* GetServiceRequestName() const override {
        return "CancelQuery";
    }

    std::string SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline const std::string& GetQueryId() const {
        return m_queryId;
    }

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline bool QueryIdHasBeenSet() const {
        return m_queryIdHasBeenSet;
    }

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline void SetQueryId(const std::string& value) {
        m_queryIdHasBeenSet = true;
        m_queryId = value;
    }

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline void SetQueryId(std::string&& value) {
        m_queryIdHasBeenSet = true;
        m_queryId = std::move(value);
    }

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline void SetQueryId(const char* value) {
        m_queryIdHasBeenSet = true;
        m_queryId.assign(value);
    }

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline CancelQueryRequest& WithQueryId(const std::string& value) {
        SetQueryId(value);
        return *this;
    }

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline CancelQueryRequest& WithQueryId(std::string&& value) {
        SetQueryId(std::move(value));
        return *this;
    }

    /**
     * <p> The id of the query that needs to be cancelled. <code>QueryID</code>
     * is returned as part of QueryResult. </p>
     */
    inline CancelQueryRequest& WithQueryId(const char* value) {
        SetQueryId(value);
        return *this;
    }

   private:
    std::string m_queryId;
    bool m_queryIdHasBeenSet;
};