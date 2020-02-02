//
// Created by fushenshen on 2020/2/1.
//

#ifndef LYSQL_XAGENT_H
#define LYSQL_XAGENT_H

#include "../lysql/Lysql.h"
#include <string>

class Xagent
{
public:
    bool init(const std::string &ip);

    virtual ~Xagent();

    static Xagent *get()
    {
        if (!xagent)
        {
            xagent = new Xagent();
        }
        return xagent;
    }

    void loop();

    bool saveLog(const std::string &log);

    std::string getLocalIpAddress();

private:
    Xagent();

public:
    Lysql::Lysql *lysql = nullptr;
    static Xagent *xagent;
    std::string ip;
};


#endif //LYSQL_XAGENT_H
