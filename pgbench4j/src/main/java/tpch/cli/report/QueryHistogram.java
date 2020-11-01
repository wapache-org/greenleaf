package tpch.cli.report;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

import org.apache.commons.collections4.CollectionUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public final class QueryHistogram<T> {

  private static final Logger LOGGER = LoggerFactory.getLogger(QueryHistogram.class);

  private static final List<String> HISTOGRAM_TITLE = new ArrayList<String>() {{
    add("query");
    add("avg");
    add("size");
    add("total");
    add("min");
    add("max");
    add("25%");
    add("50%");
    add("75%");
    add("90%");
    add("95%");
  }};

  private String query;
  private double avg;
  private int size;
  private T total;
  private T min;
  private T max;
  private T quarter;
  private T half;
  private T three_quarters;
  private T ninety;
  private T ninety_five;

  @Override public String toString() {
    final StringBuffer sb = new StringBuffer("QueryHistogram{");
    sb.append("query='").append(query).append("'");
    sb.append(", avg=").append(avg);
    sb.append(", size=").append(size);
    sb.append(", total=").append(total);
    sb.append(", min=").append(min);
    sb.append(", max=").append(max);
    sb.append(", 25%=").append(quarter);
    sb.append(", 50%=").append(half);
    sb.append(", 75%=").append(three_quarters);
    sb.append(", 90%=").append(ninety);
    sb.append(", 95%=").append(ninety_five);
    sb.append('}');
    return sb.toString();
  }

  public static List<String> getTitle() {
    return HISTOGRAM_TITLE;
  }

  public String getQuery() {
    return query;
  }

  public List<String> getRawValue() {
    List<String> value = new ArrayList<>(10);
    value.add(String.valueOf(this.query));
    value.add(String.valueOf(this.avg));
    value.add(String.valueOf(this.size));
    value.add(String.valueOf(this.total));
    value.add(String.valueOf(this.min));
    value.add(String.valueOf(this.max));
    value.add(String.valueOf(this.quarter));
    value.add(String.valueOf(this.half));
    value.add(String.valueOf(this.three_quarters));
    value.add(String.valueOf(this.ninety));
    value.add(String.valueOf(this.ninety_five));

    return value;
  }

  public static final class QueryHistogramBuilder<T> {
    private String query;
    private double avg;
    private int size;
    private T total;
    private T min;
    private T max;
    private T quarter;
    private T half;
    private T three_quarters;
    private T ninety;
    private T ninety_five;

    private QueryHistogramBuilder() {
    }

    public static QueryHistogramBuilder createBuilder() {
      return new QueryHistogramBuilder();
    }

    public QueryHistogramBuilder withQuery(String query) {
      this.query = query;
      return this;
    }

    public QueryHistogramBuilder withSize(int size) {
      this.size = size;
      return this;
    }

    public QueryHistogramBuilder withTotal(T total) {
      this.total = total;
      return this;
    }

    public QueryHistogramBuilder withMin(T min) {
      this.min = min;
      return this;
    }

    public QueryHistogramBuilder withMax(T max) {
      this.max = max;
      return this;
    }

    public QueryHistogramBuilder withAvg(double avg) {
      this.avg = avg;
      return this;
    }

    public QueryHistogramBuilder withQuarter(T quarter) {
      this.quarter = quarter;
      return this;
    }

    public QueryHistogramBuilder withHalf(T half) {
      this.half = half;
      return this;
    }

    public QueryHistogramBuilder withThree_quarters(T three_quarters) {
      this.three_quarters = three_quarters;
      return this;
    }

    public QueryHistogramBuilder withNinety(T ninety) {
      this.ninety = ninety;
      return this;
    }

    public QueryHistogramBuilder withNinety_five(T ninety_five) {
      this.ninety_five = ninety_five;
      return this;
    }

    public QueryHistogram build() {
      QueryHistogram queryHistogram = new QueryHistogram();
      queryHistogram.ninety = this.ninety;
      queryHistogram.half = this.half;
      queryHistogram.ninety_five = this.ninety_five;
      queryHistogram.max = this.max;
      queryHistogram.total = this.total;
      queryHistogram.min = this.min;
      queryHistogram.quarter = this.quarter;
      queryHistogram.three_quarters = this.three_quarters;
      queryHistogram.avg = this.avg;
      queryHistogram.size = this.size;
      queryHistogram.query = this.query;
      return queryHistogram;
    }
  }

  public static QueryHistogram statisticList(String query, List<Long> list) {
    if (CollectionUtils.isEmpty(list)) {
      LOGGER.error("Encounter empty list for statistic, will return empty histogram");
      return new QueryHistogram();
    }
    list.sort(new Comparator<Long>() {
      @Override public int compare(Long o1, Long o2) {
        return o1.compareTo(o2);
      }
    });
    //size
    int size = list.size();
    //total
    long total = 0;
    for (Long l : list) {
      total += l;
    }
    //min
    long min = list.get(0);
    //max
    long max = list.get(size - 1);
    //avg
    double avg =
        new BigDecimal(total * 0.1 / size * 10).setScale(1, BigDecimal.ROUND_HALF_UP).doubleValue();
    //25%
    long quarter = list.get(size / 4);
    //50%
    long half = list.get(size / 2);
    //75%
    long three_quarters = list.get(size * 3 / 4);
    //90%
    long ninety = list.get(size * 9 / 10);
    //95%
    long ninety_five = list.get(size * 95 / 100);

    QueryHistogramBuilder queryHistogramBuilder =
        QueryHistogramBuilder.createBuilder();

    return queryHistogramBuilder
        .withQuery(query)
        .withSize(size)
        .withTotal(total)
        .withMin(min)
        .withMax(max)
        .withAvg(avg)
        .withQuarter(quarter)
        .withHalf(half)
        .withThree_quarters(three_quarters)
        .withNinety(ninety)
        .withNinety_five(ninety_five)
        .build();
  }
}
