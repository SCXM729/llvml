# llvm toturial 入门教程

记录我自己的学习过程，也是我的第一篇博客,如果有错误，还望大家指正   
主要是对 llvm 官网上的教程的翻译和补充，多增加了有些有用的内容，帮助大家了解 llvm

## 安装llvm
- 编译工具
   1. clang 和 clang++ ：这个比 gcc 好，gcc 编译 llvm 会提示很多条警告信息，clang 只有一两条

   2. lld：链接器，属于 llvm 工具链，官网上说这个可以大大减少编译的时间。 实际编译的时候，链接占用的内存 lld 比 ld 少很多，而且用 ld 可能会因为链接内存不够导致编译失败

- 编译流程
  1. 下载源代码， 整个项目大概占用4.9G空间      
     ```bash
     git clone https://github.com/llvm/llvm-project.git              
     ```
     为了减少项目下载大小和下载时间（下载时间挺长的），也可以后面加上 --depth=1   
     ```bash
     git clone https://github.com/llvm/llvm-project.git --depth=1
     ```
  2. 一些 cmake 变量
     - CMAKE_BUILD_TYPE=Debug 设置为 Debug    
     - CMAKE_C_COMPILER=clang 使用 clang 编译 c,不指定的话gcc
     - CMAKE_CXX_COMPILER=clang++ 使用 clang++编译 c++， 不指定的话g++
     - LLVM_USE_LINKER=lld 使用 lld 作为链接器, 不指定的话ld
     - LLVM_ENABLE_PROJECTS="llvm;clang" 要编译哪些项目，以分号为间隔，这里我写的是 llvm 和 clang，也可以加上其他的 lld,mlir 之类的
     - LLVM_ENABLE_ASSERTIONS=ON 启动断言检查，Debug 模式默认是开启的
     - LLVM_TARGETS_TO_BUILD="Native;ARM" 编译哪些平台,同样是以分号为间隔，这里是本机（我的是amd也就是x86）和arm处理器，不指定的话就是所有平台
  3. cmake 确定命令如下     
     ```bash
     cmake -S llvm -B build -G Ninja \    
     -DCMAKE_BUILD_TYPE=Debug \    
     -DCMAKE_C_COMPILER=clang \    
     -DCMAKE_CXX_COMPILER=clang++  \    
     -DLLVM_ENABLE_PROJECTS="llvm" \    
     -DLLVM_USE_LINKER=lld \    
     -DLLVM_ENABLE_ASSERTIONS=ON  \    
     -DLLVM_TARGETS_TO_BUILD="Native;ARM"   
     ```        
  3. 构建命令, 这里的check-llvm是为了执行测试llvm,上面的cmake命令只有llvm项目,所以这里只测试了llvm, 想要测试所有项目的话使用 check-all
      ```
      cmake --build build --target check-llvm 
      ```

---
## 使用llvm
使用cmake构建项目，这里我把上面下载的llvm-project放在了家目录下  
main.cpp
```c++
#include"llvm/Support/raw_ostream.h"
int main(){
   llvm::outs()<<"llvm";
}
```
CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20.0)
# put the command on the project command
project(llvmL LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(LLVM_DIR ~/llvm-project/build/lib/cmake/llvm)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Now build our tools
add_executable(main main.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(main ${llvm_libs})
```