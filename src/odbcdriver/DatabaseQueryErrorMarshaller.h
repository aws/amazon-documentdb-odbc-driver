/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once

#include <aws/core/client/AWSErrorMarshaller.h>

class DatabaseQueryErrorMarshaller : public Aws::Client::JsonErrorMarshaller {
   public:
    Aws::Client::AWSError< Aws::Client::CoreErrors > FindErrorByName(
        const char* exceptionName) const override;
};