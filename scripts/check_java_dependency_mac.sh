java_ver=temurin
if brew ls --versions $java_ver --cask > /dev/null; then
    # Java installed via brew
    echo "$java_ver is installed."
else
    # Java not installed via brew
    echo "$java_ver is not installed."
    java_not_installed=1
fi