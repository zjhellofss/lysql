//
// Created by fushenshen on 2020/1/30.
//

#include "Lysql.h"
#include <mysql.h>
#include <iostream>
#include <map>

using std::cout;
using std::cerr;
const char endl = '\n';
namespace Lysql
{
    bool Lysql::init()
    {
        this->mysql = mysql_init(nullptr);
        if (!mysql)
        {
            cerr << "mysql_init error: " << mysql_error(this->mysql) << endl;
            return false;
        }
        return true;
    }

    Lysql::~Lysql()
    {
        if (this->result != nullptr)
        {
            mysql_free_result(this->result);
            this->result = nullptr;
        }

        if (this->mysql != nullptr)
        {
            this->mysql = nullptr;
            mysql_close(this->mysql);
        }
    }

    bool Lysql::connect(const char *host, const char *user, const char *pswd, const char *db, unsigned int port,
                        unsigned int flag)
    {
        if (!this->mysql)
        {
            bool f = this->init();
            assert(f);
        }
        if (!mysql_real_connect(this->mysql, host, user, pswd, db, port, 0, flag))
        {
            //connect error
            cerr << "real connect error " << endl;
            return false;
        }
        return true;
    }

    bool Lysql::query(const char *sql, unsigned long sqlLen)
    {
        if (!sql)
        {
            cerr << "sql is null" << endl;
            return false;
        }
        if (!sqlLen)
        {
            sqlLen = strlen(sql);
            assert(sqlLen > 0);
        }
        int res = mysql_real_query(this->mysql, sql, sqlLen);
        if (res)
        {
            cerr << "mysql query error: " << mysql_error(this->mysql) << endl;
            cerr << "error sql is : " << sql << endl;
            return false;
        }
        return true;
    }


    bool Lysql::option(mysql_option option, const void *arg)
    {
        int res = mysql_options(this->mysql, option, arg);
        if (res != 0)
        {
            cerr << "mysql option error: " << mysql_error(this->mysql) << endl;
            return false;
        }
        return true;
    }

    bool Lysql::setConnectTimeout(int seconds)
    {
        return this->option(MYSQL_OPT_CONNECT_TIMEOUT, &seconds);
    }

    bool Lysql::setReconnect(bool r)
    {
        return this->option(MYSQL_OPT_RECONNECT, &r);
    }

    bool Lysql::storeResult()
    {
        if (!this->mysql)
        {
            cerr << "store result failed : mysql is null" << endl;
            return false;
        }
        this->freeResult();
        this->result = mysql_store_result(this->mysql);
        if (!result)
        {
            cerr << "store result failed :  mysql don't have result" << endl;
            return false;
        }
        return true;
    }

    bool Lysql::useResult()
    {
        if (!this->mysql)
        {
            cerr << "store result failed : mysql is null" << endl;
            return false;
        }
        this->freeResult();
        this->result = mysql_use_result(this->mysql);
        if (!result)
        {
            cerr << "store result failed : " << mysql_error(this->mysql) << endl;
            return false;
        }
        return true;
    }

    bool Lysql::freeResult()
    {
        if (result != nullptr)
        {
            mysql_free_result(this->result);
            this->result = nullptr;
            return true;
        }
        return false;
    }

    std::vector<ResultData> Lysql::getOneRow()
    {
        std::vector<ResultData> res;
        if (!this->result)
        {
            this->storeResult();
        }
        MYSQL_ROW row = mysql_fetch_row(result);
        if (!row)
        {
            return res;
        }
        else
        {
            unsigned long *lengths = mysql_fetch_lengths(result);
            int num = mysql_num_fields(result);
            for (int i = 0; i < num; ++i)
            {
                auto field = mysql_fetch_field_direct(this->result, i);
                ResultData resultData(lengths[i], field->type, row[i]);
                resultData.type = field->type;
                res.push_back(resultData);
            }
        }
        return res;
    }

    std::string Lysql::getInsertSql(const std::map<std::string, ResultData> &resultData, const std::string &tableName)
    {
        using std::string;
        string re;
        re = "insert into `";
        re += tableName;
        re += '`';
        string key;
        string value;
        for (const auto &e:resultData)
        {
            key += "`";
            key += e.first;
            key += "`";
            key += ',';
            value += "'";
            value += e.second.data;
            value += "'";
            value += ',';
        }
        key[key.size() - 1] = ' ';
        value[value.size() - 1] = ' ';
        re += '(';
        re += key;
        re += ")values(";
        re += value;
        re += ')';
        return re;
    }

    bool Lysql::insert(const std::map<std::string, ResultData> &resultData, const std::string &tableName)
    {
        if (mysql != nullptr)
        {
            std::string re;
            if ((re = this->getInsertSql(resultData, tableName)).empty())
            {
                return false;
            }
            bool f = this->query(re.data());
            if (!f)
            {
                return false;
            }
            else
            {
                int num = mysql_affected_rows(this->mysql);
                return num > 0;
            }
        }
        else
        {
            return false;
        }
    }

    bool Lysql::insertBin(const std::map<std::string, ResultData> &resultData, const std::string &tableName)
    {
        using std::string;
        if (!this->mysql || tableName.empty() || resultData.empty())
        {
            return false;
        }
        string re;
        re = "insert into `";
        re += tableName;
        re += '`';
        MYSQL_BIND mysqlBind[256];


        string key;
        string value;
        int i = 0;
        for (const auto &e:resultData)
        {
            key += "`";
            key += e.first;
            key += "`";
            key += ',';
            value += "?,";

            mysqlBind[i].buffer = (char *) e.second.data;
            mysqlBind[i].buffer_length = e.second.size;
            mysqlBind[i].buffer_type = static_cast<enum_field_types>(e.second.type);
            i++;
        }
        key[key.size() - 1] = ' ';
        value[value.size() - 1] = ' ';
        re += '(';
        re += key;
        re += ")values(";
        re += value;
        re += ')';
        MYSQL_STMT *stmt = mysql_stmt_init(this->mysql);
        if (!stmt)
        {
            cerr << "mysql_stmt_init error : " << mysql_error(this->mysql) << endl;
            return false;
        }
        if (mysql_stmt_prepare(stmt, re.data(), re.size()))
        {
            mysql_stmt_close(stmt);
            cerr << "mysql_stmt_prepare error : " << mysql_error(this->mysql) << endl;
            return false;
        }
        if (mysql_stmt_bind_param(stmt, mysqlBind) != 0)
        {
            mysql_stmt_close(stmt);
            cerr << "mysql_stmt_prepare error : " << mysql_error(this->mysql) << endl;
            return false;
        }
        if (mysql_stmt_execute(stmt) != 0)
        {
            mysql_stmt_close(stmt);
            return false;
        }
        mysql_stmt_close(stmt);
        return true;
    }

    std::string Lysql::getUpdateSql(const std::map<std::string, ResultData> &resultData, const std::string &tableName,
                                    const std::string &where)
    {
        using std::string;
        string sql;
        if (!this->mysql || tableName.empty() || resultData.empty())
        {
            return sql;
        }
        sql = "update `";
        sql += tableName;
        sql += "` set";
        for (const auto &e:resultData)
        {
            sql += "`";
            sql += e.first;
            sql += "`='";
            if (e.second.type == MYSQL_TYPE_LONG)
            {
                sql += std::to_string(*((int *) e.second.data));
            }
            else
            {
                sql += e.second.data;
            }
            sql += "',";
        }
        sql[sql.size() - 1] = ' ';
        sql += " where ";
        sql += where;
        return sql;
    }

    int Lysql::update(const std::map<std::string, ResultData> &resultData, const std::string &tableName,
                      const std::string &where)
    {
        if (!this->mysql)
        {
            return -1;
        }
        std::string sql = getUpdateSql(resultData, tableName, where);
        if (sql.empty())
        {
            return -1;
        }
        if (!query(sql.data()))
        {
            return -1;
        }
        return mysql_affected_rows(this->mysql);

    }

    int Lysql::updateBin(const std::map<std::string, ResultData> &resultData, const std::string &tableName,
                         const std::string &where)
    {

        using std::string;
        string sql;
        if (!this->mysql || tableName.empty() || resultData.empty())
        {
            return -1;
        }

        sql = "update `";
        sql += tableName;
        sql += "` set ";
        MYSQL_BIND mysqlBind[256];
        int i = 0;
        for (const auto &e:resultData)
        {
            sql += "`";
            sql += e.first;
            sql += "`= ?";
            sql += ",";

            mysqlBind[i].buffer = (char *) e.second.data;
            mysqlBind[i].buffer_length = e.second.size;
            mysqlBind[i].buffer_type = static_cast<enum_field_types>(e.second.type);
            i++;
        }
        sql[sql.size() - 1] = ' ';
        sql += " where ";
        sql += where;

        MYSQL_STMT *stmt = mysql_stmt_init(this->mysql);
        if (!stmt)
        {
            cerr << "mysql_stmt_init error : " << mysql_error(this->mysql) << endl;
            return false;
        }
        if (mysql_stmt_prepare(stmt, sql.data(), sql.size()))
        {
            mysql_stmt_close(stmt);
            cerr << "mysql_stmt_prepare error : " << mysql_error(this->mysql) << endl;
            return false;
        }
        if (mysql_stmt_bind_param(stmt, mysqlBind) != 0)
        {
            mysql_stmt_close(stmt);
            cerr << "mysql_stmt_prepare error : " << mysql_error(this->mysql) << endl;
            return false;
        }
        if (mysql_stmt_execute(stmt) != 0)
        {
            mysql_stmt_close(stmt);
            return false;
        }
        int r = mysql_affected_rows(this->mysql);
        mysql_stmt_close(stmt);
        return r;
    }

    bool Lysql::stopTransaction()
    {
        return this->query("set autocommit= 1 ");
    }

    bool Lysql::commit()
    {
        return this->query("commit");
    }

    bool Lysql::rollBack()
    {

        return this->query("rollback ");
    }

    bool Lysql::startTransaction()
    {
        return this->query("set autocommit = 0 ");
    }

    std::vector<std::vector<ResultData>> Lysql::getResults(const std::string &sql)
    {

        auto res = std::vector<std::vector<ResultData>>();
        if (!this->query(sql.data()))
        {
            return res;
        }
        if (!this->storeResult())
        {
            return res;
        }
        while (true)
        {
            auto r = this->getOneRow();
            if (r.empty())
            {
                break;
            }
            else
            {
                res.push_back(r);
            }
        }
        return res;
    }

}




