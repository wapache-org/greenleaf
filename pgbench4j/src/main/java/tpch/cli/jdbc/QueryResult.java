package tpch.cli.jdbc;

public final class QueryResult {
  private QuerySlice querySlice;
  private long duration = -1;
  private int resultSize;

  public QueryResult() {
  }

  @Override
  public String toString() {
    final StringBuffer sb = new StringBuffer("QueryResult{");
    sb.append("querySlice=").append(querySlice);
    sb.append(", duration=").append(duration);
    sb.append(", resultSize=").append(resultSize);
    sb.append('}');
    return sb.toString();
  }

  public QuerySlice getQuerySlice() {
    return querySlice;
  }

  public void setQuerySlice(QuerySlice querySlice) {
    this.querySlice = querySlice;
  }

  public long getDuration() {
    return duration;
  }

  public void setDuration(long duration) {
    this.duration = duration;
  }

  public int getResultSize() {
    return resultSize;
  }

  public void setResultSize(int resultSize) {
    this.resultSize = resultSize;
  }

  public static final class QueryResultBuilder {
    private QuerySlice querySlice;
    private long duration;
    private int resultSize;

    private QueryResultBuilder() {
    }

    public static QueryResultBuilder aQueryResult() {
      return new QueryResultBuilder();
    }

    public QueryResultBuilder withQuerySlice(QuerySlice querySlice) {
      this.querySlice = querySlice;
      return this;
    }

    public QueryResultBuilder withDuration(long duration) {
      this.duration = duration;
      return this;
    }

    public QueryResultBuilder withResultSize(int resultSize) {
      this.resultSize = resultSize;
      return this;
    }

    public QueryResult build() {
      QueryResult queryResult = new QueryResult();
      queryResult.setQuerySlice(querySlice);
      queryResult.setDuration(duration);
      queryResult.setResultSize(resultSize);
      return queryResult;
    }
  }
}
