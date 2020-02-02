//
// Created by fushenshen on 2020/2/1.
//

#include "MD5Util.h"

bool getMd5(std::string &strMd5, const char *buffer, size_t bufferSize)
{
    if (buffer == nullptr)
    {
        return false;
    }
    if (!bufferSize)
    {
        bufferSize = strlen(buffer);
    }
    boost::uuids::detail::md5 md5;
    md5.process_bytes(buffer, bufferSize);
    boost::uuids::detail::md5::digest_type digest;
    md5.get_digest(digest);
    const auto char_digest = reinterpret_cast<const char *>(&digest);
    strMd5.clear();
    boost::algorithm::hex(char_digest, char_digest + sizeof(boost::uuids::detail::md5::digest_type),
                          std::back_inserter(strMd5));
    return true;
}

