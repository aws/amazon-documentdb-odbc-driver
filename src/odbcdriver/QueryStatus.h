/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
namespace Aws {
namespace Utils {
namespace Json {
class JsonValue;
class JsonView;
}  // namespace Json
}  // namespace Utils
}  // namespace Aws

/**
 * <p>Information about the status of the query, including progress and bytes
 * scannned.</p><p><h3>See Also:</h3>   <a
 * href="http://docs.aws.amazon.com/goto/WebAPI/timestream-query-2018-11-01/QueryStatus">AWS
 * API Reference</a></p>
 */
class QueryStatus {
   public:
    QueryStatus();
    QueryStatus(Aws::Utils::Json::JsonView jsonValue);
    QueryStatus& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;

    /**
     * <p>The progress of the query, expressed as a percentage.</p>
     */
    inline double GetProgressPercentage() const {
        return m_progressPercentage;
    }

    /**
     * <p>The progress of the query, expressed as a percentage.</p>
     */
    inline bool ProgressPercentageHasBeenSet() const {
        return m_progressPercentageHasBeenSet;
    }

    /**
     * <p>The progress of the query, expressed as a percentage.</p>
     */
    inline void SetProgressPercentage(double value) {
        m_progressPercentageHasBeenSet = true;
        m_progressPercentage = value;
    }

    /**
     * <p>The progress of the query, expressed as a percentage.</p>
     */
    inline QueryStatus& WithProgressPercentage(double value) {
        SetProgressPercentage(value);
        return *this;
    }

   private:
    double m_progressPercentage;
    bool m_progressPercentageHasBeenSet;
};