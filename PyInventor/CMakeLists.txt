find_package(Python COMPONENTS Interpreter Development REQUIRED)

list(APPEND CMAKE_PREFIX_PATH "${Python_SITELIB}")

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
pybind11_add_module(PyInventor ${srcs})

target_link_libraries(PyInventor PRIVATE
    Coin::Coin
)

get_target_property(COIN_INCLUDE_DIR Coin::Coin INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "COIN_INCLUDE_DIR: ${COIN_INCLUDE_DIR}")

file(GLOB_RECURSE pchs CONFIGURE_DEPENDS ${COIN_INCLUDE_DIR}/Inventor/*.h)

target_precompile_headers(PyInventor PRIVATE

    <pybind11/pybind11.h>
    <pybind11/stl.h>

    <Inventor/So.h>
)
