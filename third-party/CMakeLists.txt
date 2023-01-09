# Backtesting
# Copyright (C) 2022 Creed and Bear LLC
# Copyright (C) 2022 Joshua Ogunyinka

include_directories(.)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

if(NOT USE_SHARED_CSV_PARSER)
  add_subdirectory(csv-parser)
endif()

if(NOT USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if(NOT USE_SHARED_RAPIDJSON)
  add_subdirectory(rapidjson)
endif()

if(NOT USE_SHARED_SPDLOG)
  add_subdirectory(spdlog)
endif()
