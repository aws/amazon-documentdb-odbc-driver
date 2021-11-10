/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <string>
#include <vector>

#include <utility>
#include "ColumnInfo.h"
#include "QueryStatus.h"
#include "Row.h"

namespace Aws {
template < typename RESULT_TYPE >
class AmazonWebServiceResult;

namespace Utils {
namespace Json {
class JsonValue;
}  // namespace Json
}  // namespace Utils
}  // namespace Aws

class QueryResult {
   public:
    QueryResult();
    QueryResult(
        const Aws::AmazonWebServiceResult< Aws::Utils::Json::JsonValue >&
            result);
    QueryResult& operator=(
        const Aws::AmazonWebServiceResult< Aws::Utils::Json::JsonValue >&
            result);

    /**
     * <p> A unique ID for the given query. </p>
     */
    inline const std::string& GetQueryId() const {
        return m_queryId;
    }

    /**
     * <p> A unique ID for the given query. </p>
     */
    inline void SetQueryId(const std::string& value) {
        m_queryId = value;
    }

    /**
     * <p> A unique ID for the given query. </p>
     */
    inline void SetQueryId(std::string&& value) {
        m_queryId = std::move(value);
    }

    /**
     * <p> A unique ID for the given query. </p>
     */
    inline void SetQueryId(const char* value) {
        m_queryId.assign(value);
    }

    /**
     * <p> A unique ID for the given query. </p>
     */
    inline QueryResult& WithQueryId(const std::string& value) {
        SetQueryId(value);
        return *this;
    }

    /**
     * <p> A unique ID for the given query. </p>
     */
    inline QueryResult& WithQueryId(std::string&& value) {
        SetQueryId(std::move(value));
        return *this;
    }

    /**
     * <p> A unique ID for the given query. </p>
     */
    inline QueryResult& WithQueryId(const char* value) {
        SetQueryId(value);
        return *this;
    }

    /**
     * <p> A pagination token that can be used again on a <code>Query</code>
     * call to get the next set of results. </p>
     */
    inline const std::string& GetNextToken() const {
        return m_nextToken;
    }

    /**
     * <p> A pagination token that can be used again on a <code>Query</code>
     * call to get the next set of results. </p>
     */
    inline void SetNextToken(const std::string& value) {
        m_nextToken = value;
    }

    /**
     * <p> A pagination token that can be used again on a <code>Query</code>
     * call to get the next set of results. </p>
     */
    inline void SetNextToken(std::string&& value) {
        m_nextToken = std::move(value);
    }

    /**
     * <p> A pagination token that can be used again on a <code>Query</code>
     * call to get the next set of results. </p>
     */
    inline void SetNextToken(const char* value) {
        m_nextToken.assign(value);
    }

    /**
     * <p> A pagination token that can be used again on a <code>Query</code>
     * call to get the next set of results. </p>
     */
    inline QueryResult& WithNextToken(const std::string& value) {
        SetNextToken(value);
        return *this;
    }

    /**
     * <p> A pagination token that can be used again on a <code>Query</code>
     * call to get the next set of results. </p>
     */
    inline QueryResult& WithNextToken(std::string&& value) {
        SetNextToken(std::move(value));
        return *this;
    }

    /**
     * <p> A pagination token that can be used again on a <code>Query</code>
     * call to get the next set of results. </p>
     */
    inline QueryResult& WithNextToken(const char* value) {
        SetNextToken(value);
        return *this;
    }

    /**
     * <p> The result set rows returned by the query. </p>
     */
    inline const std::vector< Row >& GetRows() const {
        return m_rows;
    }

    /**
     * <p> The result set rows returned by the query. </p>
     */
    inline void SetRows(const std::vector< Row >& value) {
        m_rows = value;
    }

    /**
     * <p> The result set rows returned by the query. </p>
     */
    inline void SetRows(std::vector< Row >&& value) {
        m_rows = std::move(value);
    }

    /**
     * <p> The result set rows returned by the query. </p>
     */
    inline QueryResult& WithRows(const std::vector< Row >& value) {
        SetRows(value);
        return *this;
    }

    /**
     * <p> The result set rows returned by the query. </p>
     */
    inline QueryResult& WithRows(std::vector< Row >&& value) {
        SetRows(std::move(value));
        return *this;
    }

    /**
     * <p> The result set rows returned by the query. </p>
     */
    inline QueryResult& AddRows(const Row& value) {
        m_rows.push_back(value);
        return *this;
    }

    /**
     * <p> The result set rows returned by the query. </p>
     */
    inline QueryResult& AddRows(Row&& value) {
        m_rows.push_back(std::move(value));
        return *this;
    }

    /**
     * <p> The column data types of the returned result set. </p>
     */
    inline const std::vector< ColumnInfo >& GetColumnInfo() const {
        return m_columnInfo;
    }

    /**
     * <p> The column data types of the returned result set. </p>
     */
    inline void SetColumnInfo(const std::vector< ColumnInfo >& value) {
        m_columnInfo = value;
    }

    /**
     * <p> The column data types of the returned result set. </p>
     */
    inline void SetColumnInfo(std::vector< ColumnInfo >&& value) {
        m_columnInfo = std::move(value);
    }

    /**
     * <p> The column data types of the returned result set. </p>
     */
    inline QueryResult& WithColumnInfo(const std::vector< ColumnInfo >& value) {
        SetColumnInfo(value);
        return *this;
    }

    /**
     * <p> The column data types of the returned result set. </p>
     */
    inline QueryResult& WithColumnInfo(std::vector< ColumnInfo >&& value) {
        SetColumnInfo(std::move(value));
        return *this;
    }

    /**
     * <p> The column data types of the returned result set. </p>
     */
    inline QueryResult& AddColumnInfo(const ColumnInfo& value) {
        m_columnInfo.push_back(value);
        return *this;
    }

    /**
     * <p> The column data types of the returned result set. </p>
     */
    inline QueryResult& AddColumnInfo(ColumnInfo&& value) {
        m_columnInfo.push_back(std::move(value));
        return *this;
    }

    /**
     * <p>Information about the status of the query, including progress and
     * bytes scannned.</p>
     */
    inline const QueryStatus& GetQueryStatus() const {
        return m_queryStatus;
    }

    /**
     * <p>Information about the status of the query, including progress and
     * bytes scannned.</p>
     */
    inline void SetQueryStatus(const QueryStatus& value) {
        m_queryStatus = value;
    }

    /**
     * <p>Information about the status of the query, including progress and
     * bytes scannned.</p>
     */
    inline void SetQueryStatus(QueryStatus&& value) {
        m_queryStatus = std::move(value);
    }

    /**
     * <p>Information about the status of the query, including progress and
     * bytes scannned.</p>
     */
    inline QueryResult& WithQueryStatus(const QueryStatus& value) {
        SetQueryStatus(value);
        return *this;
    }

    /**
     * <p>Information about the status of the query, including progress and
     * bytes scannned.</p>
     */
    inline QueryResult& WithQueryStatus(QueryStatus&& value) {
        SetQueryStatus(std::move(value));
        return *this;
    }

   private:
    std::string m_queryId;

    std::string m_nextToken;

    std::vector< Row > m_rows;

    std::vector< ColumnInfo > m_columnInfo;

    QueryStatus m_queryStatus;
};