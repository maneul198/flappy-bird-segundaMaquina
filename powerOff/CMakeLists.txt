
cmake_minimum_required( VERSION 3.0.0 FATAL_ERROR )

set( CMAKE_CXX_STANDARD 11 )

project( powerOff-E135 )

set( QT_MIN_VERSION "5.5.5" )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )
# Find includes in corresponding build directories
set( CMAKE_INCLUDE_CURRENT_DIR ON )

find_library( dpci_lib NAMES dpci )

find_package( Qt5  ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
)


set(CMAKE_BUILD_TYPE "Debug")

set(SRCS
    main.cpp
    key.cpp
)

set( SRCH
    key.h
)

add_executable(${PROJECT_NAME} ${RCC_SOURCES} ${SRCS} )

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    "/usr/local/lib/libdigitalio_dpx.so"
    dpci
)

install( TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION "bin" )
