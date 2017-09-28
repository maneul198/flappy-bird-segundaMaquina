 
cmake_minimum_required( VERSION 3.0.0 FATAL_ERROR )

set( CMAKE_CXX_STANDARD 11 )

project(serial_arduino_pc)

set( HDRS )

set( SRCS )

add_library( ${PROJECT_NAME} SHARED ${SRCS} )
