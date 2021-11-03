﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/timestream-query/TimestreamQuery_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/timestream-query/model/Datum.h>
#include <utility>

  /**
   * <p>Represents a single row in the query results.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/timestream-query-2018-11-01/Row">AWS
   * API Reference</a></p>
   */
  class Row
  {
  public:
    Row();
    Row(Aws::Utils::Json::JsonView jsonValue);
    Row& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline const Aws::Vector<Aws::TimestreamQuery::Model::Datum>& GetData() const{ return m_data; }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline bool DataHasBeenSet() const { return m_dataHasBeenSet; }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline void SetData(
        const Aws::Vector< Aws::TimestreamQuery::Model::Datum >& value) {
        m_dataHasBeenSet = true;
        m_data = value;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline void SetData(
        Aws::Vector< Aws::TimestreamQuery::Model::Datum >&& value) {
        m_dataHasBeenSet = true;
        m_data = std::move(value);
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& WithData(
        const Aws::Vector< Aws::TimestreamQuery::Model::Datum >& value) {
        SetData(value);
        return *this;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& WithData(
        Aws::Vector< Aws::TimestreamQuery::Model::Datum >&& value) {
        SetData(std::move(value));
        return *this;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& AddData(const Aws::TimestreamQuery::Model::Datum& value) {
        m_dataHasBeenSet = true;
        m_data.push_back(value);
        return *this;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& AddData(Aws::TimestreamQuery::Model::Datum&& value) {
        m_dataHasBeenSet = true;
        m_data.push_back(std::move(value));
        return *this;
    }

  private:

    Aws::Vector< Aws::TimestreamQuery::Model::Datum > m_data;
    bool m_dataHasBeenSet;
  };
