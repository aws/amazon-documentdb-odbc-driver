# script to check if formulas in req_apps are installed. 

# add the required dependencies to the req_apps array. 
# There is no need to change other variables
req_apps=("libiodbc" "cmake" "openssl" "boost" "mongo-cxx-driver") 
num_apps=${#req_apps[@]}
apps_installed=()

for (( i=0 ; i<$num_apps ; i++ )); 
do
  apps_installed+=(0)
  if brew ls --versions ${req_apps[i]} > /dev/null; then
    # ${req_apps[i]} is installed via brew
    echo "${req_apps[i]} is installed."
    apps_installed[i]=1
  else
    # ${req_apps[i]} not installed via brew
    echo "${req_apps[i]} is not installed."
    missing_formula=1
  fi
done

for (( i=0 ; i<$num_apps ; i++ )); 
do
  if [[ "${apps_installed[i]}" -eq "0" ]]; then
    echo  "${req_apps[i]} is not installed."
  else
    echo  "${req_apps[i]} is installed."
  fi
done