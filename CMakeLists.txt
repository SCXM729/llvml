cmake_minimum_required(VERSION 3.20.0)
# put the command on the project command
project(llvmL LANGUAGES C CXX)

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
add_executable(testl)
target_sources(testl PRIVATE testllvm.cpp)

add_executable(main2)
target_sources(main2 PRIVATE main2.cpp KaleidoscopeJIT.h)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader orcjit native)

# Link against LLVM libraries
target_link_libraries(main2 PRIVATE ${llvm_libs})
target_link_libraries(testl PRIVATE ${llvm_libs})

# 把主文件目录下的code.txt复制到build目录下
add_custom_target(parse_text ALL COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/code.txt ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(main2 parse_text)

# copy the executable file main2 to the upper directory
add_custom_command(TARGET main2 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/main2 ${CMAKE_CURRENT_SOURCE_DIR})

# 输出编译时间和链接时间
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

# 时间戳
#string(TIMESTAMP START_TIME %Y%m%d_%H%M%S)


if(DEBUG_PARSE)
  target_compile_definitions(main2 PRIVATE -DDEBUG_PARSE)
endif()

if(Op)
  target_compile_definitions(main2 PRIVATE -DOptimizations)
endif()

include(CTest)
enable_testing()
# add_test(NAME mtest COMMAND $<TARGET_FILE)


# add_library(c)
# target_sources(c
#   PUBLIC
#     FILE_SET CXX_MODULES FILES
#       c.cc)
# add_executable(mainc c.cpp)
# target_link_libraries(mainc  PRIVATE c)
