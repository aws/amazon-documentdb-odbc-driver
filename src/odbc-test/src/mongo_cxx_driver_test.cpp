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

#define BOOST_TEST_MODULE MongoCXXDriverTest
#include <boost/test/unit_test.hpp>
#include <cstdlib>
#include <iostream>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

#define BOOST_TEST_MODULE MongoCXXDriverTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( mongo_cxx_driver_connect_test )
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    // The mongocxx::instance constructor and destructor initialize and shut down the driver,
    // respectively. Therefore, a mongocxx::instance must be created before using the driver and
    // must remain alive for as long as the driver is in use.
    mongocxx::instance inst;

    try {
        const auto uri = mongocxx::uri{"mongodb://documentdb:bqdocumentdblab@127.0.0.1:27017/?tls=true&tlsCAFile=~/.ssh/rds-combined-ca-bundle.pem&tlsAllowInvalidHostnames=true"};

        auto client = mongocxx::client{uri};
        auto database = client["test"];
        auto collection = database["test"];
        mongocxx::cursor cursor = collection.find({});
        for(auto doc : cursor) {
            std::cout << bsoncxx::to_json(doc) << "\n";
        }
        // auto result = test.run_command(make_document(kvp("isMaster", 1)));
        // std::cout << bsoncxx::to_json(result) << std::endl;

        return EXIT_SUCCESS;
    } catch (const std::exception& xcp) {
        std::cout << "connection failed: " << xcp.what() << std::endl;
        return EXIT_FAILURE;
    }
}

