cmake_minimum_required (VERSION 2.6)

project (ar-color-balancing)

if(WIN32)
  #set(OpenCV_DIR "d:/libs/opencv-2.4.8/build")
else()
  set(OpenCV_DIR "/usr/local/lib/opencv")
endif()

find_package(OpenCV REQUIRED core imgproc imgcodecs highgui)

LINK_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set (CMAKE_DEBUG_POSTFIX "_d")

set(RUN_DIR ${PROJECT_SOURCE_DIR}/bin)

############# Main #############

set(TARGET_NAME main)

add_executable(${TARGET_NAME} WIN32
  src/${TARGET_NAME}.cpp
  include/rbf.h include/colors.h include/datahelpers.h include/mathext.h

)

target_link_libraries(${TARGET_NAME} ${OpenCV_LIBS})

set_property(TARGET ${TARGET_NAME} PROPERTY DEBUG_POSTFIX _d)
if(MSVC)
  configure_file(${PROJECT_SOURCE_DIR}/build/templates/vs2013.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.vcxproj.user @ONLY)
endif(MSVC)

############# randomPoints #############

add_executable(randomPoints WIN32 src/utils/randomPoints.cpp)
set_property(TARGET randomPoints PROPERTY DEBUG_POSTFIX _d)
if(MSVC)
  configure_file(${PROJECT_SOURCE_DIR}/build/templates/vs2013.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/randomPoints.vcxproj.user @ONLY)
endif(MSVC)

############# sampleColorsImagePair #############

add_executable(sampleColorsImagePair WIN32
  src/utils/sampleColorsImagePair.cpp
  include/colors.h include/datahelpers.h include/mathext.h
)

target_link_libraries(sampleColorsImagePair ${OpenCV_LIBS})

set_property(TARGET sampleColorsImagePair PROPERTY DEBUG_POSTFIX _d)
if(MSVC)
  configure_file(${PROJECT_SOURCE_DIR}/build/templates/vs2013.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/sampleColorsImagePair.vcxproj.user @ONLY)
endif(MSVC)

############# ############# #############

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)
include_directories(deps/eigen)
include_directories(${OpenCV_INCLUDE_DIRS})


if (APPLE)
include_directories(/usr/local/include)
endif()

ADD_CUSTOM_TARGET(debug ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Debug ${PROJECT_SOURCE_DIR})
ADD_CUSTOM_TARGET(release ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Release ${PROJECT_SOURCE_DIR})
