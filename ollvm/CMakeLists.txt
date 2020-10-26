add_library(ollvm MODULE
        # List your source files here.
        BogusControlFlow.cpp
        CryptoUtils.cpp
        Flattening.cpp
        SplitBasicBlocks.cpp
        Substitution.cpp
        Utils.cpp
        include/llvm/CryptoUtils.h
        include/Transforms/Obfuscation/BogusControlFlow.h
        include/Transforms/Obfuscation/Flattening.h
        include/Transforms/Obfuscation/Split.h
        include/Transforms/Obfuscation/Substitution.h
        include/Transforms/Obfuscation/Utils.h
        Enter.cpp
        )

# Use C++11 to compile your pass (i.e., supply -std=c++11).
target_compile_features(ollvm PRIVATE cxx_range_for cxx_auto_type)

include_directories(include)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(ollvm PROPERTIES
        COMPILE_FLAGS "-fno-rtti"
        )

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if (APPLE)
    set_target_properties(ollvm PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif (APPLE)
