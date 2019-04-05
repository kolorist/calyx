cmake_minimum_required (VERSION 3.0)

include ("project_configs.cmake")

project (${PROJECT_NAME})

include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/include")

set (file_list
	"${PROJECT_SOURCE_DIR}/src/context.cpp"
	"${PROJECT_SOURCE_DIR}/src/memory.cpp"
	"${PROJECT_SOURCE_DIR}/src/life_cycle.cpp")

add_definitions(
	-D_CRT_SECURE_NO_WARNINGS)
	
if (${ANDROID_BUILD})
	message(STATUS ${PROJECT_NAME} " will be built using Android configs")
	add_definitions (
		-DPLATFORM_POSIX)
		
	# platform abi
	if (${ANDROID_ABI} STREQUAL "arm64-v8a")
		message(STATUS ${PROJECT_NAME} " Android ABI: arm64")
		add_definitions (
			-DPOSIX64)
	else ()
		message(STATUS ${PROJECT_NAME} " Android ABI: arm")
		add_definitions (
			-DPOSIX32)
	endif (${ANDROID_ABI} STREQUAL "arm64-v8a")	
	
	set (file_list
		${file_list}
		"${PROJECT_SOURCE_DIR}/src/platform/android/system.cpp"
		"${PROJECT_SOURCE_DIR}/src/platform/android/entry_point.cpp")
	
else ()
	message(STATUS ${PROJECT_NAME} " will be built using Windows configs")
	add_definitions (
		-DPLATFORM_WINDOWS)
	set (file_list
		${file_list}
		"${PROJECT_SOURCE_DIR}/src/platform/windows/entry_point.cpp"
		"${PROJECT_SOURCE_DIR}/src/platform/windows/system.cpp")
endif (${ANDROID_BUILD})

add_library (${PROJECT_NAME} ${file_list})

target_link_libraries (${PROJECT_NAME}
	floral
	helich
	clover
	insigne
	refrain2)

target_include_directories (${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
