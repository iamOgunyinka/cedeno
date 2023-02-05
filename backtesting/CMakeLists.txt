cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Project
get_filename_component(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

set(PROJECT_NAME jbacktest)

# find python3 development package
find_package(Python3 COMPONENTS Development Interpreter REQUIRED)
if (NOT Python3_FOUND)
  message(FATAL_ERROR "You need to have Python3 installed on your machine")
else()
  message("Python3 include lib found at ${Python3_INCLUDE_DIRS}")
  include_directories(${Python3_INCLUDE_DIRS})
  link_directories(${Python3_LIBRARY_DIRS})
  link_libraries(${Python3_LIBRARIES})
endif()

# find pybind11 package
find_package(pybind11 CONFIG REQUIRED)
if (NOT pybind11_FOUND)
  message(FATAL_ERROR "Please install pybind11 via pip")
else()
  include_directories(${pybind11_INCLUDE_DIRS})
  link_libraries(${pybind11_LIBRARIES})
endif()

find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "You need to have Boost installed")
else()
  set(Boost_USE_STATIC_LIBS OFF) 
  set(Boost_USE_MULTITHREADED ON)  
  set(Boost_USE_STATIC_RUNTIME OFF)
# message("${PROJECT_DIR}/../third-party/CLI11")
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif()

# use openSSL
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
  message(FATAL_ERROR "You need to have OpenSSL installed (e.g. libssl-dev)")
else()
  include_directories(${OPENSSL_INCLUDE_DIR})
endif()

include_directories(${PROJECT_DIR}/include)
include_directories(${PROJECT_DIR}/../third-party)
include_directories(${PROJECT_DIR}/../third-party/pybind11/include)
include_directories(${PROJECT_DIR}/../third-party/spdlog/include)
include_directories(${PROJECT_DIR}/../third-party/csv-parser/single_include)

if(WIN32)
  # set stuff for windows
else()
  # set stuff for other systems
  link_directories(/usr/lib)
  link_libraries(stdc++fs pthread odbc)
endif()

# Outputs
set(OUTPUT_DEBUG ${PROJECT_DIR}/lib)
set(OUTPUT_RELEASE ${PROJECT_DIR}/lib)

project(${PROJECT_NAME} CXX)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

############## Artifacts Output ############################
# Defines outputs , depending BUILD TYPE                   #
############################################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DEBUG}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DEBUG}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${OUTPUT_DEBUG}")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")
endif()

# Messages
message("${PROJECT_NAME}: MAIN PROJECT: ${CMAKE_PROJECT_NAME}")
message("${PROJECT_NAME}: CURR PROJECT: ${CMAKE_CURRENT_SOURCE_DIR}")
message("${PROJECT_NAME}: CURR BIN DIR: ${CMAKE_CURRENT_BINARY_DIR}")

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

if (ENABLE_BT_DATABASE)
  add_definitions( -DBT_USE_WITH_DB)
endif()

# Source Files
set(SRC_FILES
  src/adaptor.cpp
  src/aggregate_trade.cpp
  src/arg_parser_extension.cpp
  src/arguments_parser.cpp
  src/callbacks.cpp
  src/candlestick_data.cpp
  src/common.cpp
  src/database_connector.cpp
  src/depth_data.cpp
  src/futures_order_book.cpp
  src/global_data.cpp
  src/matching_engine.cpp
  src/spot_order_book.cpp
  src/py_wrapper.cpp
  src/trades_data.cpp
  src/user_data.cpp
)

source_group("Sources" FILES ${SRC_FILES})

# Header Files
set(HEADERS_FILES
  include/adaptor.hpp
  include/aggregate_trade.hpp
  include/arguments_parser.hpp
  include/callbacks.hpp
  include/candlestick_data.hpp
  include/common.hpp
  include/container.hpp
  include/data_streamer.hpp
  include/database_connector.hpp
  include/database_types.hpp
  include/depth_data.hpp
  include/global_data.hpp
  include/matching_engine.hpp
  include/futures_order_book.hpp
  include/spot_order_book.hpp
  include/order_book_base.hpp
  include/py_wrapper.hpp
  include/trades_data.hpp
  include/user_data.hpp
)

source_group("Headers" FILES ${HEADERS_FILES})

# Add executable to build.

if (ENABLE_LIBS_FOR_TEST)
  add_library(jbacktest STATIC ${SRC_FILES} ${HEADERS_FILES})
else()
  pybind11_add_module(jbacktest ${SRC_FILES} ${HEADERS_FILES})
endif()

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 -O3")
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /O2")
endif()

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
   -D_DEBUG 
   -D_CONSOLE 
    )
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /MD /Od /Zi /EHsc /std:c++17)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
   -DNDEBUG 
   -D_CONSOLE 
    )
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /GL /Oi /Gy /Zi /EHsc /std:c++17)
    endif()
endif()
