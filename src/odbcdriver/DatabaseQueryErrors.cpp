/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/client/AWSError.h>
#include <aws/core/utils/HashingUtils.h>
#include "DatabaseQueryErrors.h"

using namespace Aws::Client;
using namespace Aws::Utils;

namespace DatabaseQueryErrorMapper
{

static const int INVALID_ENDPOINT_HASH = HashingUtils::HashString("InvalidEndpointException");
static const int CONFLICT_HASH = HashingUtils::HashString("ConflictException");
static const int INTERNAL_SERVER_HASH = HashingUtils::HashString("InternalServerException");
static const int QUERY_EXECUTION_HASH = HashingUtils::HashString("QueryExecutionException");


AWSError<CoreErrors> GetErrorForName(const char* errorName)
{
  int hashCode = HashingUtils::HashString(errorName);

  if (hashCode == INVALID_ENDPOINT_HASH)
  {
    return AWSError<CoreErrors>(static_cast<CoreErrors>(DatabaseQueryErrors::INVALID_ENDPOINT), false);
  }
  else if (hashCode == CONFLICT_HASH)
  {
    return AWSError<CoreErrors>(static_cast<CoreErrors>(DatabaseQueryErrors::CONFLICT), false);
  }
  else if (hashCode == INTERNAL_SERVER_HASH)
  {
    return AWSError<CoreErrors>(static_cast<CoreErrors>(DatabaseQueryErrors::INTERNAL_SERVER), false);
  }
  else if (hashCode == QUERY_EXECUTION_HASH)
  {
    return AWSError<CoreErrors>(static_cast<CoreErrors>(DatabaseQueryErrors::QUERY_EXECUTION), false);
  }
  return AWSError<CoreErrors>(CoreErrors::UNKNOWN, false);
}

} // namespace DatabaseQueryErrorMapper