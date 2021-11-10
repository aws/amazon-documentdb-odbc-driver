/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <string>

enum class ScalarType {
    NOT_SET,
    VARCHAR,
    BOOLEAN,
    BIGINT,
    DOUBLE,
    TIMESTAMP,
    DATE,
    TIME,
    INTERVAL_DAY_TO_SECOND,
    INTERVAL_YEAR_TO_MONTH,
    UNKNOWN,
    INTEGER
};

namespace ScalarTypeMapper {
    ScalarType GetScalarTypeForName(const std::string& name);
    std::string GetNameForScalarType(ScalarType value);
}
