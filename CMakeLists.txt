cmake_minimum_required(VERSION 3.12)

set(EXEC agle)
project(${EXEC} VERSION 0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 2.5 
  COMPONENTS 
    system window graphics network audio REQUIRED
  )

file(GLOB SOURCES src/*)

# Compile executable
add_executable(${EXEC} ${SOURCES})

# Set include directory search paths
target_include_directories(${EXEC} 
  PRIVATE
  "${PROJECT_BINARY_DIR}"
  )

# Link executable to required SFML libraries
target_link_libraries(${EXEC} sfml-graphics)

# Install target
install(TARGETS ${EXEC} DESTINATION bin)

