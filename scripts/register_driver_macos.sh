
PROJECT_DIR=$(pwd)
ODBC_LIB_PATH=$PROJECT_DIR/build/odbc/lib
echo "[Apache Ignite]"                              > "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "Description=Apache Ignite"                   >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "Driver=$ODBC_LIB_PATH/libignite-odbc.dylib"  >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "Setup=$ODBC_LIB_PATH/libignite-odbc.dylib"   >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "DriverODBCVer=03.00"                         >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "FileUsage=0"                                 >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
odbcinst -i -d                                     -f "$ODBC_LIB_PATH/ignite-odbc-install.ini"