//
// Created by fushenshen on 2020/2/1.
//

#include "Xcenter.h"
#include "../lysql/Lysql.h"
#include "../lysql/ResultData.h"
#include "../lysql/util/MD5Util.h"
#include <iostream>
#include <map>
#include <thread>
#include <regex>

using std::cout;
using std::endl;
using std::cerr;

bool Xcenter::install()
{
    bool f = this->init();
    if (!f)
    {
        cerr << "安装配置文件初始化失败" << endl;
        return false;
    }
    cout << "安装配置文件初始化成功" << " 当前ip地址为: " << this->ip << endl;
    this->lysql->startTransaction();
    this->lysql->query("drop table if exists `t_strategy`");
    std::string sql = "CREATE TABLE IF NOT EXISTS `t_strategy`  (`id` int AUTO_INCREMENT,`name` varchar(512)  character set 'utf8' COLLATE 'utf8_bin',`strategy` varchar(8192),PRIMARY KEY(`id`)) ENGINE =InnoDB";
    f = this->lysql->query(sql.data());
    if (!f)
    {
        //数据库发生错误后回滚
        this->lysql->rollBack();
        return false;
    }
    else
    {
        std::map<std::string, ResultData> values;
        values.insert({"name", "用户登录失败"});
        values.insert({"strategy",
                       ".*Failed password for (.+) from ([0-9.]+) port ([0-9]+).*"});
        f = this->lysql->insert(values, "t_strategy");
        if (!f)
        {
            this->lysql->rollBack();
            return false;
        }
        values.clear();

        values.insert({"name", "用户登录成功"});
        values.insert({"strategy",
                       ".*Accepted password for (.+) from ([0-9.]+) port ([0-9]+).*"});
        f = this->lysql->insert(values, "t_strategy");
        if (!f)
        {
            this->lysql->rollBack();
            return false;
        }
    }
    this->lysql->query("drop table if exists `t_user`");
    sql = "CREATE TABLE IF NOT EXISTS `t_user`  (`id` int AUTO_INCREMENT,`name` varchar(512)  character set 'utf8' COLLATE 'utf8_bin',`pswd` varchar(1024),PRIMARY KEY(`id`)) ENGINE = InnoDB";
    f = this->lysql->query(sql.data());
    if (!f)
    {
        //数据库发生错误后回滚
        this->lysql->rollBack();
        return false;
    }
    else
    {
        std::map<std::string, ResultData> values;
        std::string pswd;
        getMd5(pswd, "123");
        values.insert({"name", "fss"});
        values.insert({"pswd",
                       pswd.data()});
        f = this->lysql->insert(values, "t_user");
        if (!f)
        {
            this->lysql->rollBack();
            return false;
        }
    }

    sql = "CREATE TABLE IF NOT EXISTS `t_log`  (\
		`id` int AUTO_INCREMENT,\
		`ip` varchar(32),\
		`log` varchar(8192),\
        `log_time` datetime default CURRENT_TIMESTAMP, \
		PRIMARY KEY(`id`)\
		) ENGINE =InnoDB";
    f = lysql->query(sql.data());
    if (!f)
    {
        //数据库发生错误后回滚
        this->lysql->rollBack();
        return false;
    }
    else
    {
        //成功创建日指标
    }

    sql = "CREATE TABLE IF NOT EXISTS `t_device`  (\
		`id` int AUTO_INCREMENT,\
		`ip` varchar(32),\
		`name` varchar(8192),\
        `last_time` datetime, \
		PRIMARY KEY(`id`)\
		) ENGINE =InnoDB";
    f = lysql->query(sql.data());
    if (!f)
    {
        //数据库发生错误后回滚
        this->lysql->rollBack();
        return false;
    }
    else
    {
        //成功创建设备表
    }
    sql = "CREATE TABLE IF NOT EXISTS `t_res`  (\
		`id` int AUTO_INCREMENT,\
		`name` varchar(8192),\
		`content` varchar(4096),\
        `user` varchar(512), \
        `device_ip` varchar(16) ,\
        `from_ip` varchar(16), \
        `port` int ,\
        `last_heart` datetime default CURRENT_TIMESTAMP,\
		 PRIMARY KEY(`id`)\
		) ENGINE =InnoDB";

    //创建审计结果表
    f = lysql->query(sql.data());
    if (!f)
    {
        //数据库发生错误后回滚
        this->lysql->rollBack();
        return false;
    }

    this->lysql->stopTransaction();
    this->lysql->commit();
    return true;
}

Xcenter *Xcenter::xcenter = nullptr;

bool Xcenter::init()
{
    if (!this->lysql)
    {
        this->lysql = new Lysql::Lysql();
    }
    bool connectSuccess = lysql->connect(this->ip.data(), "root", "123", "mysqlCourse", 3306);
    if (!connectSuccess)
    {
        cerr << "数据库连接失败\n" << endl;
        return false;
    }

    return true;
}

Xcenter::~Xcenter()
{
    if (this->lysql != nullptr)
    {
        delete this->lysql;
        this->lysql = nullptr;
    }
}

bool Xcenter::addDevice(const std::string &ipAddr, const std::string &name)
{

    /*  sql = "CREATE TABLE IF NOT EXISTS `t_device`  (\
          `id` int AUTO_INCREMENT,\
          `ip` varchar(32),\
          `name` varchar(8192),\
          `last_time` datetime, \
          PRIMARY KEY(`id`)\
          ) ENGINE =InnoDB";*/
    std::map<std::string, ResultData> values;
    values.insert({"ip", ipAddr.data()});
    values.insert({"name", name.data()});
    bool f = this->lysql->insert(values, "t_device");
    return f;
}

void Xcenter::loop()
{
    int lastId;

    std::map<std::string, std::regex> strategies;
    std::string sql = "select * from t_strategy ";
    auto rows = this->lysql->getResults(sql);
    for (const auto &row:rows)
    {
        //加入审计规则
        if (!row[1].data || !row[2].data)
        {
            continue;
        }
        strategies.insert({std::string(row[1].data), std::regex(row[2].data)});
    }
    //Xcenter的主循环操作
    rows = this->lysql->getResults("select max(id) from t_log");
    assert(rows.size() == 1);
    lastId = atoi(rows[0][0].data);
    while (true)
    {
        sql = "select * from t_log where id > " + std::to_string(lastId);
        rows = this->lysql->getResults(sql);
        if (rows.empty())
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(500ms);
        }
        else
        {
            for (const auto &row:rows)
            {
                lastId = atoi(row[0].data);
                if (!row[2].data)
                {
                    continue;
                }
                //打印log信息
                for (const auto &s:strategies)
                {
                    std::smatch match;
                    std::string data = row[2].data;
                    bool r = std::regex_match(data, match, s.second);
                    if (!r || match.empty())
                    {
                        continue;
                    }
                    else
                    {
                        //     在匹配的情况下
                        std::map<std::string, ResultData> values;
                        //规则(strategy)对应的用户
                        values.insert({"name", s.first.data()});
                        //log表中的内容
                        values.insert({"content", data.data()});
                        //策略表中的ip地址
                        if (row[1].data != nullptr)
                        {
                            values.insert({"device_ip", row[1].data});
                        }
                        //该条日志的产生者
                        std::string user = match[1];
                        //该条日志(log)的产生的IP地址
                        std::string fromIp = match[2];
                        std::string port = match[3];
                        values.insert({"user", user.data()});
                        values.insert({"from_ip", fromIp.data()});
                        values.insert({"port", port.data()});
                        this->lysql->insert(values, "t_res");
                    }

                }
            }
        }
    }

}
