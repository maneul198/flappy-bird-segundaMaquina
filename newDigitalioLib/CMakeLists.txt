cmake_minimum_required( VERSION 3.0.0 FATAL_ERROR )

include_directories(${Qt5Widgets_INCLUDE_DIRS})

set( CMAKE_CXX_STANDARD 14 )

project(digitalio_dpx)

set( QT_MIN_VERSION "5.5.1" )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )
# Find includes in corresponding build directories
set( CMAKE_INCLUDE_CURRENT_DIR ON )


find_package( Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core )

set( SRCS
    digitalinput.cpp
    digitaloutput.cpp
    watchioports.cpp
    main.cpp
)

set( HDRS
    digitalinput.h
    digitalio_dpx_global.h
    digitaloutput.h
    digital_ports.h
    watchioports.h
)

add_library( ${PROJECT_NAME} SHARED ${SRCS} )

target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Core
)

install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib )

install( FILES ${HDRS} DESTINATION include )
