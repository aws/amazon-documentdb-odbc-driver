/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/client/AWSError.h>
#include "DatabaseQueryErrorMarshaller.h"
#include "DatabaseQueryErrors.h"

using namespace Aws::Client;

AWSError< CoreErrors > DatabaseQueryErrorMarshaller::FindErrorByName(
    const char* errorName) const {
    AWSError< CoreErrors > error =
        DatabaseQueryErrorMapper::GetErrorForName(errorName);
    if (error.GetErrorType() != CoreErrors::UNKNOWN) {
        return error;
    }

    return AWSErrorMarshaller::FindErrorByName(errorName);
}