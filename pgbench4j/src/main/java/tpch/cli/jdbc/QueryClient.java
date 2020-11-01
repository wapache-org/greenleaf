package tpch.cli.jdbc;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

import com.google.gson.Gson;
import tpch.cli.Procedure;
import tpch.cli.common.Utils;
import tpch.cli.report.HistogramReporter;
import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class QueryClient implements Procedure {

  private static final Logger LOGGER = LoggerFactory.getLogger(QueryClient.class);

  private String inputFile;
  private QueryModel queryModel;
  private QueryProcessor queryProcessor;

  public QueryClient() {
  }

  @Override
  public void setInputFiles(String... inputFiles) {
    this.inputFile = inputFiles[0];
  }

  private void loadQueryModel(File file) throws IOException {
    Gson gson = new Gson();
    Reader reader = null;
    try {
      reader = new FileReader(file);
      queryModel = gson.fromJson(reader, QueryModel.class);
      LOGGER.info("加载配置文件: " + file.getAbsolutePath());
    } catch (IOException e) {
      LOGGER.error("Failed to load model for sql execution from path " + file.getAbsolutePath(), e);
      throw e;
    } finally {
      if (null != reader) {
        reader.close();
      }
    }
  }

  private void validate() throws IllegalArgumentException {
    if (StringUtils.isBlank(queryModel.getJdbcUrl())) {
      throw new IllegalArgumentException("'jdbcUrl' is required in QueryModel");
    }
    if (StringUtils.isBlank(queryModel.getJdbcDriver())) {
      throw new IllegalArgumentException("'jdbcDriver' is required in QueryModel");
    }
    if (CollectionUtils.isEmpty(queryModel.getQuerySlices())) {
      throw new IllegalArgumentException("'querySlices' is required in QueryModel");
    }
  }

  @Override
  public void ignite() throws Exception {

    LOGGER.info("开始执行查询: {}" , inputFile);

    File file = FileUtils.getFile(inputFile);
    loadQueryModel(file);
    validate();

    ConnectionMgr.getInstance().init(
        queryModel.getJdbcDriver(), queryModel.getJdbcUrl(),
        queryModel.getJdbcUser(), queryModel.getJdbcPwd(),
        queryModel.getJdbcPoolSize(), queryModel.getConnInitQuery()
    );

    if(queryModel.getDataLoads()!=null){
      load(queryModel.getDataLoads());
    }

    int iteration = queryModel.getExecIteration();
    List<String> allReports = new ArrayList<>(iteration);
    List<QueryResult> allResults4Stat = new ArrayList<>();
    List<Long> end2EndDuration = new ArrayList<>(iteration);
    for (int i = 0; i < iteration; i++) {
      long start = System.currentTimeMillis();
      List<QueryResult> cycleResult = query(queryModel.getQuerySlices());
      end2EndDuration.add(System.currentTimeMillis() - start);

      cycleResult.forEach(r->{
        if(r.getQuerySlice().isConsumeResult()){
          LOGGER.info("查询语句 {}.{} 返回了 {} 条记录",
              r.getQuerySlice().getType(),
              r.getQuerySlice().getId(),
              r.getResultSize()
          );
        }
      });

      List<QueryResult> results4Stat = cycleResult.stream()
          .filter(r -> r.getQuerySlice().isCountInStatistics())
          .filter(r -> r.getDuration() >= 0)
          .collect(Collectors.toList());
      if (cycleResult.size() > results4Stat.size()) {
        LOGGER.info("Skip {} results for statistic in iteration {} due to execlusion or failure",
            cycleResult.size() - results4Stat.size(),
            i + 1
        );
      }

      // do statistics for each iteration
      if (results4Stat.size() != 0) {
        String eachReport = HistogramReporter.statistic(results4Stat, queryModel.getReportStore());
        LOGGER.debug("第{}轮查询统计(ms): {}", i + 1, eachReport);
        allResults4Stat.addAll(results4Stat);
        allReports.add(eachReport);
      } else {
        LOGGER.warn("Skip statistic in iteration {} due to empty results", i + 1);
      }

      if (i < iteration - 1 && queryModel.getExecInterval() > 0) {
        LOGGER.info("Waiting {}s for the next iteration of query",
            queryModel.getExecInterval()
        );
        Thread.sleep(1000 * queryModel.getExecInterval());
      }
    }

    // this is for final output, we will print statistics for each iteration
    for (int i = 0; i < allReports.size(); i++) {
      LOGGER.info("查询统计汇总(ms), 第{}轮的统计结果: {}", i + 1, allReports.get(i));
    }

    // statistic over all iterations
    if (allResults4Stat.size() != 0 && queryModel.getExecIteration() > 1) {
      String mergedResports = HistogramReporter.statistic( allResults4Stat, queryModel.getReportStore());
      LOGGER.info("查询统计合并(ms): {}", mergedResports);
    } else {
      LOGGER.warn("Skip statistic for all iterations due to empty query results");
    }

    LOGGER.info("执行查询({})总耗时(ms): {}" , inputFile, StringUtils.join(end2EndDuration, ","));
  }

  private void load(List<DataLoadModel> dataLoads) throws IOException, SQLException {
    DataLoader loader = new DataLoader();
    Connection conn = ConnectionMgr.getInstance().borrowConnection();
    try {
      // TODO 加入统计

      for (DataLoadModel model : dataLoads) {
        if(model.getTables()==null){
          if(model.isTruncate()){
            loader.truncate(conn);
          }
          LOGGER.info("");
          loader.load(conn, model.getPath());
        }else{
          if(model.isTruncate()){
            loader.truncate(conn, model.getTables());
          }
          loader.load(conn, model.getPath(), model.getTables());
        }
      }
    } finally {
      ConnectionMgr.getInstance().returnConnection(conn);
    }
  }

  private List<QueryResult> query(List<QuerySlice> querySlices) throws InterruptedException {
    if (CollectionUtils.isEmpty(querySlices)) {
      return new ArrayList<>();
    }

    List<QuerySlice> threadQuerySlices = new ArrayList<>(querySlices.size());
    for (QuerySlice querySlice : querySlices) {
      for (int i = 0; i < querySlice.getThreads(); i++) {
        threadQuerySlices.add(querySlice);
      }
    }

    List<String> originSeq = threadQuerySlices.stream()
        .map(QuerySlice::getId)
        .collect(Collectors.toList());

    if (queryModel.isShuffleExecute()) {
      LOGGER.info("原始SQL语句执行顺序: " + StringUtils.join(originSeq, ","));
      // shuffle the queries
      QuerySlice[] simpleQuerySliceArray = threadQuerySlices.toArray(new QuerySlice[0]);
      Utils.shuffleArray(simpleQuerySliceArray);
      List<String> shuffledSeq = Arrays.stream(simpleQuerySliceArray)
          .map(QuerySlice::getId)
          .collect(Collectors.toList());
      LOGGER.info("打乱后的SQL语句执行顺序: " + StringUtils.join(shuffledSeq, ","));
      querySlices = Arrays.asList(simpleQuerySliceArray);
    }

    queryProcessor = new QueryProcessor(queryModel.getExecConcurrentSize());
    List<QueryResult> queryResults = queryProcessor.processBatch(querySlices);

    List<Long> timeSeq = queryResults.stream()
        .map(QueryResult::getDuration)
        .collect(Collectors.toList());

    LOGGER.info("SQL语句执行耗时(ms): " + StringUtils.join(timeSeq, ","));

    return queryResults;
  }

  @Override
  public void close() {
    ConnectionMgr.getInstance().close();
    if(queryProcessor!=null){
      queryProcessor.close();
    }
  }
}
