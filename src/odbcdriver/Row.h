/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <vector>
#include "Datum.h"
#include <utility>

namespace Aws {
namespace Utils {
namespace Json {
class JsonValue;
class JsonView;
}  // namespace Json
}  // namespace Utils
}  // namespace Aws

/**
 * <p>Represents a single row in the query results
 * API Reference</a></p>
 */
class Row {
   public:
    Row();
    Row(Aws::Utils::Json::JsonView jsonValue);
    Row& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline const std::vector< Datum >& GetData()
        const {
        return m_data;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline bool DataHasBeenSet() const {
        return m_dataHasBeenSet;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline void SetData(
        const std::vector< Datum >& value) {
        m_dataHasBeenSet = true;
        m_data = value;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline void SetData(
        std::vector< Datum >&& value) {
        m_dataHasBeenSet = true;
        m_data = std::move(value);
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& WithData(
        const std::vector< Datum >& value) {
        SetData(value);
        return *this;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& WithData(
        std::vector< Datum >&& value) {
        SetData(std::move(value));
        return *this;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& AddData(const Datum& value) {
        m_dataHasBeenSet = true;
        m_data.push_back(value);
        return *this;
    }

    /**
     * <p>List of data points in a single row of the result set.</p>
     */
    inline Row& AddData(Datum&& value) {
        m_dataHasBeenSet = true;
        m_data.push_back(std::move(value));
        return *this;
    }

   private:
    std::vector< Datum > m_data;
    bool m_dataHasBeenSet;
};
