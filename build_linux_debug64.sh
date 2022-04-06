BUILD_DIR=cmake-build64
BUILD_TYPE=Debug
PROJECT_DIR=$(pwd)
DRIVER_BIN_DIR="$PROJECT_DIR/build/odbc/bin"

mkdir cmake-build64
cd cmake-build64
cmake ../src -DCMAKE_BUILD_TYPE="Debug" -DCODE_COVERAGE="OFF" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="ON" -DWITH_CORE="OFF" -DWITH_ODBC="ON"
cd ..

# Download the DocumentDB JDBC Driver
if [ -z "$JDBC_DRIVER_VERSION" ]; then 
# Reminder: update Github action cache key as well
    JDBC_DRIVER_VERSION="1.2.1"
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

cd cmake-build64
make -j 4
cd ..
