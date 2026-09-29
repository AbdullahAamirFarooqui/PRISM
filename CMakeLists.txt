cmake_minimum_required(VERSION 3.16)
project(PRISM VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PRISM_BUILD_GUI "Build the Qt desktop application" ON)

# ---------------------------------------------------------------------------
# 1) prism_core : the backend (DSA, Algorithms, Data, FileSystem)
#    Pure C++17, no Qt, no STL containers.
# ---------------------------------------------------------------------------
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS
    src/DSA/*.cpp
    src/Algorithms/*.cpp
    src/Data/*.cpp
    src/FileSystem/*.cpp
)
file(GLOB CORE_HEADERS CONFIGURE_DEPENDS
    src/DSA/*.h
    src/Algorithms/*.h
    src/Data/*.h
    src/FileSystem/*.h
)

add_library(prism_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(prism_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(prism_core PRIVATE /W4)
else()
    target_compile_options(prism_core PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# 2) prism_tests : console unit tests for every data structure
# ---------------------------------------------------------------------------
add_executable(prism_tests tests/test_main.cpp)
target_link_libraries(prism_tests PRIVATE prism_core)

# 3) prism_console : small command line demo of the backend
add_executable(prism_console tests/console_demo.cpp)
target_link_libraries(prism_console PRIVATE prism_core)

# ---------------------------------------------------------------------------
# 4) PRISM : the Qt Widgets GUI (UI + Visualization)
# ---------------------------------------------------------------------------
if(PRISM_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    set(CMAKE_AUTOMOC ON)

    file(GLOB GUI_SOURCES CONFIGURE_DEPENDS
        src/UI/*.cpp src/UI/*.h
        src/Visualization/*.cpp src/Visualization/*.h
        src/main.cpp
    )

    qt_add_executable(PRISM WIN32 ${GUI_SOURCES})
    target_link_libraries(PRISM PRIVATE prism_core Qt6::Widgets)

    # Copy the sample datasets next to the executable so "Open sample" works.
    add_custom_command(TARGET PRISM POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:PRISM>/data)
endif()
