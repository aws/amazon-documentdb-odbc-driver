#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This script installs all required dependencies on Mac and sets the appropriate vars

# get the current script directory
PROJECT_DIR=$(pwd)
MAC_SCRIPT_DIR="$PROJECT_DIR/scripts/mac"

# install formula dependencies via brew
brew tap homebrew/services

# unlink unix ODBC driver manager
if brew ls --versions unixodbc > /dev/null; then
    echo "unixodbc is installed via brew. unlink unixodbc"
    brew unlink unixodbc
fi
# check if dependnecies are installed

# check if user is under the DocDB ODBC repository
if [[ ! -f "$MAC_SCRIPT_DIR/check_dependencies_mac.sh" ]]; then
    echo "Cannot find file $MAC_SCRIPT_DIR/check_dependencies_mac.sh, please check that you're under the DocumentDB ODBC repository before executing this script"
    exit 1
fi

chmod +x $MAC_SCRIPT_DIR/check_dependencies_mac.sh
source $MAC_SCRIPT_DIR/check_dependencies_mac.sh
# num_apps, apps_installed and req_apps are variables from check_dependencies_mac.sh

if [[ "${missing_formula}" -eq "0" ]]; then
    echo "Most required dependencies are installed. Checking if JAVA is installed."
else
    echo "Installing the missing required dependencies"
    for (( i=0 ; i<$num_apps ; i++ )); 
    do
    if [[ "${apps_installed[i]}" -eq "0" ]]; then
        echo  "${req_apps[i]} is not installed, attempt to install it with brew."
        brew install ${req_apps[i]}
    fi
    done
fi

# [Re-]Install mongodb server
# TODO [AD-833] make re-installing mongodb server optional
# https://bitquill.atlassian.net/browse/AD-833
chmod +x $MAC_SCRIPT_DIR/reinstall_mongodb_mac.sh
$MAC_SCRIPT_DIR/reinstall_mongodb_mac.sh

# check if java dependency is installed
chmod +x $MAC_SCRIPT_DIR/check_java_dependency_mac.sh
source $MAC_SCRIPT_DIR/check_java_dependency_mac.sh
# jdkFound is a variable from check_java_dependency_mac.sh

java_ver=corretto
if [[ "${jdkFound}" -ne "1" ]]; then
    echo "$java_ver is not installed, attempt to install it with brew."
    # install JAVA dependency via brew
    brew tap homebrew/cask-versions
    brew install --cask $java_ver
fi

# set JAVA_HOME and save JAVA /bin and JAVA /server paths to $PATH
chmod +x $MAC_SCRIPT_DIR/set_java_def_mac.sh
source $MAC_SCRIPT_DIR/set_java_def_mac.sh

# add llvm to path
chmod +x $MAC_SCRIPT_DIR/set_llvm_mac.sh
source $MAC_SCRIPT_DIR/set_llvm_mac.sh

echo "The mac dependency installation script has finshed running."
