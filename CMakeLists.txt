# 1. required version
cmake_minimum_required(VERSION 3.4.1)

# 2. initial setup
include ("project_configs.cmake")
project (${PROJECT_NAME})

# 3. build commands output
if (${USE_MSVC_PROJECT})
	message (STATUS "MSVC Solution / Project structure will be generated")
endif ()

# 4.1 file listing
if (${USE_MSVC_PROJECT})
	file (GLOB_RECURSE file_list
		LIST_DIRECTORIES false
		"${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h"
		"${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.inl"
		"${PROJECT_SOURCE_DIR}/src/*.cpp")
else ()
	file (GLOB_RECURSE file_list
		LIST_DIRECTORIES false
		"${PROJECT_SOURCE_DIR}/src/*.cpp")
endif ()

# 4.2 exclude file according to platform
if (${TARGET_PLATFORM} STREQUAL "arm64-v8a" OR ${TARGET_PLATFORM} STREQUAL "arm-v7a")
	file (GLOB_RECURSE remove_file_list
		LIST_DIRECTORIES false
		"${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/platform/windows/*.h"
		"${PROJECT_SOURCE_DIR}/src/platform/windows/*.cpp")
	list (REMOVE_ITEM file_list ${remove_file_list})
else ()
	file (GLOB_RECURSE remove_file_list
		LIST_DIRECTORIES false
		"${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/platform/android/*.h"
		"${PROJECT_SOURCE_DIR}/src/platform/android/*.cpp")
	list (REMOVE_ITEM file_list ${remove_file_list})
endif ()

# 5. specify target
add_library (${PROJECT_NAME} ${file_list})

# 6. compiling
target_include_directories (${PROJECT_NAME}
	PUBLIC		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# 7. linking
target_link_libraries (${PROJECT_NAME}
	floral
	helich
	clover
	insigne
	refrain2)

# 8. misc
if (${USE_MSVC_PROJECT})
	# organize filters
	foreach(_source IN ITEMS ${file_list})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${PROJECT_SOURCE_DIR}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endif (${USE_MSVC_PROJECT})