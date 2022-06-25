# store project directory
PROJECT_DIR=$(pwd)

#find correct path to java_home_path
llvm_path="/Library/Developer/CommandLineTools/usr/bin/"
set_llvm_path="export PATH=\"$llvm_path:\$PATH\""
llvm_app_path"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/"
set_llvm_app_path="export PATH=\"$llvm_app_path:\$PATH\""

cd ~
env_var_file=".zshrc"
updated=0

if [[ ! :$PATH: == *:"$llvm_path":* ]] && [[ ! :$PATH: == *:"$llvm_app_path":* ]]; then
    echo "attempt to add llvm to your path"    
    if [[ -d "${llvm_path}" ]]; then
        echo "setting llvm path"
        echo "$set_llvm_path" >> $env_var_file
        echo "llvm path has been set"
        updated=1
    else
        echo "cannot find directory \"${llvm_path}\", checking if \"${llvm_app_path}\" exists."
        if [[ -d "${llvm_app_path}" ]]; then
            echo "setting XCode application llvm path"
            echo "$set_llvm_app_path" >> $env_var_file
            echo "XCode application llvm path has been set"
            updated=1
        else
            echo "cannot find directory \"${set_llvm_app_path}\". Will not update \$PATH"
            no_llvm_found=1
        fi
    fi
fi

# cd back to original directory
cd $PROJECT_DIR

if [[ ${no_llvm_found} -eq "1" ]]; then
    echo "No llvm found on your machine. No new updates has been made to ~/$env_var_file"
elif [[ ${updated} -eq "0" ]]; then
    echo "PATH includes at least 1 path to llvm. No new changes made to ~/$env_var_file"
else
    echo "~/$env_var_file has been updated to include llvm in your \$PATH. Please restart your terminal to load the system path"
fi