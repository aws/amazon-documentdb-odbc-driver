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

#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

int add( int i, int j ) { return i+j; }

BOOST_AUTO_TEST_CASE( my_test )
{
    // seven ways to detect and report the same error:
    BOOST_CHECK( add( 2,2 ) == 4 );        // #1 continues on error

    BOOST_REQUIRE( add( 2,2 ) == 4 );      // #2 throws on error

    if( add( 2,2 ) != 4 )
      BOOST_ERROR( "Ouch..." );            // #3 continues on error

    if( add( 2,2 ) != 4 )
      BOOST_FAIL( "Ouch..." );             // #4 throws on error

    if( add( 2,2 ) != 4 ) throw "Ouch..."; // #5 throws on error

    BOOST_CHECK_MESSAGE( add( 2,2 ) == 4,  // #6 continues on error
                         "add(..) result: " << add( 2,2 ) );

    BOOST_CHECK_EQUAL( add( 2,2 ), 4 );	  // #7 continues on error
}
