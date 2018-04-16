cmake_minimum_required (VERSION 3.0)

include ("project_configs.cmake")

project (${PROJECT_NAME})

include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/include")

file (GLOB_RECURSE file_list
	"${PROJECT_SOURCE_DIR}/src/*.cpp")

add_definitions(
	-D_CRT_SECURE_NO_WARNINGS
	-DPLATFORM_WINDOWS)

add_library (${PROJECT_NAME} ${file_list})

target_link_libraries (${PROJECT_NAME}
	floral
	clover
	refrain2)

target_include_directories (${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
