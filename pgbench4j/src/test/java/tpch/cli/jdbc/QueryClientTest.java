package tpch.cli.jdbc;

import java.io.File;
import java.io.FileFilter;

import com.google.gson.Gson;
import tpch.cli.common.TestUtil;
import tpch.cli.report.HistogramReporter;
import org.apache.commons.io.FileUtils;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class QueryClientTest {

  private static final Logger LOGGER = LoggerFactory.getLogger(QueryClientTest.class);

  private String sqlMetaFile = "test/sql_meta_test.txt";
  private String rptStorePath = "test/report";
  private String mergedTargetrPath = "test/merged_stat";

  @Before
  public void setUp() throws Exception {
    for (String path : new String[] { sqlMetaFile, rptStorePath, mergedTargetrPath }) {
      FileUtils.deleteQuietly(FileUtils.getFile(path));
    }
    prepareQueryModel();
  }

  @After
  public void tearDown() throws Exception {
    for (String path : new String[] { sqlMetaFile, rptStorePath, mergedTargetrPath }) {
      FileUtils.deleteQuietly(FileUtils.getFile(path));
    }
  }

  private void prepareQueryModel() throws Exception {
    QueryModel queryModel = TestUtil.prepareQueryModel(rptStorePath);
//    LOGGER.info("Prepare query model: " + queryModel);
    Gson gson = new Gson();
    String jsonStr = gson.toJson(queryModel);
//    LOGGER.info("Write query model in json format: " + jsonStr);
    TestUtil.writeToFile(jsonStr, sqlMetaFile);
  }

  @Test
  public void testQueryClient() throws Exception {
    QueryClient queryClient = new QueryClient();
    queryClient.setInputFiles(sqlMetaFile);
    queryClient.ignite();
    queryClient.close();
  }

  @Test
  public void testMergeStatistic() throws Exception {
    QueryClient queryClient = new QueryClient();
    // query 2 times will result in 2 statistic file
    queryClient.setInputFiles(sqlMetaFile);
    queryClient.ignite();
    queryClient.ignite();

    File[] jsonRptFiles = FileUtils.getFile(rptStorePath).listFiles(new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        return pathname.getName().endsWith(HistogramReporter.JSON_RPT_SUFFIX);
      }
    });
    assert jsonRptFiles != null;

    String[] jsonRptFilePaths = new String[jsonRptFiles.length];
    for (int i = 0; i < jsonRptFiles.length; i++) {
      jsonRptFilePaths[i] = jsonRptFiles[i].getAbsolutePath();
    }
    String mergedResult =
        HistogramReporter.mergeStatisticFromFile(mergedTargetrPath, jsonRptFilePaths);
    LOGGER.info(mergedResult);

    queryClient.close();
  }
}