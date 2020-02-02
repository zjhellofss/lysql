//
// Created by fushenshen on 2020/1/30.
//

#include "ResultData.h"
#include <fstream>

bool ResultData::loadFile(const char *filename)
{
    if (!filename)
    {
        return false;
    }
    std::ifstream f(filename,std::ios::in|std::ios::binary);
    if (!f.is_open())
    {
        return false;
    }
    f.seekg(0, std::ios::end);
    int s = f.tellg();
    this->size = s;
    f.seekg(0, std::ios::beg);
    if (s <= 0)
    {
        return false;
    }
    this->data = new char[this->size + 1];
    int readed = 0;
    while (!f.eof())
    {
        f.read((char *) data + readed, size - readed);
        if (f.gcount() > 0)
        {
            readed += f.gcount();
        }
        else
        {
            break;
        }
    }
    this->type = MYSQL_TYPE_BLOB;
    f.close();
    return true;
}

ResultData::~ResultData()
{
    if (this->type != MYSQL_TYPE_LONG && data != nullptr)
    {
        delete[]data;
        data = nullptr;
    }
}

bool ResultData::saveFile(const char *filename)
{
    if (!data)
    {
        return false;
    }
    std::fstream out(filename, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        return false;
    }
    out.write(this->data, size);
    out.close();
    return true;
}

ResultData::ResultData(int size, int type, const char *data) : size(size), type(type)
{

    if (type != MYSQL_TYPE_LONG)
    {
        this->data = new char[this->size + 1];
        strcpy((char *) this->data, data);
    }
    else
    {
        this->data = data;
    }
}


