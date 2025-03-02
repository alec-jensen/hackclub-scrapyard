cmake_minimum_required(VERSION 3.15)
project(simon_game_ffi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

set(SOURCES
    src/SerialMonitor.cpp
    src/Logger.cpp
    src/middleWhere.cpp
    src/ffi.cpp
)

add_library(simon_game SHARED ${SOURCES})

target_include_directories(simon_game 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(WIN32)
    target_link_libraries(simon_game PRIVATE user32 gdi32)
endif()

if(WIN32)
    set_target_properties(simon_game PROPERTIES 
        OUTPUT_NAME "simon_game"
        PREFIX ""
        SUFFIX ".dll"
    )
elseif(APPLE)
    set_target_properties(simon_game PROPERTIES 
        OUTPUT_NAME "simon_game"
        PREFIX "lib"
        SUFFIX ".dylib"
    )
else()
    set_target_properties(simon_game PROPERTIES 
        OUTPUT_NAME "simon_game"
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()
include(GNUInstallDirs)
install(TARGETS simon_game
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES include/simon_game.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)