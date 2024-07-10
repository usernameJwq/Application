project(app_linklist)

file(GLOB SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
add_subdirectory(test)