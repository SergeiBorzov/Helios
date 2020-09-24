cmake_minimum_required(VERSION 2.8.11)

project(Helios)
find_package(Embree 3.0 REQUIRED)
set(CMAKE_CXX_FLAGS "-Wall -g -pedantic")
set(CMAKE_CXX_STANDARD 17)


file(GLOB Sources
   "src/main.cpp"
   "src/renderer.cpp"
)

add_executable(Helios ${Sources})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/lib)
target_include_directories(Helios PUBLIC ${EMBREE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/include)
target_link_libraries(Helios PRIVATE ${EMBREE_LIBRARY})
