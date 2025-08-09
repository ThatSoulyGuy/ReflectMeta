cmake_minimum_required (VERSION 3.23...4.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("ReflectMeta")

file(GLOB_RECURSE REFLECT_META_FILES "${CMAKE_SOURCE_DIR}/ReflectMeta/Include/*.hpp" "${CMAKE_SOURCE_DIR}/Example/*.hpp" "${CMAKE_SOURCE_DIR}/Example/*.cpp")

add_executable (ReflectMeta ${REFLECT_META_FILES})

target_include_directories(ReflectMeta PUBLIC "${CMAKE_SOURCE_DIR}/ReflectMeta/Header/")