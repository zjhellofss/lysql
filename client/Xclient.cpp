//
// Created by fushenshen on 2020/2/2.
//

#include "Xclient.h"
#include "../lysql/util/MD5Util.h"
#include <iostream>
#include <chrono>

using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::chrono::system_clock;

Xclient::~Xclient()
{
    if (this->lysql)
    {
        delete this->lysql;
        this->lysql = nullptr;
    }
}

Xclient::Xclient() noexcept
{}


bool Xclient::init(const std::string &ip)
{

    this->lysql = new Lysql::Lysql;
    cout << "客户端初始化成功 " << "当前ip地址为: " << ip << endl;
    bool connectSuccess = lysql->connect(ip.data(), "root", "123", "mysqlCourse", 3306);
    if (!connectSuccess)
    {
        cerr << "数据库连接失败\n" << endl;
        return false;
    }
    return true;
}

bool Xclient::login(const std::string &name, const std::string &pswd)
{
    bool loginSuccess = false;
    string md5Pswd;
    if (!checkInput(name) || !checkInput(pswd))
    {
        return false;
    }
    getMd5(md5Pswd, pswd.data());
    string sql = "select name from t_user where name = '" + name + "'";
    sql = sql + " and  pswd = '" + md5Pswd + "'";
    cout << sql << endl;
    bool f = this->lysql->query(sql.data());
    if (!f)
    {
        return false;
    }
    else
    {
        auto r = this->lysql->getOneRow();
        if (!r.empty())
        {
            loginSuccess = true;
        }
    }
    return loginSuccess;
}

bool Xclient::checkInput(const std::string &in)
{
    bool f = true;
    for (const char &a:in)
    {
        if (a == '\\' || a == '\"' || a == '(' || a == ')')
        {
            return false;
        }
    }

    return true;
}

void Xclient::loop(const std::string &cmd)
{
    if (cmd == "log")
    {
        cout << "输入页数" << endl;
        int page;
        std::cin >> page;
        this->getLog(page);
    }
    else if (cmd == "search")
    {
        cout << "输入查询的ip" << endl;
        std::string ip;
        std::cin >> ip;
        this->searchIp(ip);
    }

}

void Xclient::getLog(int page)
{
    if (page < 1)
    {
        return;
    }
    int count = 5;
    std::string sql;
    sql = "select * from t_log limit " + std::to_string((page - 1) * count) + "," + std::to_string(page * count);

    auto rows = this->lysql->getResults(sql);
    for (const auto &r:rows)
    {
        if (r[2].data)
        {
            cout << r[2].data << endl;
        }
    }
}

void Xclient::searchIp(const std::string &ip)
{
    string sql = "select * from t_log where ip = '" + ip + "'";
    cout << sql << endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto rows = this->lysql->getResults(sql);
    for (const auto &row:rows)
    {
        for (const auto &r:row)
        {
            if (r.data != nullptr)
            {
                cout << r.data << " ";
            }
        }
        cout << endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    cout << "查找用时: " << ms.count() << " ms " <<"共有数据 "<<rows.size()<<"条"<< endl;
}
