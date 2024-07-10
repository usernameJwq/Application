project(app_opengl)

file(GLOB SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC
	glfw
	glad::glad
	app_base
	glm::glm
)
target_include_directories(${PROJECT_NAME} PRIVATE 
	${Stb_INCLUDE_DIR}
)