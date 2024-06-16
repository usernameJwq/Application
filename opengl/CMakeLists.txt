project(app_opengl)

file(GLOB_RECURSE SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} "glfw_cube.cpp" "glfw_cube.h")
target_link_libraries(${PROJECT_NAME} PUBLIC
	glfw
	glad::glad
	app_base
	glm::glm
)
target_include_directories(${PROJECT_NAME} PRIVATE 
	${Stb_INCLUDE_DIR}
)