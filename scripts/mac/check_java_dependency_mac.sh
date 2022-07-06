java_ver=corretto

# todo -AL- check if JAVA_HOME is set
# TODO change set_java_def_mac.sh as well, as we don;t need to check java_home twice. 
# Checks that JAVA_HOME is defined, the path to JAVA_HOME exists 
# and is a JDK (i.e., include subdirectory)
if [[ ! -z "${JAVA_HOME}" ]] && [[ -d "${JAVA_HOME}" ]] && [[ -d "${JAVA_HOME}/include" ]]; then
    # Test Java in path and has minimum version
#   -AL- e.g., openjdk 18 2022-03-22
    javaVersionString=`echo "$(java --version)" | head -1` # works in terminal but not in .sh 
    # -AL- tests
    #javaVersionString="java 1.2 2012-11-11" # pass, 1,2
    #javaVersionString="java 1.7 2012-11-11" # pass, 1,2
    #javaVersionString="java 1.8 2012-11-11" # pass, 1,2,3
    echo "-AL- TRACE javaVersionString: $javaVersionString"
    # IFS=';' read -ra javaVersionStringArr <<< $javaVersionString # invalid -a option "read: bad option: -a"
# In order to read in an array in zsh, read requires the option -A (instead of -a

    # arrIN=(${javaVersionString//\ / }) # does not work space is special thing, see below for what to do
    # IFS=$' '; arrIN=($javaVersionString); unset IFS; # does not work
    #javaVersionStringArr=($javaVersionString) # Reading a space-delimited string into an array

    read -a javaVersionStringArrary <<< "$javaVersionString" # Reading a space-delimited string into an array
    echo "-al- trace javaVersionStringArrary[1]: ${javaVersionStringArrary[1]}"
    javaVersion=${javaVersionStringArrary[1]}
    javaVersionArray=(${javaVersion//./ }) # separate javaVersion string by delimiter '.'
    echo "-al- trace javaVersionArray: $javaVersionArray"
    echo "-al- trace javaVersionArray array length: ${#javaVersionArray[@]}"
    echo "-al- trace javaVersionArray[0]: ${javaVersionArray[0]}"
    if [[ ${#javaVersionArray[@]} -ge 1 ]] && [[ ! -z ${javaVersionArray[0]} ]]; then
        echo "pass 1"
        if [[ "1" -eq ${javaVersionArray[0]} ]] && [[ ! -z ${javaVersionArray[1]} ]]; then
            echo "pass 2"
            if  [[ ${javaVersionArray[1]} -eq "8" || ${javaVersionArray[1]} -eq "9" ]]; then
                echo "pass 3"
                jdkFound=1
            fi
        elif [[ ${javaVersionArray[0]} -ge "10" ]]; then
            echo "pass 4"
            jdkFound=1
        fi
    fi
fi

if [[ "${jdkFound}" -eq "1" ]]; then
    echo "Found suitable JDK on user's machine."
else 
    echo "Did not find JDK on user's machine. If you have JDK installed, please ensure to set JAVA_HOME to the home directory of JDK to help me find it next time."
fi
