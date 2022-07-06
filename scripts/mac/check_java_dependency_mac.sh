java_ver=corretto

# todo -AL- check if JAVA_HOME is set
# TODO change set_java_def_mac.sh as well, as we don;t need to check java_home twice. 
# Checks that JAVA_HOME is defined, the path to JAVA_HOME exists 
# and is a JDK (i.e., include subdirectory)
if [[ ! -z "${JAVA_HOME}" ]] && [[ -d "${JAVA_HOME}" ]] && [[ -d "${JAVA_HOME}/include" ]]; then
    # Test Java in path and has minimum version
#   -AL- e.g., openjdk 18 2022-03-22
    javaVersionString=`echo "${$(java --version)}" | head -1` # works in terminal but not in .sh 
    # IFS=';' read -ra javaVersionStringArr <<< $javaVersionString # invalid -a option "read: bad option: -a"
# In order to read in an array in zsh, read requires the option -A (instead of -a

    # arrIN=(${javaVersionString//\ / }) # does not work space is special thing, see below for what to do
    # IFS=$' '; arrIN=($javaVersionString); unset IFS; # does not work
    #javaVersionStringArr=($javaVersionString) # Reading a space-delimited string into an array

    read -a javaVersionStringArrary <<< "$javaVersionString" # Reading a space-delimited string into an array
    javaVersion=${javaVersionStringArrary[2]}
    IFS='.'; read -ra javaVersionArray <<< $javaVersion; unset IFS; # -AL- works, if javaVersion=18.1.2, then echo $javaVersionArr gives 18 1 2
    if [[ ${#javaVersionArray[@]} -ge 1 ]] && [[ ! -z ${javaVersionArray[0]} ]]; then
        if [[ "1" -eq $javaVersionArray[0] ]] && [[ ! -z ${javaVersionArray[1]} ]]; then
            if  [[ $javaVersionArray[1] -eq "8" || $javaVersionArray[1] -eq "9" ]]; then
                jdkFound=1
            fi
        elif [[ $javaVersionArray[0] -ge "10" ]]; then
            jdkFound=1
        fi
    fi
fi

if brew ls --versions $java_ver --cask > /dev/null; then
    # Java installed via brew
    echo "$java_ver is installed."
else
    # Java not installed via brew
    echo "$java_ver is not installed."
    java_not_installed=1
fi
