# store project directory
PROJECT_DIR=$(pwd)

#find correct path to java_home_path
docdb_home="$PROJECT_DIR/build/odbc/bin"
set_docdb_home="export DOCUMENTDB_HOME=\"$docdb_home\""
odbc_instini="$PROJECT_DIR/build/odbc/lib/ignite-odbc-install.ini"
set_odbc_instini="export ODBCINSTINI=\"$odbc_instini\""

cd ~
env_var_file=".zshrc"
updated=0

if [[ ! -d "${docdb_home}" ]]
then
    echo "cannot find directory \"${docdb_home}\", please check that you're under the DocumentDB ODBC repository before executing this script and that a build script has run successfully."
    exit 1
fi

if [[ ! -f "${odbc_instini}" ]]
then
    echo "cannot find file \"${odbc_instini}\", please check that you're under the DocumentDB ODBC repository before executing this script and that a build script has been run successfully."
    exit 1
fi

# check env vars and write them as needed
if [[ "${DOCUMENTDB_HOME}" != "${docdb_home}" ]]; then
    echo "DOCUMENTDB_HOME is not set properly. Appending definition of DOCUMENTDB_HOME to end of ${env_var_file}"
    echo "$set_docdb_home" >> $env_var_file
    echo "DOCUMENTDB_HOME has been set"
    updated=1
fi

if [[ "${ODBCINSTINI}" != "${odbc_instini}" ]]; then
    echo "ODBCINSTINI is not set properly. Appending definition of ODBCINSTINI to end of ${env_var_file}"
    echo "$set_odbc_instini" >> $env_var_file
    echo "ODBCINSTINI has been set"
    updated=1
fi

# cd back to original directory
cd $PROJECT_DIR

if [[ ${updated} -eq "0" ]]; then
    echo "DOCUMENTDB_HOME and ODBCINSTINI are set properly. No changes made to ~/$env_var_file"
else
    echo "~/$env_var_file has been updated to set environment variables properly. Please restart your terminal to load the variables"
fi