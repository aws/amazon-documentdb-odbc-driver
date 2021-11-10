/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/utils/json/JsonSerializer.h>
#include "CancelQueryRequest.h"

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

CancelQueryRequest::CancelQueryRequest() : m_queryIdHasBeenSet(false) {
}

std::string CancelQueryRequest::SerializePayload() const {
    JsonValue payload;

    if (m_queryIdHasBeenSet) {
        payload.WithString("QueryId", m_queryId);
    }

    return payload.View().WriteReadable();
}

Aws::Http::HeaderValueCollection CancelQueryRequest::GetRequestSpecificHeaders()
    const {
    Aws::Http::HeaderValueCollection headers;
    headers.insert(Aws::Http::HeaderValuePair(
        "X-Amz-Target", "Database_20181101.CancelQuery"));
    return headers;
}