# Backtesting
# Copyright (C) 2022-2023 Cread and Bear LLC
# Copyright (C) 2022-2023 Joshua Ogunyinka

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # We use -std=c++17 instead of -std=gnu++17 in macOS
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(COMMAND cmake_policy)
  # CMP0003: Libraries linked via full path no longer produce linker search paths.
  #cmake_policy(SET CMP0003 NEW)
  if(CMAKE_MAJOR_VERSION GREATER 2)
    # CMP0046: Old behavior to silently ignore non-existent dependencies.
    cmake_policy(SET CMP0046 OLD)
  endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are:
        None Debug Release RelWithDebInfo Profile."
      FORCE)
endif()

# Backtesting
project(backtesting C CXX)

# Check repository status
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third-party/csv-parser/CMakeLists.txt)
  message(FATAL_ERROR "Your backtesting repository is incomplete, initialize submodules using:\n  git submodule update --init --recursive")
endif()

find_package(Boost REQUIRED)

if(NOT Boost_FOUND)
  message(FATAL_ERROR "You need to have Boost installed")
else()
  set(Boost_USE_STATIC_LIBS OFF) 
  set(Boost_USE_MULTITHREADED ON)  
  set(Boost_USE_STATIC_RUNTIME OFF)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# find python3 development package
find_package(Python3 COMPONENTS Development Interpreter REQUIRED)
if (NOT Python3_FOUND)
  message(FATAL_ERROR "You need to have Python3 installed on your machine")
endif()

# find pybind11 package
find_package(pybind11 CONFIG REQUIRED)
if (NOT pybind11_FOUND)
  message(FATAL_ERROR "Please install pybind11 via pip")
endif()

######################################################################
# Options (these can be specified in cmake command line or modifying
# CMakeCache.txt)

option(ENABLE_BACKTNG_LIB     "Compile BacktestingLib" on)
option(ENABLE_DATA_EXT_LIB    "Compile DataExtractorLib" on)
option(ENABLE_TEST_DATA_EXT   "Compile TestDataExtractor" on)
option(ENABLE_TEST_BACKTNG    "Compile TestBacktesting" off)
option(ENABLE_BT_DATABASE     "Enable database support for backtesting" off)
option(ENABLE_LIBS_FOR_TEST   "Compile the libraries for testing" off)
option(ENABLE_INDICATOR       "Compile the library with indicators" off)

# Select openssl
if(REQUIRE_SSL)
  if(APPLE)
    option(CMAKE_USE_SECTRANSP "enable Apple OS native SSL/TLS" ON)
  endif()
  if(WIN32)
    set(CMAKE_USE_SCHANNEL "enable Windows native SSL/TLS" ON)
  endif()
endif()

######################################################################
# Profile build type

list(APPEND CMAKE_BUILD_TYPES Profile)
mark_as_advanced(
    CMAKE_C_FLAGS_PROFILE
    CMAKE_CXX_FLAGS_PROFILE
    CMAKE_EXE_LINKER_FLAGS_PROFILE)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS_PROFILE "-pg"
        CACHE STRING "Profiling C flags")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE}"
        CACHE STRING "Profiling C++ flags")
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg"
        CACHE STRING "Profiling linker flags")
endif()

if(MSVC)
    set(CMAKE_C_FLAGS_PROFILE "/MT /Zi /Ox /Gd"
        CACHE STRING "Profiling C flags")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE}"
        CACHE STRING "Profiling C++ flags")
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "/PROFILE /DEBUG"
        CACHE STRING "Profiling linker flags")
endif()

######################################################################
# Directories

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# We need to specify the output for each configuration to make it work
# on Visual Studio solutions.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_PROFILE "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_PROFILE "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_PROFILE "${CMAKE_BINARY_DIR}/bin")

set(CLI11_DIR           ${CMAKE_CURRENT_SOURCE_DIR}/third_party/CLI11)
set(CSVPARSER_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/third_party/csv-parser)
set(OTL_DIR             ${CMAKE_CURRENT_SOURCE_DIR}/third_party/otl_v4)
set(PYBIND11_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11)
set(RAPIDJSON_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/third_party/rapidjson)
set(SPDLOG_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog)

# Search in the "cmake" directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(ENABLE_CCACHE)
  find_package(CCache)
  if(CCache_FOUND)
    # Use e.g. "ccache clang++" instead of "clang++"
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCache_EXECUTABLE}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    "${CCache_EXECUTABLE}")
  endif()
endif()
# Put libraries into "lib".
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

######################################################################
# Common definitions to compile all sources (app code and third party)

# Debug C/C++ flags
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-DDEBUGMODE -D_DEBUG)
else()
  add_definitions(-DNDEBUG)
endif()

if(ENABLE_INDICATOR)
  add_definitions(-DBT_USE_WITH_INDICATORS)
endif()

if (ENABLE_BACKTNG_LIB)
  add_subdirectory(backtesting)
endif()

if (ENABLE_DATA_EXT_LIB)
  add_subdirectory(data_extractor)
endif()

if (ENABLE_TEST_DATA_EXT)
  add_subdirectory(test_data_extractor)
endif()

if (ENABLE_LIBS_FOR_TEST)
  if (ENABLE_TEST_BACKTNG)
    add_subdirectory(test_backtesting)
  endif()
endif()
