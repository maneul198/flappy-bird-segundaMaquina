cmake_minimum_required( VERSION 3.0.0 FATAL_ERROR )

set( CMAKE_CXX_STANDARD 11 )

project(enginesv)

set( QT_MIN_VERSION "5.5.0" )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )
# Find includes in corresponding build directories
set( CMAKE_INCLUDE_CURRENT_DIR ON )

find_package( Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core )
find_library( dpci_lib NAMES dpci )

set( HDRS
    enginesv_global.h
    enginesv.h
)

set( SRCS enginesv_dpx.cpp )
add_library( ${PROJECT_NAME} SHARED ${SRCS} )

target_link_libraries( ${PROJECT_NAME} Qt5::Core dpci )


install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib )

install( FILES ${HDRS} DESTINATION include )
