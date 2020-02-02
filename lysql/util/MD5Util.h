//
// Created by fushenshen on 2020/2/1.
//

#ifndef LYSQL_MD5UTIL_H
#define LYSQL_MD5UTIL_H

#include <boost/algorithm/hex.hpp>
#include <boost/uuid/detail/md5.hpp>

bool getMd5(std::string &strMd5, const char *buffer, size_t bufferSize = 0);

#endif //LYSQL_MD5UTIL_H
