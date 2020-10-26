add_library(Hikari MODULE
        # List your source files here.
        FunctionCallObfuscate.cpp
        CryptoUtils.cpp
        BogusControlFlow.cpp
        Substitution.cpp
        Flattening.cpp
        Utils.cpp
        SplitBasicBlocks.cpp
        AntiClassDump.cpp
        StringEncryption.cpp
        IndirectBranch.cpp
        FunctionWrapper.cpp
        Obfuscation.cpp
        include/Transforms/Obfuscation/AntiClassDump.h
        include/Transforms/Obfuscation/BogusControlFlow.h
        include/Transforms/Obfuscation/CryptoUtils.h
        include/Transforms/Obfuscation/Flattening.h
        include/Transforms/Obfuscation/FunctionCallObfuscate.h
        include/Transforms/Obfuscation/FunctionWrapper.h
        include/Transforms/Obfuscation/IndirectBranch.h
        include/Transforms/Obfuscation/Obfuscation.h
        include/Transforms/Obfuscation/Split.h
        include/Transforms/Obfuscation/StringEncryption.h
        include/Transforms/Obfuscation/Substitution.h
        include/Transforms/Obfuscation/Utils.h
        Enter.cpp
        )

# Use C++11 to compile your pass (i.e., supply -std=c++11).
target_compile_features(Hikari PRIVATE cxx_range_for cxx_auto_type)

include_directories(include)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(Hikari PROPERTIES
        COMPILE_FLAGS "-fno-rtti"
        )

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if (APPLE)
    set_target_properties(Hikari PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif (APPLE)
