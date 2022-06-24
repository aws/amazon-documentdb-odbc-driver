# store current directory
CURRENT_DIR=$(pwd)

#find correct path to java_home_path
java_home_path="/Library/Java/JavaVirtualMachines/temurin-18.jdk/Contents/Home" 
java_server_path="$java_home_path/lib/server/"
java_bin_path="$java_home_path/bin"
set_java_home="export JAVA_HOME=\"$java_home_path\""
set_java_bin_path="export PATH=\"$java_bin_path:\$PATH\""
set_java_server_path="export PATH=\"$java_server_path:\$PATH\""

cd ~
env_var_file=".zshrc"
updated=0

# check env vars
if [[ "${JAVA_HOME}" != "${java_home_path}" ]]; then
    echo "JAVA_HOME is not set properly. Appending definition of JAVA_HOME to end of ${env_var_file}"
    echo "$set_java_home" >> $env_var_file
    echo "JAVA_HOME has been set"
    updated=1
fi

if [[ ! :$PATH: == *:"$java_server_path":* ]] ; then
    echo "setting JAVA server path"
    echo "$set_java_server_path" >> $env_var_file
    echo "JAVA server path has been set"
    updated=1
fi

if [[ ! :$PATH: == *:"$java_bin_path":* ]] ; then
    echo "setting JAVA bin path"
    echo "$set_java_bin_path" >> $env_var_file
    echo "JAVA bin path has been set"
    updated=1
fi

# cd back to original directory
cd $CURRENT_DIR

if [[ ${updated} -eq "0" ]]; then
    echo "JAVA_HOME, JAVA bin path, and JAVA server path are set properly. No changes made to ~/$env_var_file"
else
    echo "~/$env_var_file has been updated to set environment variables properly"
fi