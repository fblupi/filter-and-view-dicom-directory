cmake_minimum_required(VERSION 2.8)
PROJECT(FilterAndViewDICOMDirectroy)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED QUIET)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CPP_FILES *.cpp)

qt5_wrap_ui(UISrcs ${UI_FILES})
add_executable(FilterAndViewDICOMDirectroy ${CPP_FILES} ${UISrcs} ${QT_WRAP})
qt5_use_modules(FilterAndViewDICOMDirectroy Core Gui)
target_link_libraries(FilterAndViewDICOMDirectroy ${VTK_LIBRARIES})