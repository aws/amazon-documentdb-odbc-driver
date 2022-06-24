# store current directory
CURRENT_DIR=$(pwd)

#find correct path to JAVA_HOME
JAVA_HOME="what java_home should be"
set_java_home="export JAVA_HOME=\"$JAVA_HOME\""
set_path="export PATH=\"$JAVA_HOME/lib/server/:$JAVA_HOME/bin:\$PATH"

new_zshrc=""

cd ~
env_var_file=".zshrc_temp"
# file=".zshrc"
  
# Use IFS= to keep line indentations. 
# assumes that env_var_file may or may not end with a new line

i=1  
while IFS= read -r line || [ -n "$line" ]; do  
  
    #Reading each line  
    echo "Line No. $i : $line"

    # first alternative
    # if (java home has not been not set and line is setting java home)
    #     set java home by modifying line

    # append line to new_zshrc

    # second alternative

    # check that the line exports JAVA_HOME and is not a comment

    #todo Fix required
    # note: this check would think "export JAVA_HOME="xxxx" # export JAVA_HOME" sets JAVA_HOME but is a comment
    if [[ "$line" =~ "#.*export JAVA_HOME=" ]] || [[ "$line" =~ "#export JAVA_HOME=" ]]; then
        echo "this line sets JAVA_HOME but is a comment"
    fi

    if [[ "$line" =~ "export JAVA_HOME=" ]] && ! [[ "$line" =~ "#.*export JAVA_HOME=" ]] && ! [[ "$line" =~ "#export JAVA_HOME=" ]]; then
        # modify this line before writing to new_zshrc

        # removes instances of export JAVA_HOME="anything", only if there is no # symbol before that expression in the entore string
        line=$(echo $line | sed -e '/\#/!s/export JAVA_HOME=\".*\"//g')

        echo "this line is setting JAVA_HOME"
        echo "modified line: $line"
        change_zshrc=1
    fi


    # remove duplicate of $set_path
    # the search pattern is same as set_path except $var becomes '"${var}"', and all symbols are escaped (e.g., / becomes \/, : becomes \:)
    line=$(echo $line | sed -e 's/export PATH=\"'"${JAVA_HOME}"'\/lib\/server\/\:'"${JAVA_HOME}"'\/bin:\$PATH//g')

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

# first alternative
# if (java home set (means we changed java home value))
#     echo "$new_zshrc" > ~/.zshrc

# second alternative
# if (change_zshrc)
#     echo "$new_zshrc" > ~/.zshrc
# end
# echo -n '\nexport JAVA_HOME="$JAVA_HOME"' >> ~/.zshrc
# echo -n 'export PATH="$JAVA_HOME/lib/server/:$JAVA_HOME/bin:$PATH"' >> ~/.zshrc

if [[ "${change_zshrc}" -eq "1" ]]; then
    echo "re-writing ${env_var_file}"
    echo "$new_zshrc" > $env_var_file
fi

echo "writing new lines to ${env_var_file}"

echo "$set_java_home" >> $env_var_file
echo "$set_path" >> $env_var_file

# echo 'export JAVA_HOME="${JAVA_HOME}"' >> $env_var_file
# echo 'export PATH="${JAVA_HOME}/lib/server/:${JAVA_HOME}/bin:$PATH"' >> $env_var_file

# to append lines in zshrc (instead of re-writing the file):
# echo -n '\nactual new line in zshrc with no extra space' >> ~/.zshrc

# cd back to original directory
cd $CURRENT_DIR
