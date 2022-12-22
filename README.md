# easyDDB

## start mysql 

cd /home/mysql1
./bin/mysqld --defaults-file=/etc/my1.cnf  --user=mysql --basedir=/home/mysql1 --datadir=/home/mysql1/data  --initialize 

cd /home/mysql2
./bin/mysqld --defaults-file=/etc/my2.cnf  --user=mysql --basedir=/home/mysql2 --datadir=/home/mysql2/data  --initialize

## login mysql
mysql -u root -S /home/mysql1/mysql.sock -p

mysql -u root -S /home/mysql2/mysql.sock -p


## Compile mysql_connector.cpp
g++ mysql_connector.cpp \`mysql_config --cflags --libs\` -o mysql_connector




## 执行阶段简介
首先是前端client可以输入sql语句和config语句，client需要判断是sql语句还是config语句，然后调用rpc的sql接口或者rpc的config接口来进行请求，要求对方返回sql的结果和config的结果，然后client将结果返回给用户。
然后是clusterservice的rpc服务，如果前端发来的是config请求，则采用config调用其接口，如果是create database 请求，则需要执行多个cycle并更新到etcd
之后,

## 