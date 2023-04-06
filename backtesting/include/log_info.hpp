#pragma once
#include <spdlog/spdlog.h>

#define PRINT_INFO(str, ...)                                                   \
  {                                                                            \
    if (verbose)                                                               \
      spdlog::info(str, ##__VA_ARGS__);                                        \
  }

#define ERROR_PARSE() (m_config.reset(), m_argumentParsed)
#define PRINT_ERROR(str, ...)                                                  \
  {                                                                            \
    if (verbose)                                                               \
      spdlog::error(str, ##__VA_ARGS__);                                       \
  }

#define ERROR_EXIT(str, ...)                                                   \
  PRINT_ERROR(str, ##__VA_ARGS__)                                              \
  return ERROR_PARSE();
