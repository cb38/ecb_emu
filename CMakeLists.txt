cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Set the project name
project(testTester)

# Enable testing
enable_testing()

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)
  
# Add sub directories
add_subdirectory(Moira)
add_subdirectory(Tester/Binutils)


#tester
set(TESTER_SOURCES
  Tester/main.cpp
  Tester/Tester.cpp
  Tester/TestCPU.cpp
)

add_executable(testTester ${TESTER_SOURCES})
add_executable(testTester_release ${TESTER_SOURCES})


foreach(target_name IN ITEMS testTester testTester_release)
  target_link_libraries(${target_name} PRIVATE binutils moira)

  if(MSVC)
    target_compile_options(${target_name} PRIVATE /wd4100 /wd4201 /wd4324 /wd4458)
  else()
    target_compile_options(${target_name} PRIVATE -Wno-unused-parameter)
    target_compile_options(${target_name} PRIVATE -Wno-unused-variable)
    target_compile_options(${target_name} PRIVATE -Wno-gnu-anonymous-struct)
    target_compile_options(${target_name} PRIVATE -Wno-nested-anon-types)
  endif()

  target_include_directories(${target_name} PUBLIC
    ${CMAKE_SOURCE_DIR}/.
    ${CMAKE_SOURCE_DIR}/Moira
    ${CMAKE_SOURCE_DIR}/Tester
    ${CMAKE_SOURCE_DIR}/Tester/Binutils
  )
endforeach()

# Release-flavoured target tweaks
target_compile_definitions(testTester_release PRIVATE NDEBUG)
if(MSVC)
  target_compile_options(testTester_release PRIVATE /O2)
else()
  target_compile_options(testTester_release PRIVATE -O3)
endif()

# Add tests
add_test(UnitTest testTester)
