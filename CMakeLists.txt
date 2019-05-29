cmake_minimum_required(VERSION 3.10)

project(spool LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    # Disable tests by default if this project is transitively included
    option(SPOOL_TESTS_ENABLED "Enable spool test compilation" OFF)
    # Augment the module path to allow `include(Spool)`
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake PARENT_SCOPE)
else()
    option(SPOOL_TESTS_ENABLED "Enable spool test compilation" ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
endif()


add_subdirectory(spooler)
add_subdirectory(public)

if (SPOOL_TESTS_ENABLED)
    add_subdirectory(test)
endif()
