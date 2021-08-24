//
// Created by wyz on 2021/8/23.
//

#ifndef MULTIVOLUMERENDER_LOGGER_HPP
#define MULTIVOLUMERENDER_LOGGER_HPP
#include <spdlog/spdlog.h>
#define LOG_INFO(str,...) \
    spdlog::info(str,##__VA_ARGS__)
#define LOG_ERROR(str,...) \
    spdlog::error(str,##__VA_ARGS__)
#endif //MULTIVOLUMERENDER_LOGGER_HPP
