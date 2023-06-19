#cmake_minimum_required(VERSION 3.5)
#
#project(GLSLParser)
#
#set(CMAKE_CXX_STANDARD 17)
#find_package(LLVM REQUIRED CONFIG)
#message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
#message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
#include_directories(${LLVM_INCLUDE_DIRS})
#separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
#add_definitions(${LLVM_DEFINITIONS_LIST})
#llvm_map_components_to_libnames(llvm_libs core orcjit support nativecodegen)
#include_directories(include)
#file(GLOB SRC_FILES src/*.cpp main.cpp)
#add_executable(GLSLParser ${SRC_FILES})
#target_link_libraries(GLSLParser PUBLIC ${llvm_libs})


set(LLVM_LINK_COMPONENTS
        Core
        ExecutionEngine
        Object
        OrcJIT
        Support
        TargetParser
        native
        )

add_custom_target(GLSL)
set_target_properties(GLSL PROPERTIES FOLDER Examples)

macro(add_GLSL name)
    add_dependencies(GLSL ${name})
    add_llvm_example(${name} ${ARGN})
endmacro(add_GLSL name)

file(GLOB SRC_FILES
        src/*.cpp
        ./main.cpp)


include_directories(
        include
)

add_GLSL(GLSLParser
        ${SRC_FILES})


