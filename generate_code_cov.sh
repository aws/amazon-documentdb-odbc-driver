export BOOST_TEST_CATCH_SYSTEM_ERRORS=no

cd cmake-build64
if [ -n "$RUN_CODE_COVERAGE" ]; then
    make ccov-all -j 4
fi
cd ..
