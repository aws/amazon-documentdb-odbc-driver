/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DOCUMENTDB_ODBC_DRIVER_INSTANCE
#define _DOCUMENTDB_ODBC_DRIVER_INSTANCE

#include <cstdlib>
#include <memory>

#include <bsoncxx/stdx/make_unique.hpp>
#include <bsoncxx/stdx/optional.hpp>
#include <bsoncxx/stdx/string_view.hpp>

#include <mongocxx/instance.hpp>
#include <mongocxx/logger.hpp>


class DriverInstance {
   public:
    static DriverInstance& instance() {
        static DriverInstance instance;
        return instance;
    }

    void initializeInstance() {
        if (_instance == nullptr) {

            class noop_logger : public mongocxx::logger {
                public:
                    virtual void operator()(mongocxx::log_level,
                                            bsoncxx::stdx::string_view,
                                            bsoncxx::stdx::string_view) noexcept {}
            };
            _instance = bsoncxx::stdx::make_unique<mongocxx::instance>(bsoncxx::stdx::make_unique<noop_logger>());
        }
    }

   private:
    DriverInstance() {
        initializeInstance();
    }

    std::unique_ptr<mongocxx::instance> _instance = nullptr;
};

#endif //_DOCUMENTDB_ODBC_DRIVER_INSTANCE
