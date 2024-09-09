cmake_minimum_required(VERSION 3.5)
project(vir-reflect-light
  VERSION 0.0.100
  LANGUAGES CXX)

# Define main library
add_library(vir-reflect-light INTERFACE)
target_include_directories(
  vir-reflect-light INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)

# Require at least C++17
target_compile_features(vir-reflect-light INTERFACE cxx_std_20)

if(MSVC)
  target_compile_options(vir-reflect-light INTERFACE /Zc:preprocessor)
endif()

# Define the install target
install(TARGETS vir-reflect-light EXPORT vir-reflect-light-config)
install(
  EXPORT vir-reflect-light-config
  NAMESPACE vir-reflect-light::
  DESTINATION lib/cmake/vir-reflect-light)
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/vir
  DESTINATION include
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# Test library
add_library(test test.cpp)
target_link_libraries(test PRIVATE vir-reflect-light)
