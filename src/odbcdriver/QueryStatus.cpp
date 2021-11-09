/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/utils/json/JsonSerializer.h>
#include "QueryStatus.h"

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

QueryStatus::QueryStatus()
    : m_progressPercentage(0.0),
      m_progressPercentageHasBeenSet(false) {
}

QueryStatus::QueryStatus(JsonView jsonValue)
    : m_progressPercentage(0.0),
      m_progressPercentageHasBeenSet(false) {
    *this = jsonValue;
}

QueryStatus& QueryStatus::operator=(JsonView jsonValue) {
    if (jsonValue.ValueExists("ProgressPercentage")) {
        m_progressPercentage = jsonValue.GetDouble("ProgressPercentage");

        m_progressPercentageHasBeenSet = true;
    }

    return *this;
}

JsonValue QueryStatus::Jsonize() const {
    JsonValue payload;

    if (m_progressPercentageHasBeenSet) {
        payload.WithDouble("ProgressPercentage", m_progressPercentage);
    }

    return payload;
}
