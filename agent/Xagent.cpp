//
// Created by fushenshen on 2020/2/1.
//

#include "Xagent.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/stat.h>

using std::cout;
using std::cerr;
const char endl = '\n';

Xagent::Xagent() : ip(getLocalIpAddress())
{
}

Xagent *Xagent::xagent = nullptr;

bool Xagent::init(const std::string &dbIp)
{
    if (dbIp.empty())
    {
        return false;
    }
    this->lysql = new Lysql::Lysql();
    bool connectSuccess = lysql->connect(dbIp.data(), "root", "123", "mysqlCourse", 3306);
    assert(connectSuccess);

    return true;
}

Xagent::~Xagent()
{
    if (this->lysql != nullptr)
    {
        delete this->lysql;
        this->lysql = nullptr;
    }
}

void Xagent::loop()
{

    std::string path = "/var/log/system.log";
    std::string log;
    struct stat st{};
    stat(path.data(), &st);
    int size = st.st_size;
    while (true)
    {
        std::ifstream f(path, std::ios::in);
        f.seekg(size, std::ios_base::beg);
        if (!f.is_open())
        {
            cerr << "log file open error" << endl;
            return;
        }
        while (true)
        {
            char c = f.get();
            if (c <= 0)
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(500ms);
                f.close();
                break;
            }
            if (c == '\n')
            {
                size += log.size() + 1;
                this->saveLog(log);
                log.clear();
            }
            else
            {
                log += c;
            }
        }
    }
}


bool Xagent::saveLog(const std::string &log)
{
    cout << log << endl;
    std::map<std::string, ResultData> values;
    values.insert({"log", log.data()});
    values.insert({"ip", this->ip.data()});
//    values.insert({"@log_time", "now()"});
    bool f = this->lysql->insert(values, "t_log");
    return f;
}

std::string Xagent::getLocalIpAddress()
{
    struct ifaddrs *ifAddrStruct = nullptr;
    struct ifaddrs *ifa = nullptr;
    void *tmpAddrPtr = nullptr;

    getifaddrs(&ifAddrStruct);
    std::string ip_;
    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo0") != 0)
        {
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            ip_ = std::string(addressBuffer);
        }

    }
    if (ifAddrStruct != nullptr)
        freeifaddrs(ifAddrStruct);
    return ip_;
}


















