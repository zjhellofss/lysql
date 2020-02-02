# 基于MySQL的日志采集系统

## 对MySql API的封装 
屏蔽了原生MySQL API繁杂的操作，加快了开发后续的日志采集系统的速度。

## 连接数据库的操作
```c++
    Lysql::Lysql lysql;
    lysql.init();
    lysql.setConnectTimeout(5);
    lysql.connect("127.0.0.1", "root", "xxxxyyyyy", "mydb", 3306, 0);

```

## 建表操作

```c++
string sql = "CREATE TABLE IF NOT EXISTS `t_data`  (\
		`id` int AUTO_INCREMENT,\
		`name` varchar(1024),\
		`size` int,\
        `data` blob,\
		PRIMARY KEY(`id`)\
		) ENGINE =InnoDB";

    ResultData resultData1;
    resultData1.loadFile("../Lysql.cpp");
```


## 插入操作

```c++
 ResultData r2(&resultData1.size);
    std::map<std::string, ResultData> resultData;
    resultData.insert({"name", "Lyxxsql.h"});
    resultData.insert({"size", r2});
    resultData.insert({"data", resultData1.data});
```

## update操作

```c++
    //r为update操作中受影响的条目
    int r = lysql.updateBin(resultData, "t_data", "id=14");
```

## 事务操作

```c++
    lysql.startTransaction();
    lysql.stopTransaction();
    lysql.commit();
```

## 日志审计系统 
基于这个库写出了一套日志收集和审计系统，对Linux系统中符合策略的日志进行录入和读取,并最后供用户查看。<br/>日志收集的过程不需要人为参与均为自动是自动进行的。
只需要将agent和center部署在服务端，将client部署在客户端，在登录之后就可以对审计的结果进行查看。

