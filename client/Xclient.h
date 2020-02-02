//
// Created by fushenshen on 2020/2/2.
//

#ifndef LYSQL_XCLIENT_H
#define LYSQL_XCLIENT_H

#include "../lysql/Lysql.h"
#include <string>

class Xclient
{
public:
    bool init(const std::string &ip);

    Xclient() noexcept;

    bool login(const std::string &name, const std::string &pswd);

    bool checkInput(const std::string &in);

    void loop(const std::string &cmd);

    void getLog(int page);

private:

    virtual ~Xclient();

    Lysql::Lysql *lysql = nullptr;

    void searchIp(const std::string &ip);
};

#endif //LYSQL_XCLIENT_H
