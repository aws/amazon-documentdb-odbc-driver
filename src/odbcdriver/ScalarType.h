﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/timestream-query/TimestreamQuery_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>

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
    ScalarType GetScalarTypeForName(const Aws::String& name);
    Aws::String GetNameForScalarType(ScalarType value);
}
