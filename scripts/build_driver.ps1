$CONFIGURATION = $args[0]
$WIN_ARCH = $args[1]
$SRC_DIR = $args[2]
$BUILD_DIR = $args[3]
$INSTALL_DIR = $args[4]

if ($WIN_ARCH = "x64") {
    $VCPKG_TARGET_TRIPLET="x64-windows"
} else {
    $VCPKG_TARGET_TRIPLET="x86-windows"
}

cmake -S $SRC_DIR `
    -B $BUILD_DIR `
    -A $WIN_ARCH `
    -D CMAKE_BUILD_TYPE=$CONFIGURATION `
    -D CMAKE_INSTALL_PREFIX=$INSTALL_DIR `
    -D WITH_TESTS=ON `
    -D WITH_CORE=OFF `
    -D WITH_ODBC=ON `
    -D WITH_ODBC_MSI=ON `
    -D VCPKG_TARGET_TRIPLET=$VCPKG_TARGET_TRIPLET `
    -D CMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

if ($?) {
    # # Build Project
    cmake --build $BUILD_DIR --target install --config $CONFIGURATION --parallel 4 
}
