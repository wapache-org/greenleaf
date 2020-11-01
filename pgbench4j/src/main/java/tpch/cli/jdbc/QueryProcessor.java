package tpch.cli.jdbc;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import tpch.cli.common.Utils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import tpch.prestosql.QueryGenerator;

public class QueryProcessor {

  private static final Logger LOGGER = LoggerFactory.getLogger(QueryProcessor.class);

  private static final ConnectionMgr connectionMgr = ConnectionMgr.getInstance();
  private ExecutorService executorService;

  // package private
  QueryProcessor(int size) {
    this.executorService = Executors.newFixedThreadPool(size);
  }

  public String refreshSql(QuerySlice querySlice) {

    if(querySlice.getQuery()!=null && (!querySlice.isStaticQuery() || querySlice.getSql()==null || querySlice.getSql().isEmpty())){
      QueryGenerator generator = new QueryGenerator(querySlice.getScaleFactor());
      querySlice.setSql(generator.generateQuery(querySlice.getQuery()));
      LOGGER.debug("刷新SQL: {}={}", querySlice.getQuery(), querySlice.getSql());
    }

    return querySlice.getSql();
  }

  private QueryResult internalQuery(Connection conn, QuerySlice querySlice) throws SQLException {
    ResultSet rs = null;
    PreparedStatement statement = null;
    try {

      int cnt = 0;
      long startWatch = System.currentTimeMillis();

      if(querySlice.isCopyQuery()){
        tpcc.jdbc.ExecJDBC.execCopy( conn, querySlice.getSql());
      }else{
        statement = conn.prepareStatement(refreshSql(querySlice));

        if (querySlice.isSelectQuery()) {
          rs = statement.executeQuery();

          if (querySlice.isConsumeResult()) {
            while (rs.next()) {
              cnt++;
            }
          }
        } else {
          statement.execute();
          cnt = -1;
        }
      }

      long stopWatch = System.currentTimeMillis();

      QueryResult queryResult = QueryResult.QueryResultBuilder.aQueryResult()
          .withQuerySlice(querySlice)
          .withDuration(stopWatch - startWatch)
          .withResultSize(cnt)
          .build();
      if (LOGGER.isDebugEnabled()) {
        LOGGER.debug("Query result " + queryResult);
      }
      return queryResult;
    } catch (SQLException e) {
      throw e;
    } finally {
      Utils.closeQuietly(statement);
      Utils.closeQuietly(rs);
    }
  }

  public List<QueryResult> processBatch(List<QuerySlice> querySlices) throws InterruptedException {
    List<Future<QueryResult>> executorTaskList = new ArrayList<>();

    for (QuerySlice querySlice : querySlices) {
      QueryWorker queryWorker = new QueryWorker(querySlice);
      executorTaskList.add(executorService.submit(queryWorker));
    }

    List<QueryResult> queryResults = new ArrayList<>();
    for (int i = 0; i < executorTaskList.size(); i++) {
      try {
        queryResults.add(executorTaskList.get(i).get());
      } catch (InterruptedException | ExecutionException e) {
        LOGGER.error("Failed to execute task", e);
      }
    }

    return queryResults;
  }

  private final class QueryWorker implements Callable<QueryResult> {
    private QuerySlice querySlice;

    private QueryWorker(QuerySlice querySlice) {
      this.querySlice = querySlice;
    }

    @Override public QueryResult call() throws Exception {
      Connection conn = connectionMgr.borrowConnection();
      QueryResult result = null;

      try {
        result = internalQuery(conn, querySlice);
      } catch (SQLException e) {
        LOGGER.error("Failed to execute query " + querySlice, e);
        result = QueryResult.QueryResultBuilder.aQueryResult()
            .withQuerySlice(querySlice)
            .withDuration(-1)
            .withResultSize(-1)
            .build();
      } finally {
        connectionMgr.returnConnection(conn);
      }

      return result;
    }
  }

  public void close() {
    if (null != executorService) {
      executorService.shutdown();
    }
    connectionMgr.close();
  }
}
