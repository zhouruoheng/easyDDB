# easyDDB

## start mysql 

cd /home/mysql1
./bin/mysqld --defaults-file=/etc/my1.cnf  --user=mysql --basedir=/home/mysql1 --datadir=/home/mysql1/data  --initialize 

cd /home/mysql2
./bin/mysqld --defaults-file=/etc/my2.cnf  --user=mysql --basedir=/home/mysql2 --datadir=/home/mysql2/data  --initialize

