

# install dependencies via brew
brew tap homebrew/services

# unlink unix ODBC driver manager
brew unlink unixodbc

# install iODBC driver manager
brew install libiodbc
brew install cmake
brew install openssl
brew install boost
brew install mongo-cxx-driver
