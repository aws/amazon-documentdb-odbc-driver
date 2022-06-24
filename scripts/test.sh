# store current directory
CURRENT_DIR=$(pwd)

#find correct path to JAVA_HOME
JAVA_HOME="what java_home should be"
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
    if [[ "$line" =~ "export JAVA_HOME=" ]]; then
        # do not write this line to new_zshrc

        # note: can split line by spaces and just skip the java home line
        line=$(echo $line | sed -e '/\#/!s/export JAVA_HOME=\".*\"//g')

        echo "this line is setting JAVA_HOME"
        echo "modified line: $line"
        change_zshrc=1
    fi

    if (line not whitespce) then
        # write this line to new_zshrc
        new_zshrc="$new_zshrc\n$line"
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

echo '\nexport JAVA_HOME="$JAVA_HOME"' >> $env_var_file
echo 'export PATH="$JAVA_HOME/lib/server/:$JAVA_HOME/bin:$PATH"' >> $env_var_file

# to append lines in zshrc (instead of re-writing the file):
# echo -n '\nactual new line in zshrc with no extra space' >> ~/.zshrc

# cd back to original directory
cd $CURRENT_DIR
