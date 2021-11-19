/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include "DatabaseQueryRequest.h"

#include <aws/core/utils/UUID.h>
#include <string>

#include <utility>

/**
 */
class QueryRequest : public DatabaseQueryRequest {
   public:
    QueryRequest();

    // Service request name is the Operation name which will send this request
    // out, each operation should has unique request name, so that we can get
    // operation's name from this request. Note: this is not true for response,
    // multiple operations may have the same response name, so we can not get
    // operation's name from response.
    inline virtual const char* GetServiceRequestName() const override {
        return "Query";
    }

    std::string SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline const std::string& GetQueryString() const {
        return m_queryString;
    }

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline bool QueryStringHasBeenSet() const {
        return m_queryStringHasBeenSet;
    }

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline void SetQueryString(const std::string& value) {
        m_queryStringHasBeenSet = true;
        m_queryString = value;
    }

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline void SetQueryString(std::string&& value) {
        m_queryStringHasBeenSet = true;
        m_queryString = std::move(value);
    }

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline void SetQueryString(const char* value) {
        m_queryStringHasBeenSet = true;
        m_queryString.assign(value);
    }

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline QueryRequest& WithQueryString(const std::string& value) {
        SetQueryString(value);
        return *this;
    }

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline QueryRequest& WithQueryString(std::string&& value) {
        SetQueryString(std::move(value));
        return *this;
    }

    /**
     * <p> The query to be executed by Database. </p>
     */
    inline QueryRequest& WithQueryString(const char* value) {
        SetQueryString(value);
        return *this;
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline const std::string& GetClientToken() const {
        return m_clientToken;
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline bool ClientTokenHasBeenSet() const {
        return m_clientTokenHasBeenSet;
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline void SetClientToken(const std::string& value) {
        m_clientTokenHasBeenSet = true;
        m_clientToken = value;
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline void SetClientToken(std::string&& value) {
        m_clientTokenHasBeenSet = true;
        m_clientToken = std::move(value);
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline void SetClientToken(const char* value) {
        m_clientTokenHasBeenSet = true;
        m_clientToken.assign(value);
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline QueryRequest& WithClientToken(const std::string& value) {
        SetClientToken(value);
        return *this;
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline QueryRequest& WithClientToken(std::string&& value) {
        SetClientToken(std::move(value));
        return *this;
    }

    /**
     * <p> Unique, case-sensitive string of up to 64 ASCII characters that you
     * specify when you make a Query request. Providing a
     * <code>ClientToken</code> makes the call to <code>Query</code> idempotent,
     * meaning that multiple identical calls have the same effect as one single
     * call. </p> <p>Your query request will fail in the following cases:</p>
     * <ul> <li> <p> If you submit a request with the same client token outside
     * the 5-minute idepotency window. </p> </li> <li> <p> If you submit a
     * request with the same client token but a change in other parameters
     * within the 5-minute idempotency window. </p> </li> </ul> <p> After 4
     * hours, any request with the same client token is treated as a new
     * request. </p>
     */
    inline QueryRequest& WithClientToken(const char* value) {
        SetClientToken(value);
        return *this;
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline const std::string& GetNextToken() const {
        return m_nextToken;
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline bool NextTokenHasBeenSet() const {
        return m_nextTokenHasBeenSet;
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline void SetNextToken(const std::string& value) {
        m_nextTokenHasBeenSet = true;
        m_nextToken = value;
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline void SetNextToken(std::string&& value) {
        m_nextTokenHasBeenSet = true;
        m_nextToken = std::move(value);
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline void SetNextToken(const char* value) {
        m_nextTokenHasBeenSet = true;
        m_nextToken.assign(value);
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline QueryRequest& WithNextToken(const std::string& value) {
        SetNextToken(value);
        return *this;
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline QueryRequest& WithNextToken(std::string&& value) {
        SetNextToken(std::move(value));
        return *this;
    }

    /**
     * <p> A pagination token passed to get a set of results. </p>
     */
    inline QueryRequest& WithNextToken(const char* value) {
        SetNextToken(value);
        return *this;
    }

    /**
     * <p> The total number of rows to return in the output. If the total number
     * of rows available is more than the value specified, a NextToken is
     * provided in the command's output. To resume pagination, provide the
     * NextToken value in the starting-token argument of a subsequent command.
     * </p>
     */
    inline int GetMaxRows() const {
        return m_maxRows;
    }

    /**
     * <p> The total number of rows to return in the output. If the total number
     * of rows available is more than the value specified, a NextToken is
     * provided in the command's output. To resume pagination, provide the
     * NextToken value in the starting-token argument of a subsequent command.
     * </p>
     */
    inline bool MaxRowsHasBeenSet() const {
        return m_maxRowsHasBeenSet;
    }

    /**
     * <p> The total number of rows to return in the output. If the total number
     * of rows available is more than the value specified, a NextToken is
     * provided in the command's output. To resume pagination, provide the
     * NextToken value in the starting-token argument of a subsequent command.
     * </p>
     */
    inline void SetMaxRows(int value) {
        m_maxRowsHasBeenSet = true;
        m_maxRows = value;
    }

    /**
     * <p> The total number of rows to return in the output. If the total number
     * of rows available is more than the value specified, a NextToken is
     * provided in the command's output. To resume pagination, provide the
     * NextToken value in the starting-token argument of a subsequent command.
     * </p>
     */
    inline QueryRequest& WithMaxRows(int value) {
        SetMaxRows(value);
        return *this;
    }

   private:
    std::string m_queryString;
    bool m_queryStringHasBeenSet;

    std::string m_clientToken;
    bool m_clientTokenHasBeenSet;

    std::string m_nextToken;
    bool m_nextTokenHasBeenSet;

    int m_maxRows;
    bool m_maxRowsHasBeenSet;
};
