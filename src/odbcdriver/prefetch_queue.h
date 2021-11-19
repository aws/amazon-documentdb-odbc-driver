/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */
#ifndef PREFETCH_QUEUE
#define PREFETCH_QUEUE

#include <queue>
#include <future>
#include "DatabaseQueryClient.h"

/**
 * PrefetchQueue class for query
 */
class PrefetchQueue {
   public:
    /**
     * The max capacity of the queue
     */
    static constexpr int CAPACITY = 2;
    /**
     * Push function
     * @param future_outcome std::shared_future< Aws::DatabaseQuery::Model::QueryOutcome >
     * @return bool
     */
    bool Push(std::shared_future< QueryOutcome > future_outcome);
    /**
     * Pop function
     * @return void
     */
    void Pop();
    /**
     * Front function
     * @return Aws::DatabaseQuery::Model::QueryOutcome
     */
    QueryOutcome Front();
    /**
     * Check the readiness of the front
     * It could be interrupted by if the caller cancels
     * @return bool
     */
    bool WaitForReadinessOfFront();
    /**
     * IsEmpty function
     * @return bool
     */
    bool IsEmpty();
    /**
     * Reset the queue
     * @return void
     */
    void Reset();
    /**
     * Set retrieving
     * @param retrieving_ bool
     * @return void
     */
    void SetRetrieving(bool retrieving_);
    /**
     * Is retrieving
     * @return bool
     */
    bool IsRetrieving();
    /**
     * Notify one thread that something is changed
     * @return void
     */
    void NotifyOne();
	
   private:
    /**
     * The queue storing the future object
     * If future object is not ready, it means AWS is still processing the corresponding query request
     */
    std::queue< std::shared_future< QueryOutcome > > queue;
    /**
     * If true, it is in retrieving state
     */
    bool retrieving;
    /**
     * Mutex for protection
     */
    std::mutex mutex;
    /**
     * Conditional variable for signalling
     */
    std::condition_variable condition_variable;
};
#endif
