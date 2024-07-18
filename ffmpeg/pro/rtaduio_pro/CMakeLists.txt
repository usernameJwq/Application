project(rtaudio_pro)

file(GLOB SOURCE_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC
	RtAudio::rtaudio
)

add_subdirectory(test)