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

include_directories(
        include
)

macro(add_GLSL name)
    add_dependencies(GLSL ${name})
    add_llvm_example(${name} ${ARGN})
endmacro(add_GLSL name)

file(GLOB SRC_FILES
        src/*.cpp
        main.cpp)

add_GLSL(GLSL_EXE
        ${SRC_FILES})
