cmake_minimum_required( VERSION 3.0.0 FATAL_ERROR )

set( CMAKE_CXX_STANDARD 11 )

project(ID003_LIB_V3 )


set( QT_MIN_VERSION "5.5.1" )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )
# Find includes in corresponding build directories
set( CMAKE_INCLUDE_CURRENT_DIR ON )


find_package( Qt5  ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core )

set(SRCS
    id003_lib_v3.cpp
    Idcomm.cpp
    IDcomand.cpp
    logs/KbiredUtilities.cpp

)

set( HDRS
    id003_lib_v3.h
    id003_lib_v3_global.h
    Idcomm.h
    IDcomand.h
    logs/KbiredUtilities.h
)

add_library(${PROJECT_NAME} SHARED ${SRCS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core)



install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

install(FILES ${HDRS} DESTINATION include)
