cmake_minimum_required(VERSION 2.8)
PROJECT(FilterAndViewDICOMDirectory)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
if (ITKVtkGlue_LOADED)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
else()
  find_package(ItkVtkGlue REQUIRED)
  include(${ItkVtkGlue_USE_FILE})
  set(Glue ItkVtkGlue)
endif()

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED QUIET)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CPP_FILES *.cpp)

qt5_wrap_ui(UISrcs ${UI_FILES})
add_executable(FilterAndViewDICOMDirectory ${CPP_FILES} ${UISrcs} ${QT_WRAP})
qt5_use_modules(FilterAndViewDICOMDirectory Core Gui)
target_link_libraries(FilterAndViewDICOMDirectory ${Glue} ${VTK_LIBRARIES} ${ITK_LIBRARIES})
