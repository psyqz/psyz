cmake_minimum_required(VERSION 3.10)

project(hello)

set(CMAKE_C_STANDARD 11)

add_subdirectory(../../psyz build)
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)
target_link_libraries(${PROJECT_NAME} PRIVATE psyz)
target_include_directories(psyz PRIVATE ../../psyz/include)
target_compile_definitions(psyz PRIVATE __psyz)
