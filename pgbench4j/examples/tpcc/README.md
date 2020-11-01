
# TPC-C测试配置例子

数据规模

每个仓库负责十个区域的供货，每个区域 3000 个客户服务，每个仓库维护 100000 种商品的库存纪录。
每个 Warehouse 的数据量，其大小约为 76823.04KB。
可以根据每个Warehouse的数据量，计算测试过程中的数据总量。

计算公式为：数据总量（KB）≈ Warehouse个数*76823.04KB

```
每个仓库数据量大约为76823.04KB即约为76M 
所以 Warehouse=14时，数据量大约为1G 
Warehouse=140时，数据量大约为10G
```

想要加载500G左右的数据，warehouse大约为5000即可，除了表数据还有索引。

## pg12wh1

数据库: PostgreSQL 12

规模: 1个仓库, 每个仓库10个终端


### 建表

```shell script
java -cp lib/* -Dprop=examples/tpcc/pg12wh1/tpcc-pg12wh1.properties -DcommandFile=examples/tpcc/pg12wh1/tpcc-pg12wh1-schema.sql tpcc.jdbc.ExecJDBC
```

### 生成测试数据

```shell script

# 直接入库
java -cp lib/* -Dprop=examples/tpcc/pg12wh1/tpcc-pg12wh1.properties tpcc.data.LoadData numwarehouses 1

# 先生成数据文件, 再导入到数据库
java -cp lib/* -Dprop=examples/tpcc/pg12wh1/tpcc-pg12wh1.properties tpcc.data.LoadData numwarehouses 1 filelocation data/tpcc/pg12wh1
java -cp lib/* -Dprop=examples/tpcc/pg12wh1/tpcc-pg12wh1.properties -DcommandFile=examples/tpcc/pg12wh1/tpcc-pg12wh1-data.sql tpcc.jdbc.ExecJDBC

```

### 执行测试

```shell script
java -cp lib/* -Dprop=examples/tpcc/pg12wh1/tpcc-pg12wh1.properties tpcc.jTPCC
```

### 测试结果

仅供参考

```shell script
[Thread-7] INFO tpcc.jTPCC - Term-00, Measured tpmC (NewOrders) = 2529.09
[Thread-7] INFO tpcc.jTPCC - Term-00, Measured tpmTOTAL = 5639.51
[Thread-7] INFO tpcc.jTPCC - Term-00, Session Start     = 2020-10-30 23:04:47
[Thread-7] INFO tpcc.jTPCC - Term-00, Session End       = 2020-10-30 23:05:47
[Thread-7] INFO tpcc.jTPCC - Term-00, Transaction Count = 5645
```

## h2wh1

数据库: H2

规模: 1个仓库, 每个仓库10个终端

```shell script

java -cp lib/* -Dprop=examples/tpcc/h2wh1/tpcc-h2wh1.properties -DcommandFile=examples/tpcc/h2wh1/tpcc-h2wh1-schema.sql tpcc.jdbc.ExecJDBC

java -cp lib/* -Dprop=examples/tpcc/h2wh1/tpcc-h2wh1.properties tpcc.data.LoadData numwarehouses 1

java -cp lib/* -Dprop=examples/tpcc/h2wh1/tpcc-h2wh1.properties tpcc.jTPCC

```




