//
// Created by fushenshen on 2020/1/30.
//

#ifndef STLLEAN_LYSQL_H
#define STLLEAN_LYSQL_H

#include <mysql.h>
#include <vector>
#include <map>
#include "ResultData.h"

namespace Lysql
{
    class Lysql
    {
    public:
        bool init();

        virtual ~Lysql();

        bool connect(const char *host, const char *user, const char *pswd, const char *db, unsigned int port,
                     unsigned int flag = 0);

        bool query(const char *sql, unsigned long sqlLen = 0);

        bool setConnectTimeout(int seconds);

        bool setReconnect(bool r);

        bool storeResult();

        bool useResult();

        bool freeResult();

        std::vector<ResultData> getOneRow();

        std::string getInsertSql(const std::map<std::string, ResultData> &resultData, const std::string &tableName);

        bool insert(const std::map<std::string, ResultData> &resultData, const std::string &tableName);

        bool insertBin(const std::map<std::string, ResultData> &resultData, const std::string &tableName);

        std::string getUpdateSql(const std::map<std::string, ResultData> &resultData, const std::string &tableName,
                                 const std::string &where);

        int update(const std::map<std::string, ResultData> &resultData, const std::string &tableName,
                   const std::string &where);

        int updateBin(const std::map<std::string, ResultData> &resultData, const std::string &tableName,
                      const std::string &where);

        bool startTransaction();

        bool stopTransaction();

        bool commit();

        bool rollBack();


        std::vector<std::vector<ResultData>> getResults(const std::string &sql);


    private:
        bool option(mysql_option option, const void *arg);

    protected:
        MYSQL *mysql = nullptr;
        MYSQL_RES *result = nullptr;
    };
};


#endif //STLLEAN_LYSQL_H
