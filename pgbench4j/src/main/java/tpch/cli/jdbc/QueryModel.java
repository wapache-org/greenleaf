package tpch.cli.jdbc;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public class QueryModel {

  private static final Logger LOGGER = LoggerFactory.getLogger(QueryModel.class);

  // common properties
  /**  */
  private String jdbcUrl;
  /**  */
  private String jdbcUser;
  /**  */
  private String jdbcPwd;
  /**  */
  private String jdbcDriver;

  /**  */
  private int jdbcPoolSize = 20;

  /** 执行次数 */
  private int execIteration = 1;
  /** 执行间隔, 单位: 秒 */
  private int execInterval = 1;
  /** 并发度 */
  private int execConcurrentSize = 1;
  /** 洗牌SQL statement */
  private boolean shuffleExecute = false;
  /** 统计结果保存路径 */
  private String reportStore;
  /** 每一个连接到数据库的连接都会执行这个初始化SQL语句 */
  private String connInitQuery;

  /** 单个数据加载的配置 */
  private List<DataLoadModel> dataLoads;
  /** 单个查询的配置 */
  private List<QuerySlice> querySlices;

  public QueryModel() {
  }

  @Override
  public String toString() {
    final StringBuffer sb = new StringBuffer("QueryModel{");
    sb.append("jdbcUrl='").append(jdbcUrl).append('\'');
    sb.append(", jdbcUser='").append(jdbcUser).append('\'');
    sb.append(", jdbcPwd='").append(jdbcPwd).append('\'');
    sb.append(", jdbcDriver='").append(jdbcDriver).append('\'');
    sb.append(", jdbcPoolSize=").append(jdbcPoolSize);
    sb.append(", execIteration=").append(execIteration);
    sb.append(", execInterval=").append(execInterval);
    sb.append(", execConcurrentSize=").append(execConcurrentSize);
    sb.append(", shuffleExecute=").append(shuffleExecute);
    sb.append(", reportStore='").append(reportStore).append('\'');
    sb.append(", connInitQuery='").append(connInitQuery).append('\'');
    sb.append(", dataLoads=").append(dataLoads);
    sb.append(", querySlices=").append(querySlices);
    sb.append('}');
    return sb.toString();
  }

  public String getJdbcUrl() {
    return jdbcUrl;
  }

  public void setJdbcUrl(String jdbcUrl) {
    this.jdbcUrl = jdbcUrl;
  }

  public String getJdbcUser() {
    return jdbcUser;
  }

  public void setJdbcUser(String jdbcUser) {
    this.jdbcUser = jdbcUser;
  }

  public String getJdbcPwd() {
    return jdbcPwd;
  }

  public void setJdbcPwd(String jdbcPwd) {
    this.jdbcPwd = jdbcPwd;
  }

  public String getJdbcDriver() {
    return jdbcDriver;
  }

  public void setJdbcDriver(String jdbcDriver) {
    this.jdbcDriver = jdbcDriver;
  }

  public int getJdbcPoolSize() {
    return jdbcPoolSize;
  }

  public void setJdbcPoolSize(int jdbcPoolSize) {
    this.jdbcPoolSize = jdbcPoolSize;
  }

  public int getExecIteration() {
    return execIteration;
  }

  public void setExecIteration(int execIteration) {
    this.execIteration = execIteration;
  }

  public int getExecInterval() {
    return execInterval;
  }

  public void setExecInterval(int execInterval) {
    this.execInterval = execInterval;
  }

  public int getExecConcurrentSize() {
    return execConcurrentSize;
  }

  public void setExecConcurrentSize(int execConcurrentSize) {
    this.execConcurrentSize = execConcurrentSize;
  }

  public boolean isShuffleExecute() {
    return shuffleExecute;
  }

  public void setShuffleExecute(boolean shuffleExecute) {
    this.shuffleExecute = shuffleExecute;
  }

  public String getReportStore() {
    return reportStore;
  }

  public void setReportStore(String reportStore) {
    this.reportStore = reportStore;
  }

  public String getConnInitQuery() {
    return connInitQuery;
  }

  public void setConnInitQuery(String connInitQuery) {
    this.connInitQuery = connInitQuery;
  }

  public List<QuerySlice> getQuerySlices() {
    return querySlices;
  }

  public void setQuerySlices(List<QuerySlice> querySlices) {
    this.querySlices = querySlices;
  }

  public List<DataLoadModel> getDataLoads() {
    return dataLoads;
  }

  public void setDataLoads(List<DataLoadModel> dataLoads) {
    this.dataLoads = dataLoads;
  }

  public static final class QueryModelBuilder {
    // common properties
    private String jdbcUrl;
    private String jdbcUser;
    private String jdbcPwd;
    private String jdbcDriver;
    private int jdbcPoolSize = 20;
    private int execIteration = 1;
    private int execInterval = 1;
    private int execConcurrentSize = 1;
    private boolean shuffleExecute = false;
    private String reportStore;
    private String connInitQuery;
    private List<QuerySlice> querySlices;

    private QueryModelBuilder() {
    }

    public static QueryModelBuilder aQueryModel() {
      return new QueryModelBuilder();
    }

    public QueryModelBuilder withJdbcUrl(String jdbcUrl) {
      this.jdbcUrl = jdbcUrl;
      return this;
    }

    public QueryModelBuilder withJdbcUser(String jdbcUser) {
      this.jdbcUser = jdbcUser;
      return this;
    }

    public QueryModelBuilder withJdbcPwd(String jdbcPwd) {
      this.jdbcPwd = jdbcPwd;
      return this;
    }

    public QueryModelBuilder withJdbcDriver(String jdbcDriver) {
      this.jdbcDriver = jdbcDriver;
      return this;
    }

    public QueryModelBuilder withJdbcPoolSize(int jdbcPoolSize) {
      this.jdbcPoolSize = jdbcPoolSize;
      return this;
    }

    public QueryModelBuilder withExecIteration(int execIteration) {
      this.execIteration = execIteration;
      return this;
    }

    public QueryModelBuilder withExecInterval(int execInterval) {
      this.execInterval = execInterval;
      return this;
    }

    public QueryModelBuilder withExecConcurrentSize(int execConcurrentSize) {
      this.execConcurrentSize = execConcurrentSize;
      return this;
    }

    public QueryModelBuilder withShuffleExecute(boolean shuffleExecute) {
      this.shuffleExecute = shuffleExecute;
      return this;
    }

    public QueryModelBuilder withReportStore(String reportStore) {
      this.reportStore = reportStore;
      return this;
    }

    public QueryModelBuilder withConnInitQuery(String connInitQuery) {
      this.connInitQuery = connInitQuery;
      return this;
    }

    public QueryModelBuilder withQuerySlices(List<QuerySlice> querySlices) {
      this.querySlices = querySlices;
      return this;
    }

    public QueryModel build() {
      QueryModel queryModel = new QueryModel();
      queryModel.setJdbcUrl(jdbcUrl);
      queryModel.setJdbcUser(jdbcUser);
      queryModel.setJdbcPwd(jdbcPwd);
      queryModel.setJdbcDriver(jdbcDriver);
      queryModel.setJdbcPoolSize(jdbcPoolSize);
      queryModel.setExecIteration(execIteration);
      queryModel.setExecInterval(execInterval);
      queryModel.setExecConcurrentSize(execConcurrentSize);
      queryModel.setShuffleExecute(shuffleExecute);
      queryModel.setReportStore(reportStore);
      queryModel.setConnInitQuery(connInitQuery);
      queryModel.setQuerySlices(querySlices);
      return queryModel;
    }
  }
}
