# Build AWS SDK
$CURRENT_DIR = Get-Location
$WORKING_DIR = $args[0]
$CONFIGURATION = $args[1]
$BITNESS = $args[2]
if ($BITNESS -eq "64") {
    $WIN_ARCH = "x64"
}
else {
    $WIN_ARCH = "Win32"
}

# Create build directory; remove if exists
$BUILD_DIR = "${WORKING_DIR}\build"
# $BUILD_DIR = "${WORKING_DIR}\build\${CONFIGURATION}${BITNESS}"
New-Item -Path $BUILD_DIR -ItemType Directory -Force | Out-Null


$DRIVER_SOURCE_DIR = "${WORKING_DIR}\src"
$DRIVER_BUILD_DIR = "${BUILD_DIR}\odbc\cmake"

# Download the JDBC driver
$JDBC_DRIVER_VERSION = if ($JDBC_DRIVER_VERSION -eq $null) { "1.3.0" } else { $JDBC_DRIVER_VERSION }
$JDBC_DRIVER_FILENAME = "documentdb-jdbc-$JDBC_DRIVER_VERSION-all.jar"
$JDBC_DRIVER_FULLPATH = "$DRIVER_BUILD_DIR\$CONFIGURATION\libs\$JDBC_DRIVER_FILENAME"
if (-not (Test-Path -Path $JDBC_DRIVER_FULLPATH -PathType Leaf)) {
    New-Item -Path "$DRIVER_BUILD_DIR\$CONFIGURATION\libs" -ItemType Directory -Force | Out-Null
    Write-Output "Downloading version $JDBC_DRIVER_VERSION of JDBC driver..."
    $progresspreference = 'silentlyContinue'
    Invoke-WebRequest `
        https://github.com/aws/amazon-documentdb-jdbc-driver/releases/download/v$JDBC_DRIVER_VERSION/$JDBC_DRIVER_FILENAME `
        -o $JDBC_DRIVER_FULLPATH
    $progressPreference = 'Continue'
    Write-Output "Download complete." 
}

# Build driver

.\scripts\build_driver.ps1 `
    $CONFIGURATION $WIN_ARCH `
    $DRIVER_SOURCE_DIR $DRIVER_BUILD_DIR $SDK_INSTALL_DIR
Set-Location $CURRENT_DIR

# Move driver dependencies to bin directory for testing
$DRIVER_BIN_DIR = "$DRIVER_BUILD_DIR\..\bin\$CONFIGURATION"
New-Item -Path $DRIVER_BIN_DIR -ItemType Directory -Force | Out-Null


if (Test-Path -Path $DRIVER_BUILD_DIR\$CONFIGURATION) {
    Copy-Item $DRIVER_BUILD_DIR\$CONFIGURATION\* $DRIVER_BIN_DIR -force -recurse
}
