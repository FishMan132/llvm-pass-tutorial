# ollvm/armariris/hikari port to llvm10 in 2020
由于最刚开始是clone，不是fork的，在学习的过程中做了一些修改，推到自己git下面后，为了标明原出处，特注明：
**该仓库clone自 https://github.com/LeadroyaL/llvm-pass-tutorial**

学习历程：

1、下载llvm源码，编译生成clang等文件；下载ollvm源码，编译。

2、下载了llvm-pass-tutorial这个工程，单独编写pass，使用下面方式进行编译，编译后会生成一个so文件，
```
    cd /Volumes/zy/ollvm/llvm-pass-tutorial/build
    cmake ..
    make
```
可以使用1中生成的clang加载生成的so，测试编写的pass
    `/Volumes/zy/ollvm/llvm_release/install/bin/clang++ -std=c++14 -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk/usr/include -Xclang -load -Xclang /Volumes/zy/ollvm/llvm-pass-tutorial/build/skeleton/libSkeletonPass.so -w /Volumes/zy/ollvm/llvm-pass-tutorial/skeleton/main.cpp -o main.bin`
这种方式学习llvm语法有点不方便，每次都得生成so文件，再使用clang加载，使用jit方式直接运行会方便很多。

2.1、jit环境
成功，可以试一下llvm的语法生成代码
`clang++ -g tut1.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native orcjit` -rdynamic -O3 -o tut1`


3、学习ir指令，通过clang，把c文件转成ll文件，方便查看ir指令
    `clang -emit-llvm -S ./skeleton/main.cpp -o ./skeleton/main.ll`

上面的 2、2.1、3 都是在mac环境下，不涉及交叉编译。
上面的目标是熟悉ir指令，熟悉llvm基本api，最终能得到
c代码 <--> ir指令  <--> llvm api生成对应代码 三者对应关系。

4、将ollvm集成到ndk中
```
    cp -r /Volumes/zy/ollvm/obfuscator_release/bin/. ~/Library/Android/sdk/ndk-bundle/toolchains/llvm/prebuilt/darwin-x86_64/bin
    cp -r /Volumes/zy/ollvm/obfuscator_release/lib/. ~/Library/Android/sdk/ndk-bundle/toolchains/llvm/prebuilt/darwin-x86_64/lib
```
在clion中使用集成了ollvm的ndk，在./ndk-ollvm-test/CMakeLists.txt中加入
```
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")
    
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mllvm -sub -mllvm -sobf -mllvm -fla -mllvm -bcf")
```
点击 build --> Rebuild ，在 cmake-build-ndk/ndk-ollvm-test 目录下会生成libtestollvm.so文件
存在的问题，这种方式是把pass写进llvm源码，编译需要重新编译整个llvm源码文件，再复制上面文件到ndk中，会很麻烦，有没有办法单独编写pass。
https://www.leadroyal.cn/?p=1008 这里大神介绍了一种方式，但介绍说不适用于mac环境，所以还没试。


5、使用集成的ollvm，混淆代码，学习ollvm的混淆规则

6、学习ollvm的解混淆
