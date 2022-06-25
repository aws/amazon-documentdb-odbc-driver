# this script installs all required dependencies on Mac and sets the appropriate vars

# TODO -AL-: may need to write to zshrc file for permanent vars storing

# get the current script directory
PROJECT_DIR=$(pwd)
SCRIPT_DIR="$PROJECT_DIR/scripts"

# install formula dependencies via brew
brew tap homebrew/services

# unlink unix ODBC driver manager
brew unlink unixodbc

echo "TRACE calling check_dependencies_mac.sh"

# check if dependnecies are installed
chmod +x $SCRIPT_DIR/check_dependencies_mac.sh
source $SCRIPT_DIR/check_dependencies_mac.sh
# num_apps, apps_installed and req_apps are variables from check_dependencies_mac.sh

# -AL- TRACE messages, to be removed later
echo "TRACE - apps_installed: ${apps_installed[*]}"
echo "TRACE - req_apps: ${req_apps[*]}"

if [[ "${missing_formula}" -eq "0" ]]; then
    echo "All required dependencies are installed"
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

echo "TRACE calling check_java_dependency_mac.sh"

# check if java dependency is installed
chmod +x $SCRIPT_DIR/check_java_dependency_mac.sh
source $SCRIPT_DIR/check_java_dependency_mac.sh
# java_ver and java_not_installed are variables from check_dependencies_mac.sh

if [[ "${java_not_installed}" -eq "1" ]]; then
    echo "$java_ver is not installed, attempt to install it with brew."
    # install JAVA dependency via brew
    brew tap homebrew/cask-versions
    brew install --cask $java_ver

    # set JAVA_HOME and save JAVA /bin and JAVA /server paths to $PATH
    chmod +x $SCRIPT_DIR/set_java_def_mac.sh
    source $SCRIPT_DIR/set_java_def_mac.sh
fi

# add llvm to path
chmod +x $SCRIPT_DIR/set_llvm_mac.sh
source $SCRIPT_DIR/set_llvm_mac.sh
