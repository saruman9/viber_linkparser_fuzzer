cmake_minimum_required(VERSION 3.8)
project(harness)

option(TRIAGE "Triaging" OFF)

link_directories(${CMAKE_SOURCE_DIR}/lib/arm64-v8a)

if(TRIAGE)
    add_definitions(-DTRIAGE)
    add_executable(harness "harness.c")
else(TRIAGE)
    add_library(harness SHARED "harness.c")
endif(TRIAGE)

set_property(TARGET harness APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-rpath=$ORIGIN")
target_link_libraries(harness icuBinder linkparser c++_shared)
