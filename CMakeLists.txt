cmake_minimum_required(VERSION 3.10)
project(dunebuggies)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Include SFML headers
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

file(GLOB DUNEBUGGIES_SOURCES *.cpp)
file(GLOB DUNEBUGGIES_HEADERS *.hpp)

add_executable(
  dunebuggies
  main.cpp
  ${DUNEBUGGIES_SOURCES}
  ${DUNEBUGGIES_HEADERS}
)

target_compile_features(dunebuggies PRIVATE cxx_std_17)
target_link_libraries(dunebuggies PRIVATE SFML::Graphics SFML::Network SFML::System SFML::Window)
