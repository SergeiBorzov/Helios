cmake_minimum_required(VERSION 2.8.11)

project(Helios)

find_package(Embree 3.0 REQUIRED)
find_package(assimp)
find_package(glm CONFIG REQUIRED)

# assimp cmake fix https://github.com/ros-planning/geometric_shapes/issues/23
find_library(
    assimp_LIBRARIES
    NAMES assimp
)

set(CMAKE_CXX_FLAGS "-Wall -g -pedantic")
set(CMAKE_CXX_STANDARD 17)

file(GLOB Sources
   "src/*.cpp"
   "src/Core/*.cpp"
   "src/Integrator/*.cpp"
   "src/Scene/*.cpp"
   "src/Camera/*.cpp"
   "src/Material/*.cpp"
   "src/Light/*.cpp" 
   "src/Texture/*.cpp"
)

add_executable(Helios ${Sources})
target_include_directories(Helios PUBLIC ${EMBREE_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/include)
target_link_libraries(Helios PRIVATE glm ${assimp_LIBRARIES} ${EMBREE_LIBRARY})