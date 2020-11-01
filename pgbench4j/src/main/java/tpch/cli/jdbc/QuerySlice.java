package tpch.cli.jdbc;

/**
 * 一个查询
 */
public class QuerySlice {

  private String id = "q" + System.nanoTime();
  /** SQL语句 */
  private String sql;
  /** SQL分组 */
  private String type = "unclassified";
  /** 多少个线程 */
  private int threads = 1;
  /** SQL语句是否COPY语句 */
  private boolean isCopyQuery = false;
  /** SQL语句是否查询 */
  private boolean isSelectQuery = false;
  /** 是否拉取查询结果 */
  private boolean isConsumeResult = true;
  /** 是否统计这个语句 */
  private boolean isCountInStatistics = true;

  /** 规模因子 */
  private Integer scaleFactor = 1;
  /** 查询模板编号, 取值范围[1,22] , 当这个字段有值时, 表示根据scaleFactor生成SQL. */
  private Integer query;
  /** 是否静态查询语句, true: 语句只生成一次, false: 每次执行都根据scaleFactor生成一个新的SQL */
  private boolean isStaticQuery = true;

  public QuerySlice() {
  }

  @Override
  public String toString() {
    final StringBuffer sb = new StringBuffer("QuerySlice{");
    sb.append("id='").append(id).append('\'');
    sb.append(", sql='").append(sql).append('\'');
    sb.append(", type='").append(type).append('\'');
    sb.append(", threads=").append(threads);
    sb.append(", isSelectQuery=").append(isSelectQuery);
    sb.append(", isConsumeResult=").append(isConsumeResult);
    sb.append(", isCountInStatistics=").append(isCountInStatistics);
    sb.append(", scaleFactor=").append(scaleFactor);
    sb.append(", query=").append(query);
    sb.append(", isStaticQuery=").append(query);
    sb.append('}');
    return sb.toString();
  }

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getSql() {
    return sql;
  }

  public void setSql(String sql) {
    this.sql = sql;
  }

  public String getType() {
    return type;
  }

  public void setType(String type) {
    this.type = type;
  }

  public int getThreads() {
    return threads;
  }

  public void setThreads(int threads) {
    this.threads = threads;
  }

  public boolean isSelectQuery() {
    return isSelectQuery;
  }

  public void setSelectQuery(boolean selectQuery) {
    isSelectQuery = selectQuery;
  }

  public boolean isConsumeResult() {
    return isConsumeResult;
  }

  public void setConsumeResult(boolean consumeResult) {
    isConsumeResult = consumeResult;
  }

  public boolean isCountInStatistics() {
    return isCountInStatistics;
  }

  public void setCountInStatistics(boolean countInStatistics) {
    isCountInStatistics = countInStatistics;
  }

  public Integer getScaleFactor() {
    return scaleFactor;
  }

  public void setScaleFactor(Integer scaleFactor) {
    this.scaleFactor = scaleFactor;
  }

  public Integer getQuery() {
    return query;
  }

  public void setQuery(Integer query) {
    this.query = query;
  }

  public boolean isStaticQuery() {
    return isStaticQuery;
  }

  public void setStaticQuery(boolean staticQuery) {
    isStaticQuery = staticQuery;
  }

  public boolean isCopyQuery() {
    return isCopyQuery;
  }

  public void setCopyQuery(boolean copyQuery) {
    isCopyQuery = copyQuery;
  }

  public static final class QuerySliceBuilder {
    private String id = "q" + System.nanoTime();
    private String sql;
    private String type = "unclassfied";
    private int threads = 1;
    private boolean isSelectQuery = true;
    private boolean isConsumeResult = true;
    private boolean isCountInStatistics = true;

    private QuerySliceBuilder() {
    }

    public static QuerySliceBuilder aQuerySlice() {
      return new QuerySliceBuilder();
    }

    public QuerySliceBuilder withId(String id) {
      this.id = id;
      return this;
    }

    public QuerySliceBuilder withSql(String sql) {
      this.sql = sql;
      return this;
    }

    public QuerySliceBuilder withType(String type) {
      this.type = type;
      return this;
    }

    public QuerySliceBuilder withThreads(int threads) {
      this.threads = threads;
      return this;
    }

    public QuerySliceBuilder withIsSelectQuery(boolean isSelectQuery) {
      this.isSelectQuery = isSelectQuery;
      return this;
    }

    public QuerySliceBuilder withIsConsumeResult(boolean isConsumeResult) {
      this.isConsumeResult = isConsumeResult;
      return this;
    }

    public QuerySliceBuilder withIsCountInStatistics(boolean isCountInStatistics) {
      this.isCountInStatistics = isCountInStatistics;
      return this;
    }

    public QuerySlice build() {
      QuerySlice querySlice = new QuerySlice();
      querySlice.setId(id);
      querySlice.setSql(sql);
      querySlice.setType(type);
      querySlice.setThreads(threads);
      querySlice.isSelectQuery = this.isSelectQuery;
      querySlice.isConsumeResult = this.isConsumeResult;
      querySlice.isCountInStatistics = this.isCountInStatistics;
      return querySlice;
    }
  }
}
