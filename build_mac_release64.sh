
BUILD_DIR=cmake-build64
BUILD_TYPE=Release
DRIVER_BIN_DIR=$BUILD_DIR/$BUILD_TYPE

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ../src -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCODE_COVERAGE="OFF" -DBUILD_SHARED_LIBS="OFF" -DWITH_TESTS="ON" -DWITH_CORE="OFF" -DWITH_ODBC="ON"
make -j 4

# Download the DocumentDB JDBC Driver
# Download the JDBC driver
$JDBC_DRIVER_VERSION = "1.1.0"
$JDBC_DRIVER_FILENAME = "documentdb-jdbc-$JDBC_DRIVER_VERSION-all.jar"
$JDBC_DRIVER_FULLPATH = "$DRIVER_BIN_DIR/libs/$JDBC_DRIVER_FILENAME"
if [ ! -f "$JDBC_DRIVER_FULLPATH" ]; then
    mkdir "$DRIVER_BIN_DIR/libs"
    echo "Downloading version $JDBC_DRIVER_VERSION of JDBC driver..."
    curl -o "$JDBC_DRIVER_FULLPATH" https://github.com/aws/amazon-documentdb-jdbc-driver/releases/download/v$JDBC_DRIVER_VERSION/$JDBC_DRIVER_FILENAME
    echo "Download complete." 
fi

cd ..
