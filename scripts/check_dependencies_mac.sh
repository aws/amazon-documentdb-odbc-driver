
if brew ls --versions libiodbc > /dev/null; then
  # libiodbc is installed
  echo "libiodbc is installed"
else
  # libiodbcis not installed
  echo "libiodbc is not installed."
  $libiodbc_installed=0
fi

s=("football" "cricket" "hockey" "libiodbc") 
num_apps=${#s[@]}
apps_installed=()

for (( i=0 ; i<$num_apps ; i++ )); 
do
  echo $i
  apps_installed+=(0)
  if brew ls --versions ${s[i]} > /dev/null; then
    # $${s[i]} is installed via brew
    echo "${s[i]} is installed."
    apps_installed[i]=1
  else
    # $${s[i]} not installed via brew
    echo "${s[i]} is not installed."
    # $n_installed=0
  fi
  echo $n
done

# -AL- TRACE
echo ${apps_installed[*]}

# for n in ${s[@]}; 
# do
#   if brew ls --versions $n > /dev/null; then
#     # $n is installed via brew
#     echo "$n is installed."
#     apps_installed[i]=1
#   else
#     # $n not installed via brew
#     echo "$n is not installed."
#     # $n_installed=0
#   fi
#   echo $n
# done

for (( i=0 ; i<$num_apps ; i++ )); 
do
  if [[ "${apps_installed[i]}" -eq "0" ]]; then
    echo  "${s[i]} is not installed."
  else
    echo  "${s[i]} is installed."
  fi

done