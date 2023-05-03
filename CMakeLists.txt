cmake_minimum_required(VERSION 3.12.0)
project(StarSystemSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/source")
set(INC_DIR "${CMAKE_SOURCE_DIR}/include")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}")

file(GLOB_RECURSE SOURCE_FILES
	${SRC_DIR}/*.cpp
	${SRC_DIR}/*.c)

file(GLOB_RECURSE HEADER_FILES
	${INC_DIR}/*.h)

include_directories(${INC_DIR})

link_directories(${CMAKE_SOURCE_DIR}/libs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
find_package(OpenGL REQUIRED)
set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/glm")
find_package(GLM REQUIRED)
set(GLFW3_INCLUDE_DIR "${INC_DIR}/GLFW")
set(GLFW3_LIBRARY "${CMAKE_SOURCE_DIR}/libs")
find_package(GLFW3 REQUIRED)

add_library(GLAD "${CMAKE_SOURCE_DIR}/source/glad.c")

set(LIBS GLFW3 opengl32 GLAD)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBS})
