

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_DIR="$SCRIPT_DIR/.."
ODBC_LIB_PATH="$PROJECT_DIR/build/odbc/lib"
ODBC_LIB_FILENAME="$ODBC_LIB_PATH/libignite-odbc.dylib"

if [ ! -f "$ODBC_LIB_FILENAME" ]then
	echo "Cannot find ODBC library file: $ODBC_LIB_FILENAME"
	exit 1
fi

echo "[Apache Ignite]"            > "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "Description=Apache Ignite" >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "Driver=$ODBC_LIB_FILENAME" >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "Setup=$ODBC_LIB_FILENAME"  >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "DriverODBCVer=03.00"       >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
echo "FileUsage=0"               >> "$ODBC_LIB_PATH/ignite-odbc-install.ini"
odbcinst -i -d                   -f "$ODBC_LIB_PATH/ignite-odbc-install.ini"
