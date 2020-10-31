cmake_minimum_required(VERSION 2.8.11)

project(Helios)

find_package(Embree 3.0 REQUIRED)
find_package(glm CONFIG REQUIRED)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -Wextra -g -O0 -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -O2")
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
target_include_directories(Helios PUBLIC 
    ${EMBREE_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/include/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/include/tinygltf
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/include/nlohmann
)
target_link_libraries(Helios PRIVATE glm ${EMBREE_LIBRARY})