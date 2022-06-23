# get the current script directory
PROJECT_DIR=$(pwd)
SCRIPT_DIR="$PROJECT_DIR/scripts"

# install dependencies via brew
brew tap homebrew/services

# unlink unix ODBC driver manager
brew unlink unixodbc

echo "TRACE calling check_dependencies_mac.sh"

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
