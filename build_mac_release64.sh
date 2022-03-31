BUILD_DIR=cmake-build64
BUILD_TYPE=Release
PROJECT_DIR=$(pwd)
DRIVER_BIN_DIR="$PROJECT_DIR/build/odbc/bin"

mkdir $BUILD_DIR
cd $BUILD_DIR
if [[ -z "${DCMAKE_TOOLCHAIN_FILE}" ]]; then
  cmake ../src -DCMAKE_BUILD_TYPE="Debug" -DCODE_COVERAGE="ON" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="ON" -DWITH_CORE="OFF" -DWITH_ODBC="ON"
else
  cmake ../src -DCMAKE_BUILD_TYPE="Debug" -DCODE_COVERAGE="ON" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="ON" -DWITH_CORE="OFF" -DWITH_ODBC="ON" -DCMAKE_TOOLCHAIN_FILE="$DCMAKE_TOOLCHAIN_FILE"
fi
make -j 4

# Download the DocumentDB JDBC Driver
if [ -z "$JDBC_DRIVER_VERSION" ]; then 
    JDBC_DRIVER_VERSION="1.2.0"
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

cd ..
