# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR "i386")

# which compilers to use for C and C++
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_SHARED_LINKER_FLAGS -m32)

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH  /usr/i386-linux-gnu)
