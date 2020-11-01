# pgbench4j

pgbench4j是pgbench的Java简化版, 用于测试PostgreSQL的性能。

# 命令帮助

```shell script

# 查看命令帮助
java -cp lib/* tpcb.PgBench
# 或者
java -cp lib/* tpcb.PgBench --help

# 查看内置脚本列表
# 注: 目前不能指定使用那个脚本.
pgbench4j -b list

```

# 生成表和数据

```shell script

# 初始化100倍数据
pgbench4j -i -s 100
# 等同于, 因为默认数据库地址是localhost:5432, 数据库名、用户名、密码都是postgres 
pgbench4j -i -s 100 --url jdbc:postgresql://localhost:5432/postgres -U postgres -P postgres

```

# 执行测试

```shell script

# 两个线程测试一分钟
pgbench4j -s 100 -j 2 -T 60 --url jdbc:postgresql://localhost:5432/postgres -U postgres -P postgres

```

# 清理环境

```shell script

# 删除测试表
pgbench4j -i -I d --url jdbc:postgresql://localhost:5432/postgres -U postgres -P postgres

```

# 生成表和数据然后执行测试

```shell script

# 初始化100倍数据, 然后启动两个线程, 持续执行测试一分钟
pgbench4j -i -s 100 -j 2 -T 60 --url jdbc:postgresql://localhost:5432/postgres -U postgres -P postgres

```
