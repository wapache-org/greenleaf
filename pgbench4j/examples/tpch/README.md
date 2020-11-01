
# TPC-H测试配置例子

数据量定义:

```
SupplierGenerator.SCALE_BASE = 1_0000;
CustomerGenerator.SCALE_BASE = 15_0000;
PartGenerator.SCALE_BASE     = 20_0000;
OrderGenerator.SCALE_BASE    = 150_0000;

LineItemGenerator.SCALE_BASE     = OrderGenerator.SCALE_BASE;
PartSupplierGenerator.SCALE_BASE = PartGenerator.SCALE_BASE;

totalRowCount = SCALE_BASE * scaleFactor;
```

数据量评估:

```
scale factor = 0.01 : 
  数据量大约为10M, 1.5万订单, 2000零件, 100供应商, 1500客户
scale factor = 0.1 : 
  数据量大约为100M, 15万订单, 2万零件, 1000供应商, 1.5万客户
scale factor = 1 :
  数据量大约为1G, 150万订单, 20万零件, 1万供应商, 15万客户
scale factor = 10 :
  数据量大约为10G, 1500万订单, 200万零件, 10万供应商, 150万客户
```

## h2sf1

数据库: H2

规模因子: 0.01

```shell script
java -cp lib/* tpch.cli.CliTool -c tpch -f tpch_h2sf1.json
```

依次执行生成数据, 建库建表, 加载数据, 执行查询, 数据量查询。

```json
{
  "dataGen": {
    "preProcessScript": "",
    "processMetaFilePath": "examples/tpch/h2sf1/tpch_h2sf1_gen_data.json",
    "postProcessScript": ""
  },
  "sqlExec": [
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/h2sf1/tpch_h2sf1_create_table.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/h2sf1/tpch_h2sf1_load_data.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/h2sf1/tpch_h2sf1_query_table.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/h2sf1/tpch_h2sf1_count_table.json",
      "postProcessScript": ""
    }
  ]
}
```

### 生成测试数据

执行命令:

```shell script
java -cp lib/* tpch.cli.CliTool -c gen_data -f tpch_h2sf1_gen_data.json
```

### 建表

创建TPCH需要的8个表。

```shell script
java -cp lib/* tpch.cli.CliTool -c exec_sql -f tpch_h2sf1_drop_table.json
java -cp lib/* tpch.cli.CliTool -c exec_sql -f tpch_h2sf1_create_table.json
```

drop表是为了防止重复执行导致数据重复。

### 数据加载测试

加载数据到数据库

```shell script
java -cp lib/* tpch.cli.CliTool -c exec_sql -f tpch_h2sf1_load_data.json
```

### 数据查询测试

执行一次TPCH的22个查询.

```shell script
java -cp lib/* tpch.cli.CliTool -c exec_sql -f tpch_h2sf1_query_table.json
```

### 数据量查询测试

查询8个表的数据量.

```shell script
java -cp lib/* tpch.cli.CliTool exec_sql -f tpch_h2sf1_count_table.json
```

## pg12sf1

数据库: PostgreSQL 12

规模因子: 0.01

```shell script
java -cp lib/* tpch.cli.CliTool -c tpch -f examples/tpch/pg12sf1/tpch_pg12sf1.json
```

依次执行生成数据, 建库建表, 加载数据, 执行查询, 数据量查询。

```json
{
  "dataGen": {
    "preProcessScript": "",
    "processMetaFilePath": "examples/tpch/pg12sf1/tpch_pg12sf1_gen_data.json",
    "postProcessScript": ""
  },
  "sqlExec": [
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/pg12sf1/tpch_pg12sf1_create_table.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/pg12sf1/tpch_pg12sf1_load_data.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/pg12sf1/tpch_pg12sf1_query_table.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "examples/tpch/pg12sf1/tpch_pg12sf1_count_table.json",
      "postProcessScript": ""
    }
  ]
}
```

### 生成测试数据

执行命令:

```shell script
java -cp lib/* tpch.cli.CliTool -c gen_data -f examples/tpch/pg12sf1/tpch_pg12sf1_gen_data.json
```

### 建表

创建TPCH需要的8个表。

```shell script
java -cp lib/* tpch.cli.CliTool -c exec_sql -f examples/tpch/pg12sf1/tpch_pg12sf1_drop_table.json
java -cp lib/* tpch.cli.CliTool -c exec_sql -f examples/tpch/pg12sf1/tpch_pg12sf1_create_table.json
```

drop表是为了防止重复执行导致数据重复。

### 数据加载测试

加载数据到数据库

```shell script
java -cp lib/* tpch.cli.CliTool -c exec_sql -f examples/tpch/pg12sf1/tpch_pg12sf1_load_data.json
```

### 数据查询测试

执行一次TPCH的22个查询.

```shell script
java -cp lib/* tpch.cli.CliTool -c exec_sql -f examples/tpch/pg12sf1/tpch_pg12sf1_query_table.json
```

### 数据量查询测试

查询8个表的数据量.

```shell script
java -cp lib/* tpch.cli.CliTool -c exec_sql -f examples/tpch/pg12sf1/tpch_pg12sf1_count_table.json
```
