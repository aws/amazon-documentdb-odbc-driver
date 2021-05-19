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
#include "ts_prefetch_queue.h"


void PrefetchQueue::Push(std::shared_future< Aws::TimestreamQuery::Model::QueryOutcome > future_outcome) {
	q.push(std::move(future_outcome));
}

void PrefetchQueue::Pop() {
	q.pop();
}

Aws::TimestreamQuery::Model::QueryOutcome PrefetchQueue::Front() {
	auto outcome = q.front().get();
	return outcome;
}

bool PrefetchQueue::FrontIsReady() {
    auto status = q.front().wait_for(std::chrono::milliseconds(1));
    return status == std::future_status::ready;	
}

bool PrefetchQueue::IsEmpty() {
	return q.empty();
}

void PrefetchQueue::Clear() {
	while (!q.empty()) {
		q.pop();
	}
}
