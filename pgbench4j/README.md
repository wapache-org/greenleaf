
# pgbench4j

`pgbench4j`原本定位是用Java语言实现的一个简易版的`pgbench`, 经过一段时间的积累, 目前已经包含了 TPC-B, TPC-C 和 TPC-H 3种TPC基准测试功能。

`pgbench4j`的大部分初始代码来源于以下开源项目:

1. [pgbench](https://www.postgresql.org/docs/current/pgbench.html)

    pgbench是PostgreSQL自带的运行基准测试的简单程序, 是一个类TPC-B的基准测试工具，可以执行内置的测试脚本，也可以自定义脚本文件。
    本项目参考其 [C代码](https://github.com/postgres/postgres/blob/master/src/bin/pgbench/pgbench.c) 实现了它的核心功能, 并以此作为TPC-B基准测试的一种实现。
    
2. [BenchmarkSQL](https://sourceforge.net/projects/benchmarksql)

    对于BenchmarkSQL，官方的解释如下：
    
    > An easy to use JDBC benchmark that closely resembles the TPC-C standard for OLTP. 
    > DB's supported include PostgreSQL/EnterpriseDB, DB2, Oracle, SQLSvr, MySQL.
    
    简单点说，就是一个通过JDBC测试OLTP性能的TPC-C。 
    支持PostgreSQL/EnterpriseDB, DB2, Oracle, SQLSvr, MySQL.
    本项目加入了copy命令支持, 并以此作为TPC-C基准测试的一种实现。
    
3. [prestosql/tpch](https://github.com/prestosql/tpch)

    是官方`dbgen`数据工具的Java版实现。
    它生成的数据文件每行后面都多了一个`|`, 无法直接用copy命令导入, 本项目已将最后的`|`去掉。
    
4. [tpch-java](https://github.com/xuchuanyin/tpch-java)

    > a command line tool to support easy usage for generating tpch data and running sqls.
    > It implements running 'generate tpch data', 'execute sql', 'generate data & execute sql', 'merge statistic reports'.

    `tpch-java`是一个命令行工具, 旨在提供一个更加易用的TPC-H数据生成工具和测试执行工具。
    原版的`tpch-java`只支持执行静态SQL, 本项目加入了copy命令的支持和动态生成查询SQL支持, 并以此作为TPC-H基准测试的一种实现。

以上开源项目基本都不太活跃, 有的已经好几年没更新了, 
本项目通过整合这些项目的现存代码, 按TPC标准重新整理代码结构, 新目标是成为一个涵盖TPC主流基准测试的工具。

3种基准测试的测试例子存放在`examples`目录下:

1. [TPC-B例子的说明文档](examples/tpcb/README.md)
1. [TPC-C例子的说明文档](examples/tpcc/README.md)
1. [TPC-H例子的说明文档](examples/tpch/README.md)

## 数据库基准测试 - Database Benchmarking

### 行业标准数据库基准 - Industry Standard Database Benchmarks

**行业标准数据库基准**通常被企业用于比较不同的硬件和软件系统的性能，以满足采购相关的需要。
它是比较各种硬件和软件组合在一个明确定义的场景中运行的性能的快速方法。

### Transaction Processing Council (TPC)

[Transaction Processing Council（事务处理委员会）](http://www.tpc.org/) 是一个非营利性的公司，它支持一个由硬件和数据库软件供应商组成的联合会，致力于确定事务处理和数据库相关基准。
TPC基准背后的主要目标是界定可在任何数据库上运行的功能要求，无论硬件或操作系统如何。
这使得供应商可以实现自己的基准工具包，以满足功能要求。
同样，在公开提交证据证明基准是根据规范进行的之后，最终用户更有把握知道他们得到的是有效的、苹果对苹果的比较。

[规范和源码下载](https://www.tpc.org/tpc_documents_current_versions/current_specifications5.asp)

针对数据库不同的使用场景TPC组织发布了多项测试标准。其中被业界广泛接受和使用的有TPC-C 、TPC-H和TPC-DS。

#### TPC Benchmark B (TPC-B)

TPC-B基准强调的是数据库，其特点是大量的磁盘输入/输出，适度的系统、应用程序执行时间和事务完整性。 
该基准针对数据库管理系统(DBMS)批处理应用程序和后端数据库服务器。 
TPC-B不是OLTP基准。

#### TPC Benchmark C (TPC-C)

TPC-C于1992年7月获得批准，是针对OLTP的基准测试。TPC-C将很快被淘汰，取而代之的是TPC-E。

TPC-C模拟的是一个订单输入环境，在这个环境中，一群终端操作人员通过数据库执行交易。
基准由交易组成，包括输入和交付订单、记录付款、检查订单状态和监测仓库的库存水平。
最频繁的交易包括输入一个新的订单，平均由10个不同的项目组成。
每个仓库都试图维持公司目录中10万种商品的库存，并利用这些库存来完成订单。
TPC-C所报告的业绩指标是衡量每分钟可完全处理的订单数量，用tpm-C表示。

TPC-C的测试结果主要有两个指标：

①流量指标(Throughput，简称tpmC)

按照TPC的定义，流量指标描述了系统在执行Payment、Order-status、Delivery、Stock-Level这四种交易的同时，每分钟可以处理多少个New-Order交易。
所有交易的响应时间必须满足TPC-C测试规范的要求。

流量指标值越大越好！

②性价比(Price/Performance，简称Price/tpmC)

即测试系统价格（指在美国的报价）与流量指标的比值。

性价比越大越好！

#### TPC Benchmark E (TPC-E)

[TPC-E](http://www.tpc.org/tpce/default5.asp) 基准模拟了一家经纪公司的OLTP工作量。
该基准的重点是执行与公司客户账户有关的交易的中央数据库。
TPC-E指标以每秒交易量（tps）为单位。
它具体指的是服务器在一段时间内可以维持的交易结果交易数量。

与上一代TPC-C测试模型相比，TPC-E更加强调模型的高仿真性，其模型微缩模拟了全球最大电子股票交易市场---美国纳斯达克股市的日常业务流程，模型架构完成了从C/S架构到B/S架构的过渡，是典型的Internet时代OLTP性能测试基准。同时，数据类型更加丰富，由3类扩展为10类，模拟的交易条件更复杂。因此，从理论上说，TPC-E的测试结果对于金融、电信、证券等高端行业用户采购服务器更有参考价值。
TPC-E评测的是整体方案的性能，这个方案包括服务器、存储、OS、数据库、客户端等软硬件在内的一整套系统，不仅仅是CPU的性能，服务器系统设计、操作系统与数据库软件、存储架构等都非常关键。

#### TPC Benchmark H (TPC-H)

[TPC-H](http://www.tpc.org/tpch/default5.asp) 是一个决策支持的基准测试。它由一套面向业务的即席查询和并发数据修改组成。 

TPC-H是一款面向商品零售业的决策支持系统测试基准，它定义了8张表，22个查询。

1. Q1：价格统计报告查询
2. Q2: 最小代价供货商查询
3. Q3: 运送优先级查询
4. Q4: 订单优先级查询
5. Q5: 某地区供货商为公司带来的收入查询
6. Q6: 预测收入变化查询
7. Q7: 货运盈利情况查询
8. Q8: 国家市场份额查询
9. Q9: 产品类型利润估量查询
10. Q10: 货运存在问题的查询
11. Q11: 库存价值查询
12. Q12: 货运模式和订单优先级查询
13. Q13: 消费者订单数量查询
14. Q14: 促销效果查询
15. Q15: 头等供货商查询
16. Q16: 零件/供货商关系查询
17. Q17: 小订单收入查询
18. Q18: 大订单顾客查询
19. Q19: 折扣收入查询
20. Q20: 供货商竞争力查询
21. Q21: 不能按时交货供货商查询
22. Q22: 全球销售机会查询

[22条SQL语句分析](https://yq.aliyun.com/articles/149715)

TPC-H报告的性能指标称为TPC-H每小时综合查询性能指标（QphH @ Size），并反映了系统处理查询的能力的多方面。 
这些方面包括执行查询所选的数据库大小，单个流提交查询时的查询处理能力，以及多个并发用户提交查询时的查询吞吐量。 
TPC-H的价格/性能指标表示为$ / QphH @ Size。

新兴的数据仓库开始采用新的模型，如星型模型、雪花模型。
TPC-H已经不能精准反映当今数据库系统的真实性能。
为此，TPC组织推出了新一代的面向决策应用的TPC-DS 基准。

#### TPC Benchmark DS (TPC-DS)

TPC-DS是决策支持的基本测试，提供了决策支持系统的通用见面方式，包括数据查询和数据维护。
TPC-DS基准测试提供了通用决策支持系统的性能评估。
基准测试的结果衡量了单用户模式下的响应时间，多用户模式下的查询吞吐量，特定操作系统和硬件的数据维护性能，在受限复杂的环境下数据处理系统、支持多用户决策。
TPC-DS基准测试为用户提供相关的 客观性能数据。
TPC-DS v2则支持新兴技术如大数据 进行性能测试。

tpc.org 官方也提供规范和测试包。

TPC-DS采用星型、雪花型等多维数据模式。
它包含7张事实表，17张维度表平均每张表含有18列。
其工作负载包含99个SQL查询，覆盖SQL99和2003的核心部分以及OLAP。
这个测试集包含对大数据集的统计、报表生成、联机查询、数据挖掘等复杂应用，测试用的数据和值是有倾斜的，与真实数据一致。
可以说TPC-DS是与真实场景非常接近的一个测试集，也是难度较大的一个测试集。

TPC-DS的这个特点跟大数据的分析挖掘应用非常类似。
Hadoop等大数据分析技术也是对海量数据进行大规模的数据分析和深度挖掘，也包含交互式联机查询和统计报表类应用，同时大数据的数据质量也较低，数据分布是真实而不均匀的。
因此TPC-DS成为客观衡量多个不同Hadoop版本以及SQL on Hadoop技术的最佳测试集。这个基准测试有以下几个主要特点：

 * 一共99个测试案例，遵循SQL’99和SQL 2003的语法标准，SQL案例比较复杂
 * 分析的数据量大，并且测试案例是在回答真实的商业问题
 * 测试案例中包含各种业务模型（如分析报告型，迭代式的联机分析型，数据挖掘型等）
 * 几乎所有的测试案例都有很高的IO负载和CPU计算需求


## 参考资料 

### TPC-C

1. [TPC-C基准测试简介](http://blog.sina.com.cn/s/blog_4485748101019wsh.html)
1. [从TPC-C到TPC-E测试](http://blog.chinaunix.net/uid-354915-id-3868465.html)
1. [TPC-E的一篇论文](http://www.docin.com/p-1345576360.html)

### TPC-H

1. [TPC-H 使用](https://blog.csdn.net/leixingbang1989/article/details/8766047)
1. [TPC-H（二）：22个SQL语句说明（基于TPC-H2.17.3版本）](https://blog.csdn.net/LCYong_/article/details/79894478)
1. [damaoguo/TPC-H](https://github.com/damaoguo/TPC-H)
1. [github上的tpch](https://github.com/topics/tpch)
1. [tpch-citus](https://github.com/dimitri/tpch-citus)

