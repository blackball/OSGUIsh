#
# OSGUIsh
#

# General settings
project(OSGUIsh)
cmake_minimum_required(VERSION 2.6)
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "2")

find_package(OpenSceneGraph 2.8 REQUIRED COMPONENTS osgViewer osgUtil)

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost 1.39 REQUIRED COMPONENTS signals)
add_definitions(-DBOOST_ALL_DYN_LINK)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${Boost_INCLUDE_DIRS}
                    ${OPENSCENEGRAPH_INCLUDE_DIRS})

# Build the library
set(OSGUIshSources
    Sources/EventHandler.cpp
    Sources/FocusPolicy.cpp
    Sources/ManualFocusPolicy.cpp
    Sources/MouseDownFocusPolicy.cpp
    Sources/MouseOverFocusPolicy.cpp)

add_library(OSGUIsh SHARED ${OSGUIshSources})

# Demos
add_executable(Simplest Demos/Simplest.cpp)
target_link_libraries(Simplest
    ${OPENSCENEGRAPH_LIBRARIES}
    ${Boost_LIBRARIES}
    OSGUIsh)
set_property(TARGET Simplest
    PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_executable(ThreeObjects Demos/ThreeObjects.cpp)
target_link_libraries(ThreeObjects
    ${OPENSCENEGRAPH_LIBRARIES}
    ${Boost_LIBRARIES}
    OSGUIsh)
set_property(TARGET ThreeObjects
    PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_executable(FocusPolicies Demos/FocusPolicies.cpp)
target_link_libraries(FocusPolicies
    ${OPENSCENEGRAPH_LIBRARIES}
    ${Boost_LIBRARIES}
    OSGUIsh)
set_property(TARGET FocusPolicies
    PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Copies 'data' to same place as the executable -- it's needed there
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Data
        ${PROJECT_BINARY_DIR}/Data)
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/Data
        ${PROJECT_BINARY_DIR}/Data)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")