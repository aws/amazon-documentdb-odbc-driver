/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/AmazonWebServiceResult.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/UnreferencedParam.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include "CancelQueryResult.h"

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws;

CancelQueryResult::CancelQueryResult() {
}

CancelQueryResult::CancelQueryResult(
    const Aws::AmazonWebServiceResult< JsonValue >& result) {
    *this = result;
}

CancelQueryResult& CancelQueryResult::operator=(
    const Aws::AmazonWebServiceResult< JsonValue >& result) {
    JsonView jsonValue = result.GetPayload().View();
    if (jsonValue.ValueExists("CancellationMessage")) {
        m_cancellationMessage = jsonValue.GetString("CancellationMessage");
    }

    return *this;
}
