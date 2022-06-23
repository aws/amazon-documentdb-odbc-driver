# script to check if formulas in req_apps are installed. 

req_apps=("libiodbc" "cmake" "openssl" "boost" "mongo-cxx-driver") 
num_apps=${#req_apps[@]}
apps_installed=()

for (( i=0 ; i<$num_apps ; i++ )); 
do
  echo $i # -AL- TRACE remove later
  apps_installed+=(0)
  if brew ls --versions ${req_apps[i]} > /dev/null; then
    # ${req_apps[i]} is installed via brew
    echo "${req_apps[i]} is installed."
    apps_installed[i]=1
  else
    # ${req_apps[i]} not installed via brew
    echo "${req_apps[i]} is not installed."
  fi
  echo ${req_apps[i]} # -AL- TRACE remove later
done

# -AL- TRACE, remove later
echo ${apps_installed[*]}

for (( i=0 ; i<$num_apps ; i++ )); 
do
  if [[ "${apps_installed[i]}" -eq "0" ]]; then
    echo  "${req_apps[i]} is not installed."
  else
    echo  "${req_apps[i]} is installed."
  fi
done