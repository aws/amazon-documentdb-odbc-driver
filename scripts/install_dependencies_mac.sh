# get the current script directory
PROJECT_DIR=$(pwd)
SCRIPT_DIR="$PROJECT_DIR/scripts"

# install dependencies via brew
brew tap homebrew/services

# unlink unix ODBC driver manager
brew unlink unixodbc

echo "TRACE calling check_dependencies_mac.sh"

source $SCRIPT_DIR/check_dependencies_mac.sh

echo ${apps_installed[*]}                   

# install iODBC driver manager
brew install libiodbc
brew install cmake
brew install openssl
brew install boost
brew install mongo-cxx-driver
