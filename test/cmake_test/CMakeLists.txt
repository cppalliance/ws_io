#
# Copyright (c) 2023 Christian Mazakas
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

cmake_minimum_required(VERSION 3.5...3.16)

project(cmake_subdir_test LANGUAGES CXX)
set(__ignore__ ${CMAKE_C_COMPILER})
set(__ignore__ ${CMAKE_C_FLAGS})

if(BOOST_CI_INSTALL_TEST)
  find_package(Boost CONFIG REQUIRED COMPONENTS ws_io)
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  add_subdirectory(../.. boostorg/ws_io)

  set(BOOST_URL_BUILD_TESTS OFF CACHE BOOL "" FORCE)

  set(deps
    # Primary dependencies

    asio
    assert
    buffers
    config
    http_proto
    system
    throw_exception
    url
    ws_proto

    # Secondary dependencies

    align
    context
    date_time
    variant2
    winapi
    core
    static_assert
    mp11
    pool
    predef
    smart_ptr
    algorithm
    io
    lexical_cast
    numeric/conversion
    range
    tokenizer
    type_traits
    utility
    array
    bind
    concept_check
    exception
    function
    iterator
    mpl
    regex
    tuple
    unordered
    container
    conversion
    preprocessor
    integer
    container_hash
    detail
    optional
    intrusive
    move
    describe
    function_types
    fusion
    functional
    typeof
  )

  foreach(dep IN LISTS deps)
    add_subdirectory(../../../${dep} boostorg/${dep} EXCLUDE_FROM_ALL)
  endforeach()
endif()

add_executable(main main.cpp)
target_link_libraries(main Boost::ws_io)

enable_testing()
add_test(NAME main COMMAND main)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>)
