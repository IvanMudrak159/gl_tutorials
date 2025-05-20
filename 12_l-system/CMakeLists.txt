# Set the minimum CMake version
cmake_minimum_required(VERSION 3.10)

project(12_lsystem)

add_executable(12_lsystem 
	main.cpp
	../utils/error_handling.hpp
	../utils/ogl_resource.hpp
	../utils/shader.hpp
	../utils/window.hpp
)
target_sources(12_lsystem PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/src/glad.c
)
target_link_libraries(12_lsystem utils glm::glm glfw OpenGL::GL)
target_include_directories(12_lsystem PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/../utils
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)
