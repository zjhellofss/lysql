//
// Created by fushenshen on 2020/2/1.
//

#ifndef LYSQL_XCENTER_H
#define LYSQL_XCENTER_H

#include <string>
#include <utility>
#include "../lysql/Lysql.h"

class Xcenter
{
public:
    bool install();

    static Xcenter *get(const std::string &ip)
    {
        if (!xcenter)
        {
            xcenter = new Xcenter(ip);
        }
        return xcenter;
    }

    bool init();

    bool addDevice(const std::string &ipAddr, const std::string &name);

    void loop();

private:
public:
    Xcenter(std::string ip) : ip(std::move(ip))
    {

    }

    virtual ~Xcenter();

    Xcenter() = delete;

public:
    static Xcenter *xcenter;
    Lysql::Lysql *lysql = nullptr;
    std::string ip;
};

#endif //LYSQL_XCENTER_H
