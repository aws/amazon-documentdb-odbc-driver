# Checks that JAVA_HOME is defined, the path to JAVA_HOME exists 
# and is a JDK (i.e., include subdirectory)
if [[ ! -z "${JAVA_HOME}" ]] && [[ -d "${JAVA_HOME}" ]] && [[ -d "${JAVA_HOME}/include" ]]; then
    # Test Java in path and has minimum version
    javaVersionString=`echo "$(java --version)" | head -1`
    read -a javaVersionStringArrary <<< "$javaVersionString" # Reading a space-delimited string into an array
    javaVersion=${javaVersionStringArrary[1]}
    javaVersionArray=(${javaVersion//./ }) # separate javaVersion string by delimiter '.'
    if [[ ${#javaVersionArray[@]} -ge 1 ]] && [[ ! -z ${javaVersionArray[0]} ]]; then
        if [[ "1" -eq ${javaVersionArray[0]} ]] && [[ ! -z ${javaVersionArray[1]} ]]; then
            if  [[ ${javaVersionArray[1]} -eq "8" || ${javaVersionArray[1]} -eq "9" ]]; then
                jdkFound=1
            fi
        elif [[ ${javaVersionArray[0]} -ge "10" ]]; then
            jdkFound=1
        fi
    fi
fi

if [[ "${jdkFound}" -eq "1" ]]; then
    echo "Found suitable JDK on user's machine."
else 
    echo "Did not find JDK on user's machine. If you have JDK installed, please ensure to set JAVA_HOME to the home directory of JDK to help me find it next time."
fi
