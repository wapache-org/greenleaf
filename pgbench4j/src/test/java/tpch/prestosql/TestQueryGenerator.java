package tpch.prestosql;

import org.junit.Test;
import tpch.prestosql.QueryGenerator;

public class TestQueryGenerator {

    @Test
    public void testgenerateQuerys()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String[] sqls = generator.generateQuerys();
        for (String sql : sqls){
            System.out.println(sql);
        }
    }

    @Test
    public void testQ1()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ1PricingSummaryReportQuery();
        System.out.println(sql);
    }

    @Test
    public void testQ2()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ2MinimumCostSupplierQuery();
        System.out.println(sql);
    }

    @Test
    public void testQ3()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ3ShippingPriorityQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ4()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ4OrderPriorityCheckingQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ5()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ5();
        System.out.println(sql);
    }
    @Test
    public void testQ6()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ6ForecastingRevenueChangeQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ7()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ7VolumeShippingQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ8()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ8NationalMarketShareQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ9()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ9ProductTypeProfitMeasureQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ10()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ10();
        System.out.println(sql);
    }
    @Test
    public void testQ11()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ11();
        System.out.println(sql);
    }
    @Test
    public void testQ12()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ12ShippingModesAndOrderPriorityQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ13()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ13CustomerDistributionQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ14()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ14PromotionEffectQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ15()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ15TopSupplierQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ16()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ16PartsSupplierRelationshipQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ17()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ17SmallQuantityOrderRevenueQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ18()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ18LargeVolumeCustomerQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ19()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ19DiscountedRevenueQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ20()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ20PotentialPartPromotionQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ21()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ21SuppliersWhoKeptOrdersWaitingQuery();
        System.out.println(sql);
    }
    @Test
    public void testQ22()
    {
        QueryGenerator generator = new QueryGenerator(1);
        String sql = generator.genQ22();
        System.out.println(sql);
    }
}
