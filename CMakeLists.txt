cmake_minimum_required(VERSION 3.27)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(project_name DarkWin)
project(${project_name})

add_definitions(-DUNICODE -D_UNICODE)
set(CMAKE_CXX_STANDARD 17)
# ------------------------------------
# define the stub for delayed load of uxtheme.dll
# ------------------------------------
set( uxtheme_stub_SOURCES
"${CMAKE_CURRENT_SOURCE_DIR}/src/uxtheme-stub/uxtheme-stub.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/src/uxtheme-stub/uxtheme-stub.def"
)
add_library(uxtheme-stub  SHARED ${uxtheme_stub_SOURCES} )
# ------------------------------------
# dark lib
# ------------------------------------
set( dark_lib_SOURCES
"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/IatHook.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/framework.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/targetver.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/UAHMenuBar.c"
"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/UAHMenuBar.h"
#"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/wrapper.h"
#"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/wrapper.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/dark_mode.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/DarkMode/dark_mode.cpp"
)
add_library(dark_lib  STATIC ${dark_lib_SOURCES} )
# ------------------------------------
# executable
# ------------------------------------

set( SOURCES
"${CMAKE_CURRENT_SOURCE_DIR}/src/Main/main.c"
"${CMAKE_CURRENT_SOURCE_DIR}/src/Main/main.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/Main/DarkWin.exe.manifest"
)

set(RESOURCES
${CMAKE_CURRENT_SOURCE_DIR}/res/resource.h
${CMAKE_CURRENT_SOURCE_DIR}/res/main.rc
${CMAKE_CURRENT_SOURCE_DIR}/res/main.ico
${CMAKE_CURRENT_SOURCE_DIR}/res/small.ico
)

add_executable(${project_name}  WIN32 ${SOURCES} ${RESOURCES})
target_include_directories(${project_name} PUBLIC src/)
target_include_directories(${project_name} PUBLIC 
res
src/uxtheme-stub
src/DarkMode
src/DarkMode
src/Main
)
target_link_options(${project_name} PRIVATE  "/DELAYLOAD:uxtheme.dll")
target_link_libraries(${project_name}
ntdll
OneCore
dark_lib
uxtheme-stub
-manifest:embed -manifestinput:${CMAKE_CURRENT_SOURCE_DIR}/src/Main/DarkWin.exe.manifest
)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


