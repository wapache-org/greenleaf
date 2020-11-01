# tpch-java

In this project, we introduce a command line tool to support easy usage for generating tpch data and running sqls.
It implements running 'generate tpch data', 'execute sql', 'generate data & execute sql', 'merge statistic reports'.
User can use it to run tpch or their own bussiness sqls by simply configuring some property files and get the pretty statistic report.

The provided functions are show as below:

- [Generate tpch data](#generate-tpch-data)
  - [**Step1**: provide a json file](#step1-provide-a-json-file-that-describes-the-tpch-data-to-be-generated)
    - [Parameters for `DataGenModel`](#parameters-for-datagenmodel)
    - [Parameters for `TableGenModel`](#parameters-for-tablegenmodel)
  - [**Step2**: executing command line](#step2-executing-through-command-line)
  - [**Step2 alternative**: executing code](#step2-alternative-executing-through-java-code)
- [Execute sql](#execute-sql)
  - [**Step1**: provide a json file](#step1-provide-a-json-file-that-describes-the-sqls-to-be-executed)
    - [Parameters for `QueryModel`](#parameters-for-querymodel)
    - [Parameters for `QuerySlice`](#parameters-for-queryslice)
  - [**Step2**: executing command line](#step2-executing-through-command-line)
  - [**Step2 alternative**: executing code](#step2-alternative-executing-through-java-code)
  - [**Step3**: get the report](#step3-get-the-report)
- [Generate data & Execute sql](#generate-data-and-execute-sql)
  - [**Step1**: provide a json file](#step1-provide-a-json-file-that-describes-the-sqls-to-be-executed)
    - [Parameters for `TpchModel`](#parameters-for-tpchmodel)
    - [Parameters for `dataGen`](#parameters-for-datagen)
    - [Parameters for `sqlExec`](#parameters-for-sqlexec)
  - [**Step2**: executing command line](#step2-executing-through-command-line)
  - [**Step2 alternative**: executing code](#step2-alternative-executing-through-java-code)
- [Merge statistic reports](#merge-statistic-reports)
  - [**Step1**: executing command line](#step1-executing-through-command-line)
  - [**Step1 alternative**: executing code](#step1-alternative-executing-through-java-code)

## Generate tpch data

### **Step1**: provide a json file that describes the tpch data to be generated

This file represents a `DataGenModel`. The content looks like below:

```json
{
  "targetDirectory": "data_gen_target",
  "tableGenModels": [
    {
      "tpchTableName": "customer",
      "scaleupFactor": 1,
      "filePartCnt": 2
    },
    {
      "tpchTableName": "lineitem",
      "scaleupFactor": 1,
      "filePartCnt": 3
    }
  ]
}
```

#### Parameters for `DataGenModel`

| Name | DataType | Default | Required | Note |
| --- | --- | --- | --- | --- |
| targetDirectory | String | - | Y | Where will the generated data be written |
| tableGenModels | DataGenModel | - | Y | tpch tables to be generated |

#### Parameters for `TableGenModel`

Each `TableGenModel` represents a TPCH table to be generated.

| Name | DataType | Default | Required | Note |
| --- | --- | --- | --- | --- |
| tpchTableName | String | - | Y | Name of tpch table to be generated |
| scaleupFactor | double | 1 | N | It measures the size of the input data in GB |
| filePartCnt | int | 1 | N | The number of data files will be split into |

### **Step2**: executing through command line

```bash
java -cp tpch-java-1.0-SNAPSHOT-jar-with-dependencies.jar ind.xuchuanyin.tpch.CliTool -c gen_data -f path_to_generate_tpch_data
```
**Note**: To get the jar, you need to build the project using maven.

### **Step2 alternative**: executing through java code

```java


DataGenerator dataGenerator = new DataGenerator();
dataGenerator.setInputFiles(path_to_generate_tpch_data);
dataGenerator.ignite();
dataGenerator.close();
```

## Execute sql

### **Step1**: provide a json file that describes the sqls to be executed

This file represents a `QueryModel`. The content looks like below:

```json
{
  "jdbcUrl": "jdbc:h2:~/test",
  "jdbcUser": "sa",
  "jdbcPwd": "",
  "jdbcDriver": "org.h2.Driver",
  "jdbcPoolSize": 3,
  "execIteration": 1,
  "execInterval": 2,
  "execConcurrentSize": 2,
  "shuffleExecute": true,
  "reportStore": "sql_report1",
  "connInitQuery": "use default",
  "querySlices": [
    {
      "id": "id1",
      "sql": "show tables",
      "type": "type1",
      "threads": 1,
      "isSelectQuery": false,
      "isConsumeResult": true,
      "isCountInStatistics": true
    },
    {
      "id": "id2",
      "sql": "show databases",
      "type": "type2",
      "threads": 5,
      "isSelectQuery": false,
      "isConsumeResult": true,
      "isCountInStatistics": true
    },
    {
      "id": "id3",
      "sql": "show tables;show databases",
      "type": "type3",
      "threads": 2,
      "isSelectQuery": false,
      "isConsumeResult": true,
      "isCountInStatistics": false
    }
  ]
}
```

#### Parameters for `QueryModel`

| Name | DataType | Default | Required | Note |
| --- | --- | --- | --- | --- |
| jdbcUrl | String | - | Y | Url for JDBC connection |
| jdbcUser | String | - | N | User for JDBC connection |
| jdbcPwd | String | - | N | Password for JDBC connection |
| jdbcDriver | String  | - | Y | Driver class for JDBC connection |
| jdbcPoolSize | int | 20 | N | Pool size for connection |
| execIteration | int | 1 | N | Iteration for executing the queries |
| execInterval | int  | 2 | N | Time to wait in seconds before next iteration |
| execConcurrentSize | int  | 1 | N | Pool size for concurrent querying |
| shuffleExecute | boolean  | false | N | Whether to shuffle the sql statements before execution |
| reportStore | String  | - | N | Where to store the statistic result of the query model. If this parameter is not configured, we will not store it. If it is configured, we will write the statistics to a file in both json and table format. Later user can merge different reports |
| connInitQuery | String | - | N | While the connection pool is initialized, each connection will execute this query statement. This is usually for setting some environment variables |
| querySlices | List QuerySlice | - | Y | Each `QuerySlice` represents a block of sql statments that to be executed |

#### Parameters for `QuerySlice`

Each `QuerySlice` represents a specific sql statement.

| Name | DataType | Default | Required | Note |
| --- | --- | --- | --- | --- |
| id | String | q${nanotime} | N | To distinguish a SQL, especially when we shuffle a batch of sqls, we use this id to represent each sql |
| sql | String | - | Y | SQL to be executed |
| type | String | unclassified | N | Type of SQL which will be used in statistics. For each type, a statistics will be performed |
| threads | int | 1 | N | SQL will be executed `threads` times in one iteration |
| isSelectQuery | boolean | true | whether this sql is select query. We add this parameter because that for some database, non-select query cannot execute through execteQuery method |
| isConsumedResult | boolean | true | N | For each sql execution, a ResultSet will be returned. This parameter controls whether we will iterate the ResultSet |
| isCountInStatistics | boolean | true | N | Whether to include this SQL in the statistics |

### **Step2**: executing through command line

```bash
java -cp tpch-java-1.0-SNAPSHOT-jar-with-dependencies.jar ind.xuchuanyin.tpch.CliTool -c exec_sql -f path_to_execute_sql
```

### **Step2 alternative**: executing through java code

```java


QueryClient queryClient = new QueryClient();
queryClient.setInputFiles(path_to_execute_sql);
queryClient.ignite();
queryClient.close();
```

### **Step3**: get the report

If the parameter `reportStore` has been configured, the statistic
report for the `QueryModel` will be written to that directory.
There will be two files, one is of json format which is suffixed with *_rpt.json*,
another is of plain table format which is suffixed with *_rpt.txt*.

We can later merge multiple json reports into one single report and
generate the json and table reports respectively.
You can refer to the `merge statistic reports` section for more information.

The json report looks like below:
```json
[
  {
    "query": "ALL",
    "size": 6,
    "total": 0,
    "min": 0,
    "max": 0,
    "avg": 0.0,
    "quarter": 0,
    "half": 0,
    "three_quarters": 0,
    "ninety": 0,
    "ninety_five": 0
  },
  {
    "query": "type1",
    "size": 1,
    "total": 0,
    "min": 0,
    "max": 0,
    "avg": 0.0,
    "quarter": 0,
    "half": 0,
    "three_quarters": 0,
    "ninety": 0,
    "ninety_five": 0
  },
  {
    "query": "type2",
    "size": 5,
    "total": 0,
    "min": 0,
    "max": 0,
    "avg": 0.0,
    "quarter": 0,
    "half": 0,
    "three_quarters": 0,
    "ninety": 0,
    "ninety_five": 0
  }
]
```

The table report looks like below:
```
+-------+------+-------+-----+-----+-----+-----+-----+-----+-----+-----+
| query | size | total | min | max | avg | 25% | 50% | 75% | 90% | 95% |
+-------+------+-------+-----+-----+-----+-----+-----+-----+-----+-----+
|  ALL  |   6  |   0   |  0  |  0  | 0.0 |  0  |  0  |  0  |  0  |  0  |
+-------+------+-------+-----+-----+-----+-----+-----+-----+-----+-----+
| type1 |   1  |   0   |  0  |  0  | 0.0 |  0  |  0  |  0  |  0  |  0  |
+-------+------+-------+-----+-----+-----+-----+-----+-----+-----+-----+
| type2 |   5  |   0   |  0  |  0  | 0.0 |  0  |  0  |  0  |  0  |  0  |
+-------+------+-------+-----+-----+-----+-----+-----+-----+-----+-----+
```

## Generate data and Execute sql

### **Step1**: provide a json file that describes the sqls to be executed

This file represents a `TpchModel`. The content looks like below:

```json
{
  "dataGen": {
    "preProcessScript": "",
    "processMetaFilePath": "data_gen_meta.json",
    "postProcessScript": ""
  },
  "sqlExec": [
    {
      "preProcessScript": "",
      "processMetaFilePath": "sql_meta1.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "sql_meta2.json",
      "postProcessScript": ""
    },
    {
      "preProcessScript": "",
      "processMetaFilePath": "sql_meta3.json",
      "postProcessScript": ""
    }
  ]
}
```
#### Parameters for `TpchModel`

| Name | DataType | Default | Required | Note |
| --- | --- | --- | --- | --- |
| dataGen | ModelWrapper | - | N | It describes the tpch data to be generated |
| sqlExec | List ModelWrapprt | - | N | It describes the sqls to be executed |

#### Parameters for `dataGen`

| Name | DataType | Default | Required | Note |
| --- | --- | --- | --- | --- |
| preProcessScript | String | - | N | Path to the script to be executed before generating tpch data |
| processMetaFilePath | String | - | Y | Path to the file that describes the `DataGenModel` |
| posteProcessScript | String | - | N | Path to the script to be executed after processing |

#### Parameters for `sqlExec`

| Name | DataType | Default | Required | Note |
| --- | --- | --- | --- | --- |
| preProcessScript | String | - | N | Path to the script file to be executed before processing |
| processMetaFilePath | String | - | Y | Path to the file that describes the `QueryModel` |
| posteProcessScript | String | - | N | Path to the script file to be executed after processing |

### **Step2**: executing through command line

```bash
java -cp tpch-java-1.0-SNAPSHOT-jar-with-dependencies.jar ind.xuchuanyin.tpch.CliTool -c tpch -f path_to_tpch_model
```

### **Step2 alternative**: executing through java code

```java


TpchExecutor tpchExecutor = new TpchExecutor();
tpchExecutor.setInputFiles(path_to_tpch_model);
tpchExecutor.ignite();
tpchExecutor.close();
```

## Merge statistic reports

### **Step1**: executing through command line

```bash
java -cp tpch-java-1.0-SNAPSHOT-jar-with-dependencies.jar ind.xuchuanyin.tpch.CliTool -c merge_report -f report1_json report2_json -o merge_report_out_path
```

### **Step1 alternative**: executing through java code

```java


String mergeResult = HistogramReporter.mergeStatisticFromFile(merge_report_out_path, report1_json, report2_json);
System.out.println(mergeResult);
```
