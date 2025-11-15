
cmake_minimum_required(VERSION 3.20)
project(fateInjector VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

set(wxBUILD_SHARED OFF CACHE BOOL "OFF for static library!" FORCE) # force wxWidgets to use static library
mark_as_advanced(wxBUILD_SHARED)

add_executable(fateInjector WIN32
    src/cApp.cpp
    src/cMain.cpp
    src/config.cpp
    src/FixFilePerms.cpp
    src/inject.cpp
    src/pch.cpp
    src/taskBarIcon.cpp
    src/icon.rc
)

target_precompile_headers(fateInjector PRIVATE "src/pch.h")

SET_TARGET_PROPERTIES(fateInjector PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'") # requireAdministrator

add_subdirectory(wxWidgets-3.2.0)

target_link_libraries(fateInjector wx::net wx::core wx::base)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
