# TODO: [Automate setting up environment variables when building driver on Linux](https://github.com/aws/amazon-documentdb-odbc-driver/issues/173)
# store current directory
CURRENT_DIR=$(pwd)

#find correct path to java_home_path
#java_home_path="/Library/Java/JavaVirtualMachines/temurin-18.jdk/Contents/Home" 
java_home_path="/path/to/java/home"
java_server_path="$java_home_path/lib/server/"
java_bin_path="$java_home_path/bin"
set_java_home="export JAVA_HOME=\"$java_home_path\""
set_java_bin_path="export PATH=\"$java_bin_path:\$PATH\""
set_java_server_path="export PATH=\"$java_server_path:\$PATH\""
# set_path="export PATH=\"$java_server_path:$java_bin_path:\$PATH\""

new_zshrc=""

cd ~
env_var_file=".bashrc"
# file=".zshrc"


java_home_set=1
# check enviornment variables
if [[ "${JAVA_HOME}" != "${java_home_path}" ]]; then
    echo "JAVA_HOME is not set properly. Removing previous definitions to JAVA_HOME and give new definition"
    java_home_set=0
fi
  
# Use IFS= to keep line indentations. 
# assumes that env_var_file may or may not end with a new line

i=1  
while IFS= read -r line || [ -n "$line" ]; do  
  
    #Reading each line  
    echo "Line No. $i : $line"

    # check if line is a comment. 
    if [[ ! "$line" =~ ^[[:space:]]*\#.* ]]; then
        echo "TRACE - this line is not a comment, proceeding"


        # check that the line exports JAVA_HOME
        if [[ "${java_home_set}" == "0"  ]] && [[ "$line" =~ "export JAVA_HOME=" ]]; then
            # modify this line before writing to new_zshrc

            # removes instances of export JAVA_HOME="anything", except comments
            line=$(echo $line | sed -e 's/(?<!(^[[:space:]]*\#.*))(export JAVA_HOME=\".*?\")//g')

            echo "this line is setting JAVA_HOME"
            echo "modified line: $line"
            change_zshrc=1
        fi

    fi

    # remove duplicate of $set_path
    # but this is not needed. We just need to make sure we export the path when we don't have it -AL-
    # the search pattern is same as set_path except $var becomes '"${var}"', and all symbols are escaped (e.g., / becomes \/, : becomes \:)
    # line=$(echo $line | sed -e 's/export PATH=\"'"${JAVA_HOME}"'\/lib\/server\/\:'"${JAVA_HOME}"'\/bin:\$PATH\"//g')

    # if line is not white space
    if [[ $line =~ [^[:space:]] ]]; then
        echo "line is not whitespace"
        # write this line to new_zshrc

        # do not write new line if it is the first non-empty line
        if [[ ! $new_zshrc =~ [^[:space:]] ]]; then
            new_zshrc="$new_zshrc$line"
        else
            new_zshrc="$new_zshrc\n$line"
        fi
    fi

    i=$((i+1))  
done < $env_var_file  

if [[ "${change_zshrc}" -eq "1" ]]; then
    echo "re-writing ${env_var_file}"
    echo "$new_zshrc" > $env_var_file
fi

echo "writing new lines to ${env_var_file}"

if [[ "${java_home_set}" == "0"  ]]; then
    echo "$set_java_home" >> $env_var_file
    java_home_set=1
fi

if [[ ! :$PATH: == *:"$java_server_path":* ]] ; then
    echo "setting JAVA server path"
    echo "$set_java_server_path" >> $env_var_file
fi

if [[ ! :$PATH: == *:"$java_bin_path":* ]] ; then
    echo "setting JAVA bin path"
    echo "$set_java_bin_path" >> $env_var_file
fi

# cd back to original directory
cd $CURRENT_DIR
