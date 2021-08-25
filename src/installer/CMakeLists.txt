#
#   Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License").
#   You may not use this file except in compliance with the License.
#   A copy of the License is located at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#   or in the "license" file accompanying this file. This file is distributed
#   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#   express or implied. See the License for the specific language governing
#   permissions and limitations under the License.
#

include(InstallRequiredSystemLibraries)
include(CPackComponent)

set(CMAKE_INSTALL_PREFIX ${INSTALL_ROOT})

# General package info
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

if(APPLE)
    set(CPACK_PACKAGE_FILE_NAME "AmazonTimestreamODBC-${CPACK_PACKAGE_VERSION}")
    set(CPACK_PACKAGE_NAME "Amazon Timestream ODBC Driver ${CPACK_PACKAGE_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
    set(CPACK_GENERATOR "productbuild")

    # This script will be run once the Driver component has finished installing.
    set(CPACK_POSTFLIGHT_DRIVER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/postinstall")

    # The productbuild generator copies files from this directory
    set(CPACK_PRODUCTBUILD_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")

    # Background setup to Distribution XML
    set(CPACK_PRODUCTBUILD_BACKGROUND "background.bmp")
    set(CPACK_PRODUCTBUILD_BACKGROUND_ALIGNMENT "bottomleft")
    set(CPACK_PRODUCTBUILD_BACKGROUND_SCALING "none")

    # Background setup for the Dark Aqua theme to Distribution XML
    set(CPACK_PRODUCTBUILD_BACKGROUND_DARKAQUA "background_darkaqua.bmp")
    set(CPACK_PRODUCTBUILD_BACKGROUND_DARKAQUA_ALIGNMENT "bottomleft")
    set(CPACK_PRODUCTBUILD_BACKGROUND_DARKAQUA_SCALING "none")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/Resources/README.txt")
    set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Welcome.txt")
elseif(UNIX)
    set(CPACK_PACKAGE_FILE_NAME "AmazonTimestreamODBC${BITNESS}-${CPACK_PACKAGE_VERSION}")
    if (INSTALLER_TYPE STREQUAL "DEB")
        if (BITNESS EQUAL 32)
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
        else()
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
        endif()
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_FILE_NAME "AmazonTimestreamODBC_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")

        # Deb installers fail if there's a space in them.
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "AmazonTimestreamODBCDriver${BITNESS}-bit")
        set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}${CPACK_PACKAGE_VERSION}")

        # TODO: Need to update both of these.
        set(CPACK_PACKAGE_CONTACT "lyndonb@bitquilltech.com")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lyndon Bauto")
        set(CPACK_PACKAGE_DESCRIPTION "AmazonTimestreamODBCDriver-${BITNESS}-bit")
        
        # Add postrm and postinst with dsn stuff to add and remove them.
        #set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst")

        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    elseif(INSTALLER_TYPE STREQUAL "RPM")
        if (BITNESS EQUAL 32)
            set(CPACK_RPM_PACKAGE_ARCHITECTURE i386)
        else()
            set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64)
        endif()
        set(CPACK_GENERATOR RPM)
        set(CPACK_RPM_FILE_NAME "AmazonTimestreamODBC_${CPACK_PACKAGE_VERSION}_${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")

        set(CPACK_PACKAGE_INSTALL_DIRECTORY "Amazon Timestream ODBC Driver ${BITNESS}-bit")
        set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${CPACK_PACKAGE_VERSION}")
        set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
        set(CPACK_RPM_COMPONENT_INSTALL ON)
        set(CPACK_TEMPORARY_PACKAGE_FILE_NAME "${CMAKE_BINARY_DIR}/_CPack_Packages/Linux/RPM/RPMS/${CMAKE_SYSTEM_PROCESSOR}/${CPACK_PACKAGE_FILE_NAME}.rpm")
    else()
        message(FATAL_ERROR "INSTALLER_TYPE '${INSTALLER_TYPE}' must be DEB or RPM for Linux installer." )
    endif()
else()
    set(CPACK_PACKAGE_FILE_NAME "AmazonTimestreamODBC${BITNESS}-${CPACK_PACKAGE_VERSION}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Amazon Timestream ODBC Driver")
    set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${BITNESS}-bit ${CPACK_PACKAGE_VERSION}")

    # Set generator to WIX
    set(CPACK_GENERATOR "WIX")

    # The bitmap will appear at the top of all installer pages other than the welcome and completion dialogs
    set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/banner.bmp")

    # Background bitmap used on the welcome and completion dialogs
    set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/dialog.bmp")

	if(BITNESS EQUAL 32)
		# This is a unique id for the installer - required for Windows
		# Generated at https://www.guidgen.com/
		set(CPACK_WIX_UPGRADE_GUID "b5076201-7e60-4be1-b098-49fabd619982")
		# This XML file is used for registry setup
		set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/patch.xml")
	else()
		# This is a unique id for the installer(x64) - required for Windows
		# Generated at https://www.guidgen.com/
		set(CPACK_WIX_UPGRADE_GUID "c4223f46-763a-4800-90d1-cc4fb64e9580")
		# This XML file is used for registry setup (x64)
		set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/patch64.xml")
	endif()

    # CPack doesn't allow extensionless licenses, need to make a copy with an extension, .txt is appropriate
    configure_file("${PROJECT_ROOT}/LICENSE" "${PROJECT_ROOT}/LICENSE.txt" COPYONLY)
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_VENDOR "Amazon")
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")

# CPack doesn't allow extensionless licenses, need to make a copy with an extension, .txt is appropriate
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_ROOT}/LICENSE.txt")
configure_file("${PROJECT_ROOT}/LICENSE" "${PROJECT_ROOT}/LICENSE.txt" COPYONLY)

# Set up components for installer
cpack_add_component(Docs
	DISPLAY_NAME "Documentation"
	DESCRIPTION "Documentation about Amazon Timestream ODBC Driver"
)
cpack_add_component(Driver
	DISPLAY_NAME "Driver"
	DESCRIPTION "Library files for running the Amazon Timestream ODBC Driver"
    REQUIRED 
)

# Install driver files
install(TARGETS timestreamsqlodbc DESTINATION bin COMPONENT "Driver")
# TODO: look into DSN Installer failure
# if(APPLE)
#     install(FILES "${PROJECT_ROOT}/bin/dsn_installer" DESTINATION bin COMPONENT "Driver")
#     install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/remove-dsn.sh" DESTINATION bin COMPONENT "Driver")
# endif()

# Install documentation files
install(FILES "${PROJECT_ROOT}/README.md" DESTINATION doc COMPONENT "Docs")
install(FILES "${PROJECT_ROOT}/LICENSE" DESTINATION doc COMPONENT "Docs")
install(FILES "${PROJECT_ROOT}/THIRD-PARTY" DESTINATION doc COMPONENT "Docs")

# Install AWS dependencies
if(WIN32)
    set(AWS_SDK_BIN_DIR "${PROJECT_ROOT}/build/aws-sdk/install/bin")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-crt-cpp.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-s3.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-auth.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-mqtt.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-event-stream.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-http.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-io.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-cal.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-compression.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-common.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-c-event-stream.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-checksums.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-cpp-sdk-core.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-cpp-sdk-sts.dll" DESTINATION bin COMPONENT "Driver")
    install(FILES "${AWS_SDK_BIN_DIR}/aws-cpp-sdk-timestream-query.dll" DESTINATION bin COMPONENT "Driver")
endif()

include(CPack)
