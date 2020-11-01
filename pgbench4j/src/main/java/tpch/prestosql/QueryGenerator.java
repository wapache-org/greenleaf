package tpch.prestosql;

import java.util.List;

import static com.google.common.base.Preconditions.checkArgument;

public class QueryGenerator {

    /**
     *
     --SF，Scale Factor ，数据库的比例因子。TPC-H标准规定，测试数据库的比例因子必须从下列固定值中选择：1，10,30,100,1000,3000,10000 (相当于1GB，10GB,30GB,100GB,1000GB,3000GB,10000GB)。数据库的大小缺省定义为1（例如：SF＝1；近似于1GB）。
     */
    private final int scaleFactor;

    private final RandomString marketSegmentRandom;

    private final RandomString regionRandom;
    private final RandomString nationRandom;
    private final RandomString nation2Random;

    private final RandomString partTypeRandom;
    private final RandomString partColorRandom;
    private final RandomString partContainerRandom;

    private final RandomString shipModeRandom;

    private final RandomBoundedInt randomShipDate60_120;
    private final RandomBoundedInt randomSize1_50;

    private final RandomBoundedInt randomDiscount2_9;

    private final RandomBoundedInt randomNationCode10_34;

    private final RandomBoundedInt randomQuantity1_10;
    private final RandomBoundedInt randomQuantity10_20;
    private final RandomBoundedInt randomQuantity20_30;
    private final RandomBoundedInt randomQuantity24_25;
    private final RandomBoundedInt randomQuantity312_315;

    private final RandomBoundedInt randomBrand1_5;

    private final RandomBoundedInt randomYear1993_1994;
    private final RandomBoundedInt randomYear1993_1997;
    private final RandomBoundedInt randomMonth1_12;
    private final RandomBoundedInt randomMonth1_10;
    private final RandomBoundedInt randomDate1_31;

    private final RandomString q13aRandom;
    private final RandomString q13bRandom;

    public QueryGenerator(int scaleFactor) {
        this(scaleFactor, Distributions.getDefaultDistributions(), TextPool.getDefaultTestPool());
    }

    public QueryGenerator(int scaleFactor, Distributions distributions, TextPool textPool) {

        checkArgument(scaleFactor > 0, "scaleFactor must be greater than 0");

        this.scaleFactor = scaleFactor;

        int expectedRowCount = 10;

//        this.distributions = requireNonNull(distributions, "distributions is null");
//        this.textPool = requireNonNull(textPool, "textPool is null");

        partTypeRandom = new RandomString(1841581359, distributions.getPartTypes(), expectedRowCount);
        partColorRandom = new RandomString(1841581359, distributions.getPartColors(), expectedRowCount);

        partContainerRandom = new RandomString(1841581359, distributions.getPartContainers(), expectedRowCount);

        regionRandom = new RandomString(1841581359, distributions.getRegions(), expectedRowCount);
        nationRandom = new RandomString(1841581359, distributions.getNations(), expectedRowCount);
        nation2Random = new RandomString(1841581359, distributions.getNations2(), expectedRowCount);

//        nationKeyRandom = new RandomBoundedInt(1489529863, 0, distributions.getNations().size() - 1);
        marketSegmentRandom = new RandomString(1140279430, distributions.getMarketSegments(), expectedRowCount);
        shipModeRandom = new RandomString(1140279430, distributions.getShipModes(), expectedRowCount);

        q13aRandom = new RandomString(1841581359, distributions.getQ13a(), expectedRowCount);
        q13bRandom = new RandomString(1841581359, distributions.getQ13b(), expectedRowCount);

        randomShipDate60_120 = new RandomBoundedInt(1489529863, 60, 120, expectedRowCount);
        randomSize1_50 = new RandomBoundedInt(1489529864, 1, 50, expectedRowCount);

        randomDiscount2_9 = new RandomBoundedInt(1489529864, 2, 9, expectedRowCount);

        randomNationCode10_34 = new RandomBoundedInt(1489529864, 10, 34, expectedRowCount);

        randomQuantity24_25 = new RandomBoundedInt(1489529864, 24, 25, expectedRowCount);
        randomQuantity312_315 = new RandomBoundedInt(1489529864, 312, 315, expectedRowCount);
        randomQuantity1_10 = new RandomBoundedInt(1489529864, 1, 10, expectedRowCount);
        randomQuantity10_20 = new RandomBoundedInt(1489529864, 10, 20, expectedRowCount);
        randomQuantity20_30 = new RandomBoundedInt(1489529864, 20, 30, expectedRowCount);

        randomYear1993_1994 = new RandomBoundedInt(1489529844, 1993, 1994, expectedRowCount);
        randomYear1993_1997 = new RandomBoundedInt(1489529844, 1993, 1997, expectedRowCount);
        randomMonth1_12 = new RandomBoundedInt(1489529844, 1, 12, expectedRowCount);
        randomMonth1_10 = new RandomBoundedInt(1489529844, 1, 10, expectedRowCount);
        randomDate1_31 = new RandomBoundedInt(1489529844, 1, 31, expectedRowCount);

        randomBrand1_5 = new RandomBoundedInt(1489529844, 1, 5, expectedRowCount);
    }

    public String[] generateQuerys(){
        return new String[]{

            genQ1PricingSummaryReportQuery(),
            genQ2MinimumCostSupplierQuery(),
            genQ3ShippingPriorityQuery(),
            genQ4OrderPriorityCheckingQuery(),
            genQ5(),

            genQ6ForecastingRevenueChangeQuery(),
            genQ7VolumeShippingQuery(),
            genQ8NationalMarketShareQuery(),
            genQ9ProductTypeProfitMeasureQuery(),
            genQ10(),

            genQ11(),
            genQ12ShippingModesAndOrderPriorityQuery(),
            genQ13CustomerDistributionQuery(),
            genQ14PromotionEffectQuery(),
            genQ15TopSupplierQuery(),

            genQ16PartsSupplierRelationshipQuery(),
            genQ17SmallQuantityOrderRevenueQuery(),
            genQ18LargeVolumeCustomerQuery(),
            genQ19DiscountedRevenueQuery(),
            genQ20PotentialPartPromotionQuery(),

            genQ21SuppliersWhoKeptOrdersWaitingQuery(),
            genQ22()
        };
    }

    public String generateQuery(int query){
        switch(query){
            case 1: return genQ1PricingSummaryReportQuery();
            case 2: return genQ2MinimumCostSupplierQuery();
            case 3: return genQ3ShippingPriorityQuery();
            case 4: return genQ4OrderPriorityCheckingQuery();
            case 5: return genQ5();
            case 6: return genQ6ForecastingRevenueChangeQuery();
            case 7: return genQ7VolumeShippingQuery();
            case 8: return genQ8NationalMarketShareQuery();
            case 9: return genQ9ProductTypeProfitMeasureQuery();
            case 10: return genQ10();
            case 11: return genQ11();
            case 12: return genQ12ShippingModesAndOrderPriorityQuery();
            case 13: return genQ13CustomerDistributionQuery();
            case 14: return genQ14PromotionEffectQuery();
            case 15: return genQ15TopSupplierQuery();
            case 16: return genQ16PartsSupplierRelationshipQuery();
            case 17: return genQ17SmallQuantityOrderRevenueQuery();
            case 18: return genQ18LargeVolumeCustomerQuery();
            case 19: return genQ19DiscountedRevenueQuery();
            case 20: return genQ20PotentialPartPromotionQuery();
            case 21: return genQ21SuppliersWhoKeptOrdersWaitingQuery();
            case 22: return genQ22();
            default:
                return null;
        }
    }

    /**
     * TPC-H/TPC-R Pricing Summary Report Query (Q1)
     *
     * Q1：价格统计报告查询
     *
     * Q1语句是查询lineItems的一个定价总结报告。
     * 在单个表lineitem上查询某个时间段内，
     * 对已经付款的、已经运送的等各类商品进行统计，
     * 包括业务量的计费、发货、折扣、税、平均价格等信息。
     *
     * Q1语句的特点是：
     * 带有分组、排序、聚集操作并存的单表查询操作。
     * 这个查询会导致表上的数据有95%到97%行被读取到。
     *
     * @return
     */
    String genQ1PricingSummaryReportQuery(){

        String sql = "" +
            "select                                                                  \n" +
            "    l_returnflag,                                                       \n" +
            "    l_linestatus,                                                       \n" +
            "    sum(l_quantity) as sum_qty,                                         \n" +
            "    sum(l_extendedprice) as sum_base_price,                             \n" +
            "    sum(l_extendedprice * (1 - l_discount)) as sum_disc_price,          \n" +
            "    sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge,\n" +
            "    avg(l_quantity) as avg_qty,                                         \n" +
            "    avg(l_extendedprice) as avg_price,                                  \n" +
            "    avg(l_discount) as avg_disc,                                        \n" +
            "    count(*) as count_order                                             \n" +
            "from                                                                    \n" +
            "    lineitem                                                            \n" +
            "where                                                                   \n" +
            "    l_shipdate <= date '1998-12-01' - interval ':1 days'                \n" +
            "group by                                                                \n" +
            "    l_returnflag,                                                       \n" +
            "    l_linestatus                                                        \n" +
            "order by                                                                \n" +
            "    l_returnflag,                                                       \n" +
            "    l_linestatus                                                        \n";

        sql = sql.replace(":1", String.valueOf(randomShipDate60_120.nextValue()));

        return sql;
    }


    /**
     * Q2: 最小代价供货商查询
     * Q2语句查询获得最小代价的供货商。得到给定的区域内，对于指定的零件（某一类型和大小的零件），哪个供应者能以最低的价格供应它，就可以选择哪个供应者来订货。
     * Q2语句的特点是：带有排序、聚集操作、子查询并存的多表查询操作。查询语句没有从语法上限制返回多少条元组，但是TPC-H标准规定，查询结果只返回前100行（通常依赖于应用程序实现）。
     *
     * @return
     */
    String genQ2MinimumCostSupplierQuery(){
        String sql = "" +
            "select                                   \n" +
            "    s_acctbal,                           \n" +
            "    s_name,                              \n" +
            "    n_name,                              \n" +
            "    p_partkey,                           \n" +
            "    p_mfgr,                              \n" +
            "    s_address,                           \n" +
            "    s_phone,                             \n" +
            "    s_comment                            \n" +
            "from                                     \n" +
            "    part,                                \n" +
            "    supplier,                            \n" +
            "    partsupp,                            \n" +
            "    nation,                              \n" +
            "    region                               \n" +
            "where                                    \n" +
            "    p_partkey = ps_partkey               \n" +
            "    and s_suppkey = ps_suppkey           \n" +
            "    and p_size = :1                      \n" + // 指定大小，在区间[1, 50]内随机选择
            "    and p_type like '%:2'                \n" + // 指定类型，在TPC-H标准指定的范围内随机选择
            "    and s_nationkey = n_nationkey        \n" +
            "    and n_regionkey = r_regionkey        \n" +
            "    and r_name = ':3'                    \n" + // 指定地区，在TPC-H标准指定的范围内随机选择
            "    and ps_supplycost = (                \n" +
            "        select                           \n" +
            "            min(ps_supplycost)           \n" +
            "        from                             \n" +
            "            partsupp,                    \n" +
            "            supplier,                    \n" +
            "            nation,                      \n" +
            "            region                       \n" +
            "        where                            \n" +
            "            p_partkey = ps_partkey       \n" +
            "            and s_suppkey = ps_suppkey   \n" +
            "            and s_nationkey = n_nationkey\n" +
            "            and n_regionkey = r_regionkey\n" +
            "            and r_name = ':3'            \n" +
            "    )                                    \n" +
            "order by                                 \n" +
            "    s_acctbal desc,                      \n" +
            "    n_name,                              \n" +
            "    s_name,                              \n" +
            "    p_partkey                            \n" +
            "limit 100                                ";

        sql = sql.replace(":1", String.valueOf(randomSize1_50.nextValue()));
        sql = sql.replace(":2", partTypeRandom.nextValue());
        sql = sql.replace(":3", regionRandom.nextValue());

        return sql;
    }

    /**
     * --Q3: 运送优先级查询
     * --Q3语句查询得到收入在前10位的尚未运送的订单。在指定的日期之前还没有运送的订单中具有最大收入的订单的运送优先级（订单按照收入的降序排序）和潜在的收入（潜在的收入为l_extendedprice * (1-l_discount)的和）。
     * --Q3语句的特点是：带有分组、排序、聚集操作并存的三表查询操作。查询语句没有从语法上限制返回多少条元组，但是TPC-H标准规定，查询结果只返回前10行（通常依赖于应用程序实现）。
     *
     * @return
     */
    String genQ3ShippingPriorityQuery(){
        String sql = "" +
            "select                                                 \n" +
            "    l_orderkey,                                        \n" +
            "    sum(l_extendedprice * (1 - l_discount)) as revenue,\n" +
            "    o_orderdate,                                       \n" +
            "    o_shippriority                                     \n" +
            "from                                                   \n" +
            "    customer,                                          \n" +
            "    orders,                                            \n" +
            "    lineitem                                           \n" +
            "where                                                  \n" +
            "    c_mktsegment = ':1'                                \n" + // 在TPC-H标准指定的范围内随机选择
            "    and c_custkey = o_custkey                          \n" +
            "    and l_orderkey = o_orderkey                        \n" +
            "    and o_orderdate < date ':2'                        \n" + // 指定日期段，在[1995-03-01, 1995-03-31]中随机选择
            "    and l_shipdate > date ':2'                         \n" +
            "group by                                               \n" +
            "    l_orderkey,                                        \n" +
            "    o_orderdate,                                       \n" +
            "    o_shippriority                                     \n" +
            "order by                                               \n" +
            "    revenue desc,                                      \n" +
            "    o_orderdate                                        \n" +
            "limit 10                                               ";

        sql = sql.replace(":1", marketSegmentRandom.nextValue());
        sql = sql.replace(":2", String.format("1995-03-%02d", randomDate1_31.nextValue()));

        return sql;
    }

    /**
     *
     --Q4: 订单优先级查询
     --Q4语句查询得到订单优先级统计值。计算给定的某三个月的订单的数量，在每个订单中至少有一行由顾客在它的提交日期之后收到。
     --Q4语句的特点是：带有分组、排序、聚集操作、子查询并存的单表查询操作。子查询是相关子查询。
     * @return
     */
    String genQ4OrderPriorityCheckingQuery(){
        String sql = "" +
            "select                                              \n" +
            "    o_orderpriority,                                \n" +
            "    count(*) as order_count                         \n" +
            "from                                                \n" +
            "    orders                                          \n" +
            "where                                               \n" +
            "    o_orderdate >= date ':1'                        \n" + // 指定订单的时间段--某三个月，
            "    and o_orderdate < date ':1' + interval '3' month\n" + // DATE是在1993年1月和1997年10月之间随机选择的一个月的第一天
            "    and exists (                                    \n" +
            "        select                                      \n" +
            "            *                                       \n" +
            "        from                                        \n" +
            "            lineitem                                \n" +
            "        where                                       \n" +
            "            l_orderkey = o_orderkey                 \n" +
            "            and l_commitdate < l_receiptdate        \n" +
            "    )                                               \n" +
            "group by                                            \n" +
            "    o_orderpriority                                 \n" +
            "order by                                            \n" +
            "    o_orderpriority                                 ";

        sql = sql.replace(":1", String.format("%d-%02d-01", randomYear1993_1997.nextValue(), randomMonth1_10.nextValue()));

        return sql;
    }

    /**
     *
     --Q5: 某地区供货商为公司带来的收入查询
     --Q5语句查询得到通过某个地区零件供货商而获得的收入（收入按sum(l_extendedprice * (1 -l_discount))计算）统计信息。可用于决定在给定的区域是否需要建立一个当地分配中心。
     --Q5语句的特点是：带有分组、排序、聚集操作、子查询并存的多表连接查询操作。
     * @return
     */
    String genQ5(){
        String sql = "" +
            "select                                                \n" +
            "    n_name,                                           \n" +
            "    sum(l_extendedprice * (1 - l_discount)) as revenue\n" +
            "from                                                  \n" +
            "    customer,                                         \n" +
            "    orders,                                           \n" +
            "    lineitem,                                         \n" +
            "    supplier,                                         \n" +
            "    nation,                                           \n" +
            "    region                                            \n" +
            "where                                                 \n" +
            "    c_custkey = o_custkey                             \n" +
            "    and l_orderkey = o_orderkey                       \n" +
            "    and l_suppkey = s_suppkey                         \n" +
            "    and c_nationkey = s_nationkey                     \n" +
            "    and s_nationkey = n_nationkey                     \n" +
            "    and n_regionkey = r_regionkey                     \n" +
            "    and r_name = ':1'                                 \n" + // 指定地区，在TPC-H标准指定的范围内随机选择
            "    and o_orderdate >= date ':2'                      \n" + // DATE是从1993年到1997年中随机选择的一年的1月1日
            "    and o_orderdate < date ':2' + interval '1' year   \n" +
            "group by                                              \n" +
            "    n_name                                            \n" +
            "order by                                              \n" +
            "    revenue desc                                      ";

        sql = sql.replace(":1", regionRandom.nextValue());
        sql = sql.replace(":2", String.format("%d-01-01", randomYear1993_1997.nextValue()));

        return sql;
    }

    /**
     * --Q6: 预测收入变化查询
     * --Q6语句查询得到某一年中通过变换折扣带来的增量收入。这是典型的“what-if”判断，用来寻找增加收入的途径。预测收入变化查询考虑了指定的一年中折扣在“DISCOUNT-0.01”和“DISCOUNT＋0.01”之间的已运送的所有订单，求解把l_quantity小于quantity的订单的折扣消除之后总收入增加的数量。
     * --Q6语句的特点是：带有聚集操作的单表查询操作。查询语句使用了BETWEEN-AND操作符，有的数据库可以对BETWEEN-AND进行优化。
     *
     * @return
     */
    String genQ6ForecastingRevenueChangeQuery(){
        String sql = "" +
            "select                                            \n" +
            "    sum(l_extendedprice * l_discount) as revenue  \n" +
            "from                                              \n" +
            "    lineitem                                      \n" +
            "where                                             \n" +
            "    l_shipdate >= date ':1'                       \n" + // 从[1993, 1997]中随机选择的一年的1月1日
            "    and l_shipdate < date ':1' + interval '1' year\n" + // 在区间[2, 9]中随机选择
            "    and l_discount between :2 - 0.01 and :2 + 0.01\n" + // 在区间[24, 25]中随机选择
            "    and l_quantity < :3                           ";

        sql = sql.replace(":1", String.format("%d-01-01", randomYear1993_1997.nextValue()));
        sql = sql.replace(":2", String.valueOf(randomDiscount2_9.nextValue()));
        sql = sql.replace(":3", String.valueOf(randomQuantity24_25.nextValue()));

        return sql;
    }

    /**
     *
     --Q7: 货运盈利情况查询
     --Q7语句是查询从供货商国家与销售商品的国家之间通过销售获利情况的查询。此查询确定在两国之间货运商品的量用以帮助重新谈判货运合同。
     --Q7语句的特点是：带有分组、排序、聚集、子查询操作并存的多表查询操作。子查询的父层查询不存在其他查询对象，是格式相对简单的子查询。

     * @return
     */
    String genQ7VolumeShippingQuery(){
        String sql = "" +
            "select                                                                    \n" +
            "    supp_nation,                                                          \n" +
            "    cust_nation,                                                          \n" +
            "    l_year,                                                               \n" +
            "    sum(volume) as revenue                                                \n" +
            "from                                                                      \n" +
            "    (                                                                     \n" +
            "        select                                                            \n" +
            "            n1.n_name as supp_nation,                                     \n" +
            "            n2.n_name as cust_nation,                                     \n" +
            "            extract(year from l_shipdate) as l_year,                      \n" +
            "            l_extendedprice * (1 - l_discount) as volume                  \n" +
            "        from                                                              \n" +
            "            supplier,                                                     \n" +
            "            lineitem,                                                     \n" +
            "            orders,                                                       \n" +
            "            customer,                                                     \n" +
            "            nation n1,                                                    \n" +
            "            nation n2                                                     \n" +
            "        where                                                             \n" +
            "            s_suppkey = l_suppkey                                         \n" +
            "            and o_orderkey = l_orderkey                                   \n" +
            "            and c_custkey = o_custkey                                     \n" +
            "            and s_nationkey = n1.n_nationkey                              \n" +
            "            and c_nationkey = n2.n_nationkey                              \n" +
            "            and (                                                         \n" +
            "                (n1.n_name = ':1' and n2.n_name = ':2')                   \n" + // NATION2和NATION1的值不同，表示查询的是跨国的货运情况
            "                or (n1.n_name = ':2' and n2.n_name = ':1')                \n" +
            "            )                                                             \n" +
            "            and l_shipdate between date '1995-01-01' and date '1996-12-31'\n" +
            "    ) as shipping                                                         \n" +
            "group by                                                                  \n" +
            "    supp_nation,                                                          \n" +
            "    cust_nation,                                                          \n" +
            "    l_year                                                                \n" +
            "order by                                                                  \n" +
            "    supp_nation,                                                          \n" +
            "    cust_nation,                                                          \n" +
            "    l_year                                                                ";

        sql = sql.replace(":1", nationRandom.nextValue());
        sql = sql.replace(":2", nationRandom.nextValue());

        return sql;
    }

    /**
     *
     --Q8: 国家市场份额查询
     --Q8语句是查询在过去的两年中一个给定零件类型在某国某地区市场份额的变化情况。
     --Q8语句的特点是：带有分组、排序、聚集、子查询操作并存的查询操作。子查询的父层查询不存在其他查询对象，是格式相对简单的子查询，但子查询自身是多表连接的查询。

     * @return
     */
    String genQ8NationalMarketShareQuery(){
        String sql = "" +
            "select                                                                     \n" +
            "    o_year,                                                                \n" +
            "    sum(case                                                               \n" +
            "        when nation = ':1' then volume                                     \n" + // 指定国家，在TPC-H标准指定的范围内随机选择
            "        else 0                                                             \n" +
            "    end) / sum(volume) as mkt_share                                        \n" +
            "from                                                                       \n" +
            "    (                                                                      \n" +
            "        select                                                             \n" +
            "            extract(year from o_orderdate) as o_year,                      \n" +
            "            l_extendedprice * (1 - l_discount) as volume,                  \n" +
            "            n2.n_name as nation                                            \n" +
            "        from                                                               \n" +
            "            part,                                                          \n" +
            "            supplier,                                                      \n" +
            "            lineitem,                                                      \n" +
            "            orders,                                                        \n" +
            "            customer,                                                      \n" +
            "            nation n1,                                                     \n" +
            "            nation n2,                                                     \n" +
            "            region                                                         \n" +
            "        where                                                              \n" +
            "            p_partkey = l_partkey                                          \n" +
            "            and s_suppkey = l_suppkey                                      \n" +
            "            and l_orderkey = o_orderkey                                    \n" +
            "            and o_custkey = c_custkey                                      \n" +
            "            and c_nationkey = n1.n_nationkey                               \n" +
            "            and n1.n_regionkey = r_regionkey                               \n" +
            "            and r_name = ':2'                                              \n" + // 指定地区，在TPC-H标准指定的范围内随机选择
            "            and s_nationkey = n2.n_nationkey                               \n" +
            "            and o_orderdate between date '1995-01-01' and date '1996-12-31'\n" +
            "            and p_type = ':3'                                              \n" + // 指定零件类型，在TPC-H标准指定的范围内随机选择
            "    ) as all_nations                                                       \n" +
            "group by                                                                   \n" +
            "    o_year                                                                 \n" +
            "order by                                                                   \n" +
            "    o_year                                                                 ";

        sql = sql.replace(":1", nationRandom.nextValue());
        sql = sql.replace(":2", regionRandom.nextValue());
        sql = sql.replace(":3", partTypeRandom.nextValue());

        return sql;
    }

    /**
     *
     --Q9: 产品类型利润估量查询
     --Q9语句是查询每个国家每一年所有被定购的零件在一年中的总利润。
     --Q9语句的特点是：带有分组、排序、聚集、子查询操作并存的查询操作。子查询的父层查询不存在其他查询对象，是格式相对简单的子查询，但子查询自身是多表连接的查询。子查询中使用了LIKE操作符，有的查询优化器不支持对LIKE操作符进行优化。

     * @return
     */
    String genQ9ProductTypeProfitMeasureQuery(){
        String sql = "" +
            "select                                                                               \n" +
            "    nation,                                                                          \n" +
            "    o_year,                                                                          \n" +
            "    sum(amount) as sum_profit                                                        \n" +
            "from                                                                                 \n" +
            "    (                                                                                \n" +
            "        select                                                                       \n" +
            "            n_name as nation,                                                        \n" +
            "            extract(year from o_orderdate) as o_year,                                \n" +
            "            l_extendedprice * (1 - l_discount) - ps_supplycost * l_quantity as amount\n" +
            "        from                                                                         \n" +
            "            part,                                                                    \n" +
            "            supplier,                                                                \n" +
            "            lineitem,                                                                \n" +
            "            partsupp,                                                                \n" +
            "            orders,                                                                  \n" +
            "            nation                                                                   \n" +
            "        where                                                                        \n" +
            "            s_suppkey = l_suppkey                                                    \n" +
            "            and ps_suppkey = l_suppkey                                               \n" +
            "            and ps_partkey = l_partkey                                               \n" +
            "            and p_partkey = l_partkey                                                \n" +
            "            and o_orderkey = l_orderkey                                              \n" +
            "            and s_nationkey = n_nationkey                                            \n" +
            "            and p_name like '%:1%'                                                   \n" +
            "    ) as profit                                                                      \n" +
            "group by                                                                             \n" +
            "    nation,                                                                          \n" +
            "    o_year                                                                           \n" +
            "order by                                                                             \n" +
            "    nation,                                                                          \n" +
            "    o_year desc                                                                      ";

        sql = sql.replace(":1", partColorRandom.nextValue());

        return sql;
    }

    /**
     *
     --Q10: 货运存在问题的查询
     --Q10语句是查询每个国家在某时刻起的三个月内货运存在问题的客户和造成的损失。
     --Q10语句的特点是：带有分组、排序、聚集操作并存的多表连接查询操作。查询语句没有从语法上限制返回多少条元组，但是TPC-H标准规定，查询结果只返回前10行（通常依赖于应用程序实现）。

     * @return
     */
    String genQ10(){
        String sql = "" +
            "select                                                 \n" +
            "    c_custkey,                                         \n" +
            "    c_name,                                            \n" +
            "    sum(l_extendedprice * (1 - l_discount)) as revenue,\n" +
            "    c_acctbal,                                         \n" +
            "    n_name,                                            \n" +
            "    c_address,                                         \n" +
            "    c_phone,                                           \n" +
            "    c_comment                                          \n" +
            "from                                                   \n" +
            "    customer,                                          \n" +
            "    orders,                                            \n" +
            "    lineitem,                                          \n" +
            "    nation                                             \n" +
            "where                                                  \n" +
            "    c_custkey = o_custkey                              \n" +
            "    and l_orderkey = o_orderkey                        \n" +
            "    and o_orderdate >= date ':1'                       \n" + // DATE是位于1993年一月到1994年十二月中任一月的一号
            "    and o_orderdate < date ':1' + interval '3' month   \n" +
            "    and l_returnflag = 'R'                             \n" + // 货物被回退
            "    and c_nationkey = n_nationkey                      \n" +
            "group by                                               \n" +
            "    c_custkey,                                         \n" +
            "    c_name,                                            \n" +
            "    c_acctbal,                                         \n" +
            "    c_phone,                                           \n" +
            "    n_name,                                            \n" +
            "    c_address,                                         \n" +
            "    c_comment                                          \n" +
            "order by                                               \n" +
            "    revenue desc                                       \n" +
            "limit 20                                               ";

        sql = sql.replace(":1", String.format("%d-%02d-01", randomYear1993_1994.nextValue(), randomMonth1_12.nextValue()));

        return sql;
    }

    /**
     *
     --Q11: 库存价值查询
     --Q11语句是查询库存中某个国家供应的零件的价值。
     --Q11语句的特点是：带有分组、排序、聚集、子查询操作并存的多表连接查询操作。子查询位于分组操作的HAVING条件中。

     * @return
     */
    String genQ11(){
        String sql = "" +
            "select                                               \n" +
            "    ps_partkey,                                      \n" +
            "    sum(ps_supplycost * ps_availqty) as value        \n" +
            "from                                                 \n" +
            "    partsupp,                                        \n" +
            "    supplier,                                        \n" +
            "    nation                                           \n" +
            "where                                                \n" +
            "    ps_suppkey = s_suppkey                           \n" +
            "    and s_nationkey = n_nationkey                    \n" +
            "    and n_name = ':1'                                \n" + // 指定国家
            "group by                                             \n" +
            "    ps_partkey having                                \n" +
            "        sum(ps_supplycost * ps_availqty) > (         \n" +
            "            select                                   \n" +
            "                sum(ps_supplycost * ps_availqty) * :2\n" + // FRACTION为0.0001/SF1
            "            from                                     \n" +
            "                partsupp,                            \n" +
            "                supplier,                            \n" +
            "                nation                               \n" +
            "            where                                    \n" +
            "                ps_suppkey = s_suppkey               \n" +
            "                and s_nationkey = n_nationkey        \n" +
            "                and n_name = ':1'                    \n" +
            "        )                                            \n" +
            "order by                                             \n" +
            "    value desc                                       ";

        sql = sql.replace(":1", nationRandom.nextValue());
        sql = sql.replace(":2", String.valueOf(0.0001/this.scaleFactor));

        return sql;
    }

    /**
     *
     --Q12: 货运模式和订单优先级查询
     --Q12语句查询获得货运模式和订单优先级。可以帮助决策：选择便宜的货运模式是否会导致消费者更多的在合同日期之后收到货物，而对紧急优先命令产生负面影响。
     --
     --Q12语句的特点是：带有分组、排序、聚集操作并存的两表连接查询操作。

     * @return
     */
    String genQ12ShippingModesAndOrderPriorityQuery(){
        String sql = "" +
            "select                                               \n" +
            "    l_shipmode,                                      \n" +
            "    sum(case                                         \n" +
            "        when o_orderpriority = '1-URGENT'            \n" +
            "            or o_orderpriority = '2-HIGH'            \n" +
            "            then 1                                   \n" +
            "        else 0                                       \n" +
            "    end) as high_line_count,                         \n" +
            "    sum(case                                         \n" +
            "        when o_orderpriority <> '1-URGENT'           \n" +
            "            and o_orderpriority <> '2-HIGH'          \n" +
            "            then 1                                   \n" +
            "        else 0                                       \n" +
            "    end) as low_line_count                           \n" +
            "from                                                 \n" +
            "    orders,                                          \n" +
            "    lineitem                                         \n" +
            "where                                                \n" +
            "    o_orderkey = l_orderkey                          \n" +
            "    and l_shipmode in (':1', ':2')                   \n" + // 指定货运模式的类型，在TPC-H标准指定的范围内随机选择，SHIPMODE2必须有别于SHIPMODE1
            "    and l_commitdate < l_receiptdate                 \n" +
            "    and l_shipdate < l_commitdate                    \n" +
            "    and l_receiptdate >= date ':3'                   \n" + // 从1993年到1997年中任一年的一月一号
            "    and l_receiptdate < date ':3' + interval '1' year\n" +
            "group by                                             \n" +
            "    l_shipmode                                       \n" +
            "order by                                             \n" +
            "    l_shipmode                                       ";

        sql = sql.replace(":1", shipModeRandom.nextValue());
        sql = sql.replace(":2", shipModeRandom.nextValue());
        sql = sql.replace(":3", String.format("%d-01-01", randomYear1993_1997.nextValue()));

        return sql;
    }

    /**
     *
     --Q13: 消费者订单数量查询
     --Q13语句查询获得消费者的订单数量，包括过去和现在都没有订单记录的消费者。
     --
     --Q13语句的特点是：带有分组、排序、聚集、子查询、左外连接操作并存的查询操作。
     * @return
     */
    String genQ13CustomerDistributionQuery(){
        String sql = "" +
            "select                                          \n" +
            "    c_count,                                    \n" +
            "    count(*) as custdist                        \n" +
            "from                                            \n" +
            "    (                                           \n" +
            "        select                                  \n" +
            "            c_custkey,                          \n" +
            "            count(o_orderkey)                   \n" +
            "        from                                    \n" +
            "            customer left outer join orders on  \n" +
            "                c_custkey = o_custkey           \n" +
            "                and o_comment not like '%:1%:2%'\n" +
            "        group by                                \n" +
            "            c_custkey                           \n" +
            "    ) as c_orders (c_custkey, c_count)          \n" +
            "group by                                        \n" +
            "    c_count                                     \n" +
            "order by                                        \n" +
            "    custdist desc,                              \n" +
            "    c_count desc                                ";

        sql = sql.replace(":1", q13aRandom.nextValue());
        sql = sql.replace(":2", q13bRandom.nextValue());

        return sql;
    }

    /**
     * --Q14: 促销效果查询
     * --Q14语句查询获得某一个月的收入中有多大的百分比是来自促销零件。用以监视促销带来的市场反应。
     * --
     * --Q14语句的特点是：带有分组、排序、聚集、子查询、左外连接操作并存的查询操作。
     *
     * @return
     */
    String genQ14PromotionEffectQuery(){
        String sql = "" +
            "select                                                             \n" +
            "    100.00 * sum(case                                              \n" +
            "        when p_type like 'PROMO%'                                  \n" +
            "            then l_extendedprice * (1 - l_discount)                \n" +
            "        else 0                                                     \n" +
            "    end) / sum(l_extendedprice * (1 - l_discount)) as promo_revenue\n" +
            "from                                                               \n" +
            "    lineitem,                                                      \n" +
            "    part                                                           \n" +
            "where                                                              \n" +
            "    l_partkey = p_partkey                                          \n" +
            "    and l_shipdate >= date ':1'                                    \n" + // 从1993年到1997年中任一年的任一月的一号
            "    and l_shipdate < date ':1' + interval '1' month                ";

        sql = sql.replace(":1", String.format("%d-01-01", randomYear1993_1997.nextValue()));

        return sql;
    }

    /**
     * --Q15: 头等供货商查询
     * --Q15语句查询获得某段时间内为总收入贡献最多的供货商（排名第一）的信息。可用以决定对哪些头等供货商给予奖励、给予更多订单、给予特别认证、给予鼓舞等激励。
     * --
     * --Q15语句的特点是：带有分排序、聚集、聚集子查询操作并存的普通表与视图的连接操作。
     *
     * @return
     */
    String genQ15TopSupplierQuery(){
        String sql = "" +
            "WITH revenue (supplier_no, total_revenue) as (         \n" +
            "    select                                             \n" +
            "        l_suppkey,                                     \n" +
            "        sum(l_extendedprice * (1 - l_discount))        \n" +
            "    from                                               \n" +
            "        lineitem                                       \n" +
            "    where                                              \n" +
            "        l_shipdate >= date ':1'                        \n" + // 从1993年一月到1997年十月中任一月的一号
            "        and l_shipdate < date ':1' + interval '3' month\n" +
            "    group by                                           \n" +
            "        l_suppkey                                      \n" +
            ")                                                      \n" +
            "select                                                 \n" +
            "    s_suppkey,                                         \n" +
            "    s_name,                                            \n" +
            "    s_address,                                         \n" +
            "    s_phone,                                           \n" +
            "    total_revenue                                      \n" +
            "from                                                   \n" +
            "    supplier,                                          \n" +
            "    revenue:s                                          \n" +
            "where                                                  \n" +
            "    s_suppkey = supplier_no                            \n" +
            "    and total_revenue = (                              \n" +
            "        select                                         \n" +
            "            max(total_revenue)                         \n" +
            "        from                                           \n" +
            "            revenue:s                                  \n" +
            "    )                                                  \n" +
            "order by                                               \n" +
            "    s_suppkey                                          ";

        int year = randomYear1993_1997.nextValue();
        if(year==1997){
            sql = sql.replace(":1", String.format("%d-%02d-01", year, randomMonth1_10.nextValue()));
        }else{
            sql = sql.replace(":1", String.format("%d-%02d-01", year, randomMonth1_12.nextValue()));
        }

        return sql;
    }

    /**
     *
     --Q16: 零件/供货商关系查询
     --Q16语句查询获得能够以指定的贡献条件供应零件的供货商数量。可用于决定在订单量大，任务紧急时，是否有充足的供货商。
     --Q16语句的特点是：带有分组、排序、聚集、去重、NOT IN子查询操作并存的两表连接操作。
     * @return
     */
    String genQ16PartsSupplierRelationshipQuery(){
        String sql = "" +
            "select                                             \n" +
            "    p_brand,                                       \n" +
            "    p_type,                                        \n" +
            "    p_size,                                        \n" +
            "    count(distinct ps_suppkey) as supplier_cnt     \n" +
            "from                                               \n" +
            "    partsupp,                                      \n" +
            "    part                                           \n" +
            "where                                              \n" +
            "    p_partkey = ps_partkey                         \n" +
            "    and p_brand <> ':1'                            \n" + // BRAND＝Brand＃MN ，M和N是两个字母，代表两个数值，相互独立，取值在1到5之间
            "    and p_type not like ':2%'                      \n" +
            "    and p_size in (:3, :4, :5, :6, :7, :8, :9, :10)\n" + // 在1到50之间任意选择的一组八个不同的值
            "    and ps_suppkey not in (                        \n" +
            "        select                                     \n" +
            "            s_suppkey                              \n" +
            "        from                                       \n" +
            "            supplier                               \n" +
            "        where                                      \n" +
            "            s_comment like '%Customer%Complaints%' \n" +
            "    )                                              \n" +
            "group by                                           \n" +
            "    p_brand,                                       \n" +
            "    p_type,                                        \n" +
            "    p_size                                         \n" +
            "order by                                           \n" +
            "    supplier_cnt desc,                             \n" +
            "    p_brand,                                       \n" +
            "    p_type,                                        \n" +
            "    p_size                                         ";

        sql = sql.replace(":1", String.format("Brand＃%d%d", randomBrand1_5.nextValue(), randomBrand1_5.nextValue()));
        sql = sql.replace(":2", partTypeRandom.nextValue()); // TODO 这里可能生成得不对.
        for (int i = 3; i < 11; i++) {
            sql = sql.replace(":"+i, String.valueOf(randomSize1_50.nextValue()));
        }

        return sql;
    }

    /**
     * --Q17: 小订单收入查询
     * --Q17语句查询获得比平均供货量的百分之二十还低的小批量订单。对于指定品牌和指定包装类型的零件，决定在一个七年数据库的所有订单中这些订单零件的平均项目数量（过去的和未决的）。如果这些零件中少于平均数20％的订单不再被接纳，那平均一年会损失多少呢？所以此查询可用于计算出如果没有没有小量订单，平均年收入将损失多少（因为大量商品的货运，将降低管理费用）。
     * --Q17语句的特点是：带有聚集、聚集子查询操作并存的两表连接操作。
     * @return
     */
    String genQ17SmallQuantityOrderRevenueQuery(){
        String sql = "" +
            "select                                      \n" +
            "    sum(l_extendedprice) / 7.0 as avg_yearly\n" +
            "from                                        \n" +
            "    lineitem,                               \n" +
            "    part                                    \n" +
            "where                                       \n" +
            "    p_partkey = l_partkey                   \n" +
            "    and p_brand = ':1'                      \n" + // 指定品牌。 BRAND＝’Brand#MN’ ，M和N是两个字母，代表两个数值，相互独立，取值在1到5之间
            "    and p_container = ':2'                  \n" + // 指定包装类型。在TPC-H标准指定的范围内随机选择
            "    and l_quantity < (                      \n" +
            "        select                              \n" +
            "            0.2 * avg(l_quantity)           \n" +
            "        from                                \n" +
            "            lineitem                        \n" +
            "        where                               \n" +
            "            l_partkey = p_partkey           \n" +
            "    )                                       ";

        sql = sql.replace(":1", String.format("Brand＃%d%d", randomBrand1_5.nextValue(), randomBrand1_5.nextValue()));
        sql = sql.replace(":2", partContainerRandom.nextValue());

        return sql;
    }

    /**
     * --Q18: 大订单顾客查询
     * --Q18语句查询获得比指定供货量大的供货商信息。可用于决定在订单量大，任务紧急时，验证否有充足的供货商。
     * --Q18语句的特点是：带有分组、排序、聚集、IN子查询操作并存的三表连接操作。查询语句没有从语法上限制返回多少条元组，但是TPC-H标准规定，查询结果只返回前100行（通常依赖于应用程序实现）。
     * @return
     */
    String genQ18LargeVolumeCustomerQuery(){
        String sql = "" +
            "select                              \n" +
            "    c_name,                         \n" +
            "    c_custkey,                      \n" +
            "    o_orderkey,                     \n" +
            "    o_orderdate,                    \n" +
            "    o_totalprice,                   \n" +
            "    sum(l_quantity)                 \n" +
            "from                                \n" +
            "    customer,                       \n" +
            "    orders,                         \n" +
            "    lineitem                        \n" +
            "where                               \n" +
            "    o_orderkey in (                 \n" +
            "        select                      \n" +
            "            l_orderkey              \n" +
            "        from                        \n" +
            "            lineitem                \n" +
            "        group by                    \n" +
            "            l_orderkey having       \n" +
            "                sum(l_quantity) > :1\n" + // 位于312到315之间的任意值
            "    )                               \n" +
            "    and c_custkey = o_custkey       \n" +
            "    and o_orderkey = l_orderkey     \n" +
            "group by                            \n" +
            "    c_name,                         \n" +
            "    c_custkey,                      \n" +
            "    o_orderkey,                     \n" +
            "    o_orderdate,                    \n" +
            "    o_totalprice                    \n" +
            "order by                            \n" +
            "    o_totalprice desc,              \n" +
            "    o_orderdate                     ";

        sql = sql.replace(":1", String.valueOf(randomQuantity312_315.nextValue()));

        return sql;
    }

    /**
     * --Q19: 折扣收入查询
     * --Q19语句查询得到对一些空运或人工运输零件三个不同种类的所有订单的总折扣收入。零件的选择考虑特定品牌、包装和尺寸范围。本查询是用数据挖掘工具产生格式化代码的一个例子。
     * --Q19语句的特点是：带有分组、排序、聚集、IN子查询操作并存的三表连接操作。
     *
     * @return
     */
    String genQ19DiscountedRevenueQuery(){
        String sql = "" +
            "select                                                                  \n" +
            "    sum(l_extendedprice* (1 - l_discount)) as revenue                   \n" +
            "from                                                                    \n" +
            "    lineitem,                                                           \n" +
            "    part                                                                \n" +
            "where                                                                   \n" +
            "    (                                                                   \n" +
            "        p_partkey = l_partkey                                           \n" +
            "        and p_brand = ':1'                                              \n" +
            "        and p_container in ('SM CASE', 'SM BOX', 'SM PACK', 'SM PKG')   \n" +
            "        and l_quantity >= :4 and l_quantity <= :4 + 10                  \n" +
            "        and p_size between 1 and 5                                      \n" +
            "        and l_shipmode in ('AIR', 'AIR REG')                            \n" +
            "        and l_shipinstruct = 'DELIVER IN PERSON'                        \n" +
            "    )                                                                   \n" +
            "    or                                                                  \n" +
            "    (                                                                   \n" +
            "        p_partkey = l_partkey                                           \n" +
            "        and p_brand = ':2'                                              \n" +
            "        and p_container in ('MED BAG', 'MED BOX', 'MED PKG', 'MED PACK')\n" +
            "        and l_quantity >= :5 and l_quantity <= :5 + 10                  \n" +
            "        and p_size between 1 and 10                                     \n" +
            "        and l_shipmode in ('AIR', 'AIR REG')                            \n" +
            "        and l_shipinstruct = 'DELIVER IN PERSON'                        \n" +
            "    )                                                                   \n" +
            "    or                                                                  \n" +
            "    (                                                                   \n" +
            "        p_partkey = l_partkey                                           \n" +
            "        and p_brand = ':3'                                              \n" +
            "        and p_container in ('LG CASE', 'LG BOX', 'LG PACK', 'LG PKG')   \n" +
            "        and l_quantity >= :6 and l_quantity <= :6 + 10                  \n" +
            "        and p_size between 1 and 15                                     \n" +
            "        and l_shipmode in ('AIR', 'AIR REG')                            \n" +
            "        and l_shipinstruct = 'DELIVER IN PERSON'                        \n" +
            "    )                                                                   ";

        sql = sql.replace(":1", String.format("Brand＃%d%d", randomBrand1_5.nextValue(), randomBrand1_5.nextValue()));
        sql = sql.replace(":2", String.format("Brand＃%d%d", randomBrand1_5.nextValue(), randomBrand1_5.nextValue()));
        sql = sql.replace(":3", String.format("Brand＃%d%d", randomBrand1_5.nextValue(), randomBrand1_5.nextValue()));

        sql = sql.replace(":4", String.valueOf(randomQuantity1_10.nextValue()));
        sql = sql.replace(":5", String.valueOf(randomQuantity10_20.nextValue()));
        sql = sql.replace(":6", String.valueOf(randomQuantity20_30.nextValue()));

        return sql;
    }

    /**
     * --Q20: 供货商竞争力查询
     * --Q20语句查询确定在某一年内，找出指定国家的能对某一零件商品提供更有竞争力价格的供货货。所谓更有竞争力的供货商，是指那些零件有过剩的供货商，超过供或商在某一年中货运给定国的某一零件的50％则为过剩。
     * --Q20语句的特点是：带有排序、聚集、IN子查询、普通子查询操作并存的两表连接操作。
     *
     * @return
     */
    String genQ20PotentialPartPromotionQuery(){
        String sql = "" +
            "select                                                            \n" +
            "    s_name,                                                       \n" +
            "    s_address                                                     \n" +
            "from                                                              \n" +
            "    supplier,                                                     \n" +
            "    nation                                                        \n" +
            "where                                                             \n" +
            "    s_suppkey in (                                                \n" +
            "        select                                                    \n" +
            "            ps_suppkey                                            \n" +
            "        from                                                      \n" +
            "            partsupp                                              \n" +
            "        where                                                     \n" +
            "            ps_partkey in (                                       \n" +
            "                select                                            \n" +
            "                    p_partkey                                     \n" +
            "                from                                              \n" +
            "                    part                                          \n" +
            "                where                                             \n" +
            "                    p_name like ':1%'                             \n" + // COLOR为产生P_NAME的值的列表中的任意值
            "            )                                                     \n" +
            "            and ps_availqty > (                                   \n" +
            "                select                                            \n" +
            "                    0.5 * sum(l_quantity)                         \n" +
            "                from                                              \n" +
            "                    lineitem                                      \n" +
            "                where                                             \n" +
            "                    l_partkey = ps_partkey                        \n" +
            "                    and l_suppkey = ps_suppkey                    \n" +
            "                    and l_shipdate >= date ':2'                   \n" + // 在1993年至1997年的任一年的一月一号
            "                    and l_shipdate < date ':2' + interval '1' year\n" +
            "            )                                                     \n" +
            "    )                                                             \n" +
            "    and s_nationkey = n_nationkey                                 \n" +
            "    and n_name = ':3'                                             \n" + // TPC-H标准定义的任意值
            "order by                                                          \n" +
            "    s_name                                                        ";

        sql = sql.replace(":1", String.valueOf(partColorRandom.nextValue()));
        sql = sql.replace(":2", String.format("%d-01-01", randomYear1993_1997.nextValue()));
        sql = sql.replace(":3", nation2Random.nextValue());

        return sql;
    }

    /**
     *
     --Q21: 不能按时交货供货商查询
     --Q21语句查询获得不能及时交货的供货商。
     --Q21语句的特点是：带有分组、排序、聚集、EXISTS子查询、NOT EXISTS子查询操作并存的四表连接操作。查询语句没有从语法上限制返回多少条元组，但是TPC-H标准规定，查询结果只返回前100行（通常依赖于应用程序实现）。
     * @return
     */
    String genQ21SuppliersWhoKeptOrdersWaitingQuery(){
        String sql = "" +
            "select                                            \n" +
            "    s_name,                                       \n" +
            "    count(*) as numwait                           \n" +
            "from                                              \n" +
            "    supplier,                                     \n" +
            "    lineitem l1,                                  \n" +
            "    orders,                                       \n" +
            "    nation                                        \n" +
            "where                                             \n" +
            "    s_suppkey = l1.l_suppkey                      \n" +
            "    and o_orderkey = l1.l_orderkey                \n" +
            "    and o_orderstatus = 'F'                       \n" +
            "    and l1.l_receiptdate > l1.l_commitdate        \n" +
            "    and exists (                                  \n" +
            "        select                                    \n" +
            "            *                                     \n" +
            "        from                                      \n" +
            "            lineitem l2                           \n" +
            "        where                                     \n" +
            "            l2.l_orderkey = l1.l_orderkey         \n" +
            "            and l2.l_suppkey <> l1.l_suppkey      \n" +
            "    )                                             \n" +
            "    and not exists (                              \n" +
            "        select                                    \n" +
            "            *                                     \n" +
            "        from                                      \n" +
            "            lineitem l3                           \n" +
            "        where                                     \n" +
            "            l3.l_orderkey = l1.l_orderkey         \n" +
            "            and l3.l_suppkey <> l1.l_suppkey      \n" +
            "            and l3.l_receiptdate > l3.l_commitdate\n" +
            "    )                                             \n" +
            "    and s_nationkey = n_nationkey                 \n" +
            "    and n_name = ':1'                             \n" +
            "group by                                          \n" +
            "    s_name                                        \n" +
            "order by                                          \n" +
            "    numwait desc,                                 \n" +
            "    s_name                                        \n" +
            "limit 100                                         ";

        sql = sql.replace(":1", nation2Random.nextValue());

        return sql;
    }

    /**
     * --Q22: 全球销售机会查询
     * --Q22语句查询获得消费者可能购买的地理分布。本查询计算在指定的国家，比平均水平更持肯定态度但还没下七年订单的消费者数量。能反应出普通消费者的的态度，即购买意向。
     * --Q22语句的特点是：带有分组、排序、聚集、EXISTS子查询、NOT EXISTS子查询操作并存的四表连接操作。
     *
     * @return
     */
    String genQ22(){
        String sql = "" +
            "select                                                            \n" +
            "    cntrycode,                                                    \n" +
            "    count(*) as numcust,                                          \n" +
            "    sum(c_acctbal) as totacctbal                                  \n" +
            "from                                                              \n" +
            "    (                                                             \n" +
            "        select                                                    \n" +
            "            substring(c_phone from 1 for 2) as cntrycode,         \n" +
            "            c_acctbal                                             \n" +
            "        from                                                      \n" +
            "            customer                                              \n" +
            "        where                                                     \n" +
            "            substring(c_phone from 1 for 2) in                    \n" +
            "                (':1', ':2', ':3', ':4', ':5', ':6', ':7')        \n" + // 在TPC-H中定义国家代码的可能值中不重复的任意值
            "            and c_acctbal > (                                     \n" +
            "                select                                            \n" +
            "                    avg(c_acctbal)                                \n" +
            "                from                                              \n" +
            "                    customer                                      \n" +
            "                where                                             \n" +
            "                    c_acctbal > 0.00                              \n" +
            "                    and substring(c_phone from 1 for 2) in        \n" +
            "                        (':1', ':2', ':3', ':4', ':5', ':6', ':7')\n" +
            "            )                                                     \n" +
            "            and not exists (                                      \n" +
            "                select                                            \n" +
            "                    *                                             \n" +
            "                from                                              \n" +
            "                    orders                                        \n" +
            "                where                                             \n" +
            "                    o_custkey = c_custkey                         \n" +
            "            )                                                     \n" +
            "    ) as custsale                                                 \n" +
            "group by                                                          \n" +
            "    cntrycode                                                     \n" +
            "order by                                                          \n" +
            "    cntrycode                                                     ";

        for (int i = 1; i < 8; i++) {
            sql = sql.replace(":"+i, String.valueOf(randomNationCode10_34.nextValue()));
        }

        return sql;
    }

}
