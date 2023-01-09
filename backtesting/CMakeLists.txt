cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Project
get_filename_component(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

set(PROJECT_NAME libbacktng)

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

# Source Files
set(SRC_FILES
  adaptor.cpp
  arguments_parser.cpp
  arg_parser_extension.cpp
  candlestick_data.cpp
  common.cpp
  database_connector.cpp
  depth_data.cpp
  matching_engine.cpp
  order_book.cpp
  user_data.cpp
)

source_group("Sources" FILES ${SRC_FILES})

# Header Files
set(HEADERS_FILES
  adaptor.hpp
  arguments_parser.hpp
  candlestick_data.hpp
  common.hpp
  database_connector.hpp
  data_streamer.hpp
  depth_data.hpp
  matching_engine.hpp
  order_book.hpp
  user_data.hpp
)

source_group("Headers" FILES ${HEADERS_FILES})

# Add executable to build.
add_library(libbacktng STATIC 
   ${SRC_FILES} ${HEADERS_FILES}
)

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
