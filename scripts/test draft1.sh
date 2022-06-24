# store current directory
CURRENT_DIR=$(pwd)

#find correct path to JAVA_HOME
$JAVA_HOME="what java_home should be"

cd ~
file=".zshrc_temp"
# file=".zshrc"
  
i=1  
while read line; do  
  
#Reading each line  
echo "Line No. $i : $line"

# first alternative
# if (java home has not been not set and line is setting java home)
#     set java home by modifying line

# append line to new_zshrc

# second alternative
if (line is setting java home)
    # do not write this line to new_zshrc
    change_zshrc=1
else 
    # write this line to new_zshrc

i=$((i+1))  
done < $file  

# first alternative
# if (java home set (means we changed java home value))
#     echo "$new_zshrc" > ~/.zshrc

# second alternative
# if (change_zshrc)
#     echo "$new_zshrc" > ~/.zshrc
# end
# echo -n '\nexport JAVA_HOME="$JAVA_HOME"' >> ~/.zshrc
# echo -n 'export PATH="$JAVA_HOME/lib/server/:$JAVA_HOME/bin:$PATH"'' >> ~/.zshrc

# to append lines in zshrc (instead of re-writing the file):
# echo -n '\nactual new line in zshrc with no extra space' >> ~/.zshrc

# cd back to original directory
cd $CURRENT_DIR
