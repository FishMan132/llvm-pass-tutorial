#cmake_minimum_required(VERSION 3.4)
#project(llvm-pass-tutorial)
#
## we need LLVM_HOME in order not automatically set LLVM_DIR
#SET(ENV{LLVM_HOME} /Volumes/zy/ollvm/llvm_release/install)
#if (NOT DEFINED ENV{LLVM_HOME})
#    message(FATAL_ERROR "$LLVM_HOME is not defined")
#else ()
#    set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
#endif ()
#
#find_package(LLVM REQUIRED CONFIG)
#add_definitions(${LLVM_DEFINITIONS})
#include_directories(${LLVM_INCLUDE_DIRS})
#link_directories(${LLVM_LIBRARY_DIRS})
#if (${LLVM_VERSION_MAJOR} VERSION_GREATER_EQUAL 10)
#    set(CMAKE_CXX_STANDARD 14)
#endif ()
#
#add_subdirectory(skeleton)  # Use your pass name here.
#add_subdirectory(ollvm)  # ollvm
##add_subdirectory(Hikari)  # Hikari
#add_subdirectory(Armariris)  # Armariris

cmake_minimum_required(VERSION 3.4)
project(llvm-pass-tutorial)

# we need LLVM_HOME in order not automatically set LLVM_DIR
SET(ENV{LLVM_HOME} /Volumes/zy/ollvm/llvm_release/install)
if (NOT DEFINED ENV{LLVM_HOME})
    message(FATAL_ERROR "$LLVM_HOME is not defined")
else ()
    set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
endif ()

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
if (${LLVM_VERSION_MAJOR} VERSION_GREATER_EQUAL 10)
    set(CMAKE_CXX_STANDARD 14)
endif ()

#add_subdirectory(skeleton)  # Use your pass name here.
#add_subdirectory(ollvm)  # ollvm
#add_subdirectory(Hikari)  # Hikari
#add_subdirectory(Armariris)  # Armariris
add_subdirectory(ndk-ollvm-test)  # ndk-ollvm-test
#add_subdirectory(llvm-jit)  # ndk-ollvm-test

