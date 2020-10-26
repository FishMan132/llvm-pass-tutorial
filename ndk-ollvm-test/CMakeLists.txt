add_library(testollvm MODULE
        testollvm.cpp
        )

#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mllvm -fla -mllvm -split -mllvm -split_num=3 -mllvm -sub -mllvm -sub_loop=3 -mllvm -bcf -mllvm -bcf_loop=3 -mllvm -sobf")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mllvm -fla -mllvm -split -mllvm -split_num=3 -mllvm -sub -mllvm -sub_loop=3 -mllvm -bcf -mllvm -bcf_loop=3 -mllvm -sobf")

#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")

# Use C++11 to compile your pass (i.e., supply -std=c++11).
target_compile_features(testollvm PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(testollvm PROPERTIES
        COMPILE_FLAGS "-fno-rtti"
        )

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if (APPLE)
    set_target_properties(testollvm PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif (APPLE)