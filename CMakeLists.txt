cmake_minimum_required(VERSION 3.31)

project(2d_CAE LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_executable(2d_CAE src/main.cpp)



add_library(
  imgui STATIC 
  dependencies/ImGui/include/imgui_draw.cpp
  dependencies/ImGui/include/imgui_tables.cpp
  dependencies/ImGui/include/imgui_widgets.cpp
  dependencies/ImGui/include/imgui.cpp
  dependencies/ImGui/include/backends/imgui_impl_opengl3.cpp
)
  
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGui/include)

include(FetchContent)


FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML
  GIT_TAG 2.5.1
  GIT_SHALLOW TRUE
  
)
  
FetchContent_MakeAvailable(SFML)

target_link_libraries(2d_CAE PRIVATE
  sfml-graphics
  sfml-window
  sfml-system
  imgui
)



