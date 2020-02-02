//
// Created by fushenshen on 2020/1/30.
//

#ifndef STLLEAN_RESULTDATA_H
#define STLLEAN_RESULTDATA_H


#include <cstring>
#include <binary_log_types.h>


struct ResultData
{
    ResultData(const char *data) : size(strlen(data))
    {
        if (data)
        {
            this->data = new char[this->size + 1];
            strcpy((char *) this->data, data);
        }
    }

    ResultData(const int *data) : type(MYSQL_TYPE_LONG), size(sizeof(int))
    {
        this->size = sizeof(int);
        this->data = reinterpret_cast<const char *>(data);
    }

    ResultData(int size, int type, const char *data);

    ResultData(const ResultData &resultData)
    {
        this->size = resultData.size;
        this->type = resultData.type;
        if (resultData.type != MYSQL_TYPE_LONG)
        {
            this->data = new char[this->size + 1];
            strcpy((char *) this->data, resultData.data);
        }
        else
        {
            this->data = resultData.data;
        }


    }

    ResultData &operator=(const ResultData &resultData)
    {

        this->size = resultData.size;
        this->type = resultData.type;
        if (resultData.type != MYSQL_TYPE_LONG)
        {
            this->data = new char[this->size + 1];
            strcpy((char *) this->data, resultData.data);
        }
        else
        {
            this->data = resultData.data;
        }
        return *this;
    }


    virtual ~ResultData();

    ResultData() = delete;


    bool loadFile(const char *filename);

    bool saveFile(const char *filename);

    int size = 0;
    //默认字段类型为字符串类型
    int type = MYSQL_TYPE_STRING;
    const char *data = nullptr;
};


#endif //STLLEAN_RESULTDATA_H
