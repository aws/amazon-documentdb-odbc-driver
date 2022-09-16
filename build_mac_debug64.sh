BUILD_DIR=cmake-build64
BUILD_TYPE=Debug
PROJECT_DIR=$(pwd)
DRIVER_BIN_DIR="$PROJECT_DIR/build/odbc/bin"
DRIVER_LOG_DIR="$PROJECT_DIR/build/odbc/logs"

mkdir $DRIVER_LOG_DIR

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ../src -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCODE_COVERAGE="ON" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="ON" -DWITH_ODBC="ON"
cd ..

# Download the DocumentDB JDBC Driver
if [ -z "$JDBC_DRIVER_VERSION" ]; then 
    JDBC_DRIVER_VERSION="1.3.1"
fi
JDBC_DRIVER_FILENAME="documentdb-jdbc-$JDBC_DRIVER_VERSION-all.jar"
JDBC_DRIVER_FULLPATH="$DRIVER_BIN_DIR/libs/$JDBC_DRIVER_FILENAME"
export DOCUMENTDB_HOME="$DRIVER_BIN_DIR"
if [ ! -f "$JDBC_DRIVER_FULLPATH" ]; then
    mkdir "$DRIVER_BIN_DIR/libs"
    echo "Downloading version $JDBC_DRIVER_VERSION of JDBC driver..."
    curl -o "$DRIVER_BIN_DIR/libs/$JDBC_DRIVER_FILENAME" -L https://github.com/aws/amazon-documentdb-jdbc-driver/releases/download/v$JDBC_DRIVER_VERSION/$JDBC_DRIVER_FILENAME
    echo "Download complete." 
fi

#clean up the binary files used to generate code coverage report
find . -name "*.gcda" -type f -delete

cd cmake-build64
make -j 4
cd ..
