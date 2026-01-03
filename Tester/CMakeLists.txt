add_library(tester STATIC


Tester.cpp
TestCPU.cpp

)

if (NOT MSVC)
    target_compile_options(tester PUBLIC -Wno-unused-parameter)
    target_compile_options(tester PUBLIC -Wno-unused-variable)
    target_compile_options(tester PUBLIC -Wno-unused-but-set-variable)
    target_compile_options(tester PUBLIC -Wno-implicit-fallthrough)
    target_compile_options(tester PUBLIC -Wno-missing-field-initializers)
endif()

# Add include paths
target_include_directories(tester PUBLIC

${CMAKE_SOURCE_DIR}/.
${CMAKE_SOURCE_DIR}/M68k
${CMAKE_SOURCE_DIR}/Moira
)
