package tpch.cli.e2e;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.google.gson.Gson;
import tpch.cli.CliTool;
import tpch.cli.common.TestUtil;
import tpch.cli.datagen.DataGenModel;
import tpch.cli.jdbc.QueryModel;
import tpch.cli.report.HistogramReporter;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TpchExecutorTest {
  private static final Logger LOGGER = LoggerFactory.getLogger(TpchExecutorTest.class);
  private String tpchModelMetaFile = "tpch_model_meta.json";
  private String dataGenMetaFile = "data_gen_meta.json";
  private String dataGenTargetPath = "data_gen_target";
  private String[] sqlStatementMetaFiles =
      new String[] { "sql_meta1.json", "sql_meta2.json", "sql_meta3.json" };
  private String[] queryModelReportPaths =
      new String[] { "sql_report1", "sql_report2", "sql_report3" };
  private String mergeReportPaths = "merge_report";

  @Before
  public void setUp() throws Exception {
    clearFile();
    prepareTpchModel();
  }

  @After
  public void tearDown() throws Exception {
    clearFile();
  }

  private void clearTempStore() {
    FileUtils.deleteQuietly(FileUtils.getFile(dataGenTargetPath));
    for (int i = 0; i < queryModelReportPaths.length; i++) {
      FileUtils.deleteQuietly(FileUtils.getFile(queryModelReportPaths[i]));
    }
    FileUtils.deleteQuietly(FileUtils.getFile(mergeReportPaths));
  }

  private void clearFile() {
    clearTempStore();
    FileUtils.deleteQuietly(FileUtils.getFile(dataGenMetaFile));
    for (int i = 0; i < sqlStatementMetaFiles.length; i++) {
      FileUtils.deleteQuietly(FileUtils.getFile(sqlStatementMetaFiles[i]));
    }
    FileUtils.deleteQuietly(FileUtils.getFile(tpchModelMetaFile));
  }

  private void prepareTpchModel() throws Exception {
    DataGenModel dataGenModel = TestUtil.prepareGeneratorModel(dataGenTargetPath);
    Gson gson = new Gson();
    TestUtil.writeToFile(gson.toJson(dataGenModel), dataGenMetaFile);

    for (int i = 0; i < sqlStatementMetaFiles.length; i++) {
      QueryModel queryModel = TestUtil.prepareQueryModel(queryModelReportPaths[i]);
      TestUtil.writeToFile(gson.toJson(queryModel), sqlStatementMetaFiles[i]);
    }

    TpchModel.ModelWrapper dataGenWrapper =
        TpchModel.ModelWrapperBuilder.aModelWrapper().withPreProcessScript("")
            .withProcessMetaFilePath(dataGenMetaFile).withPostProcessScript("").build();
    List<TpchModel.ModelWrapper> sqlExecWrappers = new ArrayList<>();
    for (int i = 0; i < sqlStatementMetaFiles.length; i++) {
      TpchModel.ModelWrapper sqlExecWrapper =
          TpchModel.ModelWrapperBuilder.aModelWrapper().withPreProcessScript("")
              .withProcessMetaFilePath(sqlStatementMetaFiles[i]).withPostProcessScript("").build();
      sqlExecWrappers.add(sqlExecWrapper);
    }

    TpchModel tpchModel = TpchModel.TPCHModelBuilder.aTPCHModel().withDataGen(dataGenWrapper)
        .withSqlExec(sqlExecWrappers).build();
    TestUtil.writeToFile(gson.toJson(tpchModel), tpchModelMetaFile);
  }

  private String[] getAllReportFiles(String[] reportParentPath) {
    List<File> reportFileList = new ArrayList<>();
    for (int i = 0; i < reportParentPath.length; i++) {
      File[] thisReportFiles = FileUtils.getFile(reportParentPath[i]).listFiles(new FileFilter() {
        @Override
        public boolean accept(File pathname) {
          return pathname.getName().endsWith(HistogramReporter.JSON_RPT_SUFFIX);
        }
      });
      if (thisReportFiles != null) {
        reportFileList.addAll(Arrays.asList(thisReportFiles));
      }
    }
    String[] reportFilePaths = new String[reportFileList.size()];
    for (int i = 0; i < reportFileList.size(); i++) {
      reportFilePaths[i] = reportFileList.get(i).getAbsolutePath();
    }
    return reportFilePaths;
  }

  @Test
  public void testTpchExecutor() throws Exception {
    TpchExecutor tpchExecutor = new TpchExecutor();
    tpchExecutor.setInputFiles(tpchModelMetaFile);
    tpchExecutor.ignite();
    tpchExecutor.close();

    // test merge report
    String[] reportFilePaths = getAllReportFiles(queryModelReportPaths);
    String mergeResult =
        HistogramReporter.mergeStatisticFromFile(mergeReportPaths, reportFilePaths);
    LOGGER.info(mergeResult);
  }

  @Test
  public void testCliTool() throws Exception {
    // print help
    String cmd = "-h";
    processCmd(cmd);

    clearTempStore();
    // generate data
    cmd = "-c gen_data -f " + dataGenMetaFile;
    processCmd(cmd);
    clearTempStore();

    clearTempStore();
    // execute queries
    for (int i = 0; i < sqlStatementMetaFiles.length; i++) {
      cmd = "-c exec_sql -f " + sqlStatementMetaFiles[i];
      processCmd(cmd);
    }
    clearTempStore();

    clearTempStore();
    // tpch
    cmd = "-c tpch -f " + tpchModelMetaFile;
    processCmd(cmd);

    // merge reports
    String[] reportFilePaths = getAllReportFiles(queryModelReportPaths);
    cmd =
        "-c merge_report -f " + StringUtils.join(reportFilePaths, " ") + " -o " + mergeReportPaths;
    processCmd(cmd);
  }

  private void processCmd(String cmd) {
    String[] args = cmd.split(" ");
    CliTool.main(args);
  }
}