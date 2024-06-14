project(app_ffmpeg)

file(GLOB_RECURSE SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC
	${FFMPEG_INCLUDE_DIRS}
)
target_link_directories(${PROJECT_NAME} PUBLIC
	${FFMPEG_LIBRARY_DIRS}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
	${FFMPEG_LIBRARIES}
	app_base
)