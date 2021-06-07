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
#include "prefetch_queue.h"

bool PrefetchQueue::Push(std::shared_future< Aws::TimestreamQuery::Model::QueryOutcome > future_outcome) {
    std::unique_lock< std::mutex > lock(mutex);
    condition_variable.wait(
        lock, [&]() { return queue.size() < CAPACITY || !retrieving; });
    if (retrieving) {
        queue.push(std::move(future_outcome));
        return true;
    } else {
        return false;
    }
}

void PrefetchQueue::Pop() {
    {
        std::unique_lock< std::mutex > lock(mutex);
        queue.pop();
        if (queue.empty()) {
            retrieving = false;
        }
    }
    condition_variable.notify_one();
}

Aws::TimestreamQuery::Model::QueryOutcome PrefetchQueue::Front() {
    auto outcome = queue.front().get();
    return outcome;
}

bool PrefetchQueue::WaitForReadinessOfFront() {
    std::unique_lock< std::mutex > lock(mutex);
    condition_variable.wait(lock, [&]() { 
        return std::future_status::ready
                   == queue.front().wait_for(std::chrono::milliseconds(1))
            || !retrieving;
    });
    return retrieving;
}

bool PrefetchQueue::IsEmpty() {
    return queue.empty();
}

void PrefetchQueue::Reset() {
    std::unique_lock< std::mutex > lock(mutex);
    while (!queue.empty()) {
        queue.pop();
	}
    retrieving = false;
    condition_variable.notify_one();
}

void PrefetchQueue::SetRetrieving(bool retrieving_) {
    {
        std::unique_lock< std::mutex > lock(mutex);
        retrieving = retrieving_;
    }
    condition_variable.notify_one();
}

bool PrefetchQueue::IsRetrieving() {
    return retrieving;
}

void PrefetchQueue::NotifyOne() {
    condition_variable.notify_one();
}
