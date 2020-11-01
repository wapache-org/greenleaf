package tpch.cli.cli;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.Gson;
import tpch.cli.CliTool;
import tpch.cli.common.TestUtil;
import tpch.cli.e2e.TpchModel;
import org.apache.commons.io.FileUtils;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

/**
 * Functional tests for all scenarios
 */
public class CliToolTest {

  private static final File TEST_DATA_GEN_OUT = FileUtils.getFile("test/test_gen_out");
  private static final File TEST_QUERY_REPORT_OUT = FileUtils.getFile("test/test_report_out");
  private static final File TEST_TPCH_META = FileUtils.getFile("test/test_tpch_meta.json");
  private static final File TEST_MERGE_REPORT_OUT = FileUtils.getFile("test/test_merge_report_out");

  private final String testResourceRoot = getClass().getClassLoader().getResource("jsonfiles").getPath();

  @Rule
  public ExpectedException thrownException = ExpectedException.none();

  @Before
  public void setUp() throws Exception {
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_DATA_GEN_OUT));
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_QUERY_REPORT_OUT));
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_TPCH_META));
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_MERGE_REPORT_OUT));
  }

  @After
  public void tearDown() throws Exception {
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_DATA_GEN_OUT));
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_QUERY_REPORT_OUT));
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_TPCH_META));
    FileUtils.deleteQuietly(FileUtils.getFile(TEST_MERGE_REPORT_OUT));
  }

  private void processCmd(String cmd) {
    String[] args = cmd.split(" ");
    CliTool.main(args);
  }

  @Test
  public void testDataGenSuccess() throws Exception {
    String cmd = String.format("-c gen_data -f %s", testResourceRoot + "/gen_data/success_example.json" );
    processCmd(cmd);

    File[] dataGenDirs = TEST_DATA_GEN_OUT.listFiles();
    Assert.assertNotNull(dataGenDirs);
    Assert.assertEquals("2 directories is generated", 2, dataGenDirs.length);
    for (int i = 0; i < dataGenDirs.length; i++) {
      if (dataGenDirs[i].getName().equals("nation")) {
        Assert.assertEquals("directory nation contains 2 files", 2, dataGenDirs[i].listFiles().length);
      } else if (dataGenDirs[i].getName().equals("region")) {
        Assert.assertEquals("directory region contains 3 files", 3, dataGenDirs[i].listFiles().length);
      } else {
        Assert.fail("should not reach here");
      }
    }
  }

  /**
   * for a not tpch table, we just ignore it instead of throwing error.
   * This test is for some of the table are not tpch table, which the others are.
   */
  @Test
  public void testDataGen4UnsupportedSomeTable() throws Exception {
    String cmd = String.format("-c gen_data -f %s", testResourceRoot + "/gen_data/success_some_table_not_tpch.json" );
    processCmd(cmd);

    File[] dataGenDirs = TEST_DATA_GEN_OUT.listFiles();
    Assert.assertNotNull(dataGenDirs);
    Assert.assertEquals("only one directory is generated", 1, dataGenDirs.length);
    Assert.assertTrue("should contains directory name 'nation'", dataGenDirs[0].getName().equals("nation"));
    Assert.assertEquals("directory nation contains 2 files", 2, dataGenDirs[0].listFiles().length);
  }

  /**
   * for a not tpch table, we just ignore it instead of throwing error.
   * This test is for all the tables are not tpch table.
   */
  @Test
  public void testDataGen4UnsupportedAllTable() throws Exception {
    String cmd = String.format("-c gen_data -f %s", testResourceRoot + "/gen_data/success_all_table_not_tpch.json" );
    processCmd(cmd);

    File[] dataGenDirs = TEST_DATA_GEN_OUT.listFiles();
    Assert.assertNotNull(dataGenDirs);
    Assert.assertEquals("no data will be generated", 0, dataGenDirs.length);
  }

  /**
   * tableGenModels is empty
   */
  @Test
  public void testDataGen4EmptyTableGenModels() throws Exception {
    String cmd = String.format("-c gen_data -f %s", testResourceRoot + "/gen_data/success_empty_table_gen_model.json" );
    processCmd(cmd);

    File[] dataGenDirs = TEST_DATA_GEN_OUT.listFiles();
    Assert.assertNotNull(dataGenDirs);
    Assert.assertEquals("no data will be generated", 0, dataGenDirs.length);
  }

  /**
   * tableGenModels is missing
   */
  @Test
  public void testDataGen4MissingTableGenModels() throws Exception {
    String cmd = String.format("-c gen_data -f %s", testResourceRoot + "/gen_data/fail_missing_table_gen_model.json" );
    thrownException.expect(IllegalArgumentException.class);
    thrownException.expectMessage("'tableGenModel' is required in DataGenModel");
    processCmd(cmd);
  }

  /**
   * targetDirectory in dataGenModel is missing
   */
  @Test
  public void testDataGen4MissingTargetDirectory() throws Exception {
    String cmd = String.format("-c gen_data -f %s", testResourceRoot + "/gen_data/fail_missing_target_gen_directory.json" );
    thrownException.expect(IllegalArgumentException.class);
    thrownException.expectMessage("'targetDirectory' is required in DataGenModel");
    processCmd(cmd);
  }

  /**
   * duplicate tpch tables are not allowed
   */
  @Test
  public void testDataGen4DuplicatedTpchTable() throws Exception {
    String cmd = String.format("-c gen_data -f %s", testResourceRoot + "/gen_data/fail_duplicate_tpch_table.json" );
    thrownException.expect(IllegalArgumentException.class);
    thrownException.expectMessage("Duplicate tpch table names are found");
    processCmd(cmd);
  }

  @Test
  public void testExecSqlSuccess() throws Exception {
    String cmd = String.format("-c exec_sql -f %s", testResourceRoot + "/exec_sql/success_example.json" );
    processCmd(cmd);

    File[] queryReport = TEST_QUERY_REPORT_OUT.listFiles();
    Assert.assertNotNull(queryReport);
    // for each iteration, there will be 2 reports, and an extra merge iteration will be there if
    // more than 1 iteration
    Assert.assertEquals("reports will be generated", 6, queryReport.length);
  }

  /**
   * exe_sql will never fail, because we want to execute as many sqls as we can, so we just ignore the errors
   */
  @Test
  public void testExecSql4WrongCollection() throws Exception {
    String cmd = String.format("-c exec_sql -f %s", testResourceRoot + "/exec_sql/success_wrong_connection_url.json" );
    processCmd(cmd);
    Assert.assertTrue("no report will be generated", !TEST_QUERY_REPORT_OUT.exists());
  }

  /**
   * some of the sqls fail
   */
  @Test
  public void testExecSql4SomeSqlFail() throws Exception {
    String cmd = String.format("-c exec_sql -f %s", testResourceRoot + "/exec_sql/success_some_sql_fail.json" );
    processCmd(cmd);

    File[] queryReport = TEST_QUERY_REPORT_OUT.listFiles();
    Assert.assertNotNull(queryReport);
    Assert.assertEquals("reports will be generated", 2, queryReport.length);
  }

  /**
   * all the sqls fail
   */
  @Test
  public void testExecSql4AllSqlFail() throws Exception {
    String cmd = String.format("-c exec_sql -f %s", testResourceRoot + "/exec_sql/success_all_sql_fail.json" );
    processCmd(cmd);

    Assert.assertTrue("should not generate query report", !TEST_QUERY_REPORT_OUT.exists());
  }

  /**
   * querySlices is empty
   */
  @Test
  public void testExecSql4EmptyQuerySlices() throws Exception {
    String cmd = String.format("-c exec_sql -f %s", testResourceRoot + "/exec_sql/success_empty_sqls.json" );
    processCmd(cmd);
    Assert.assertTrue("should not generate query report", !TEST_QUERY_REPORT_OUT.exists());
  }

  /**
   * querySlices is missing
   */
  @Test
  public void testExecSql4MissingQuerySlices() throws Exception {
    String cmd = String.format("-c exec_sql -f %s", testResourceRoot + "/exec_sql/success_missing_sqls.json" );
    processCmd(cmd);
    Assert.assertTrue("should not generate query report", !TEST_QUERY_REPORT_OUT.exists());
  }

  private void prepareTpchModel() throws Exception {
    TpchModel.ModelWrapper dataGenWrapper = TpchModel.ModelWrapperBuilder.aModelWrapper()
        .withPreProcessScript("")
        .withProcessMetaFilePath(testResourceRoot + "/gen_data/success_example.json")
        .withPostProcessScript("")
        .build();
    List<TpchModel.ModelWrapper> sqlExecWrappers = new ArrayList<>();
    for (int i = 0; i < 2; i++) {
      TpchModel.ModelWrapper sqlExecWrapper = TpchModel.ModelWrapperBuilder.aModelWrapper()
          .withPreProcessScript("")
          .withProcessMetaFilePath(testResourceRoot + "/exec_sql/success_example.json")
          .withPostProcessScript("")
          .build();
      sqlExecWrappers.add(sqlExecWrapper);
    }

    TpchModel tpchModel = TpchModel.TPCHModelBuilder.aTPCHModel().withDataGen(dataGenWrapper)
        .withSqlExec(sqlExecWrappers).build();
    TestUtil.writeToFile(new Gson().toJson(tpchModel), TEST_TPCH_META.getAbsolutePath());
  }

  @Test
  public void testTpchSuccess() throws Exception {
    prepareTpchModel();
    String cmd = String.format("-c tpch -f %s", TEST_TPCH_META.getAbsolutePath());
    processCmd(cmd);

    File[] dataGenDirs = TEST_DATA_GEN_OUT.listFiles();
    Assert.assertNotNull(dataGenDirs);
    Assert.assertEquals("2 directories is generated", 2, dataGenDirs.length);
    for (int i = 0; i < dataGenDirs.length; i++) {
      if (dataGenDirs[i].getName().equals("nation")) {
        Assert.assertEquals("directory nation contains 2 files", 2, dataGenDirs[i].listFiles().length);
      } else if (dataGenDirs[i].getName().equals("region")) {
        Assert.assertEquals("directory region contains 3 files", 3, dataGenDirs[i].listFiles().length);
      } else {
        Assert.fail("should not reach here");
      }
    }

    File[] queryReport = TEST_QUERY_REPORT_OUT.listFiles();
    Assert.assertNotNull(queryReport);
    // for per exec_sql per iteration, there will be 2 reports, and an extra merge iteration
    // will be there if more than 1 iteration
    Assert.assertEquals("reports will be generated", 12, queryReport.length);
  }

  private void prepareTpchModelWithoutDataGen() throws Exception {
    List<TpchModel.ModelWrapper> sqlExecWrappers = new ArrayList<>();
    for (int i = 0; i < 2; i++) {
      TpchModel.ModelWrapper sqlExecWrapper = TpchModel.ModelWrapperBuilder.aModelWrapper()
          .withPreProcessScript("")
          .withProcessMetaFilePath(testResourceRoot + "/exec_sql/success_example.json")
          .withPostProcessScript("")
          .build();
      sqlExecWrappers.add(sqlExecWrapper);
    }

    TpchModel tpchModel = TpchModel.TPCHModelBuilder.aTPCHModel()
        .withSqlExec(sqlExecWrappers).build();
    TestUtil.writeToFile(new Gson().toJson(tpchModel), TEST_TPCH_META.getAbsolutePath());
  }

  /**
   * tpch model can work with no dataGen procedure
   */
  @Test
  public void testTpchMissingDataGen() throws Exception {
    prepareTpchModelWithoutDataGen();
    String cmd = String.format("-c tpch -f %s", TEST_TPCH_META.getAbsolutePath());
    processCmd(cmd);

    File[] queryReport = TEST_QUERY_REPORT_OUT.listFiles();
    Assert.assertNotNull(queryReport);
    Assert.assertEquals("reports will be generated", 12, queryReport.length);
  }

  private void prepareTpchModelWithoutSqlExec() throws Exception {
    TpchModel.ModelWrapper dataGenWrapper = TpchModel.ModelWrapperBuilder.aModelWrapper()
        .withPreProcessScript("")
        .withProcessMetaFilePath(testResourceRoot + "/gen_data/success_example.json")
        .withPostProcessScript("")
        .build();

    TpchModel tpchModel = TpchModel.TPCHModelBuilder.aTPCHModel()
        .withDataGen(dataGenWrapper)
        .build();
    TestUtil.writeToFile(new Gson().toJson(tpchModel), TEST_TPCH_META.getAbsolutePath());
  }

  /**
   * tpch model can work with no dataGen procedure
   */
  @Test
  public void testTpchMissingSqlExec() throws Exception {
    prepareTpchModelWithoutSqlExec();
    String cmd = String.format("-c tpch -f %s", TEST_TPCH_META.getAbsolutePath());
    processCmd(cmd);

    File[] dataGenDirs = TEST_DATA_GEN_OUT.listFiles();
    Assert.assertNotNull(dataGenDirs);
    Assert.assertEquals("2 directories is generated", 2, dataGenDirs.length);
    Assert.assertTrue("no reports will be generated", !TEST_QUERY_REPORT_OUT.exists());
  }

  @Test
  public void testMergeRptSuccess() throws Exception {
    String cmd = String.format("-c merge_report -f %s %s -o %s",
        testResourceRoot + "/merge_report/rpt1.json",
        testResourceRoot + "/merge_report/rpt2.json",
        TEST_MERGE_REPORT_OUT);
    processCmd(cmd);
    File[] mergeReport = TEST_MERGE_REPORT_OUT.listFiles();

    Assert.assertNotNull(mergeReport);
    Assert.assertEquals("merge report will be generated", 2, mergeReport.length);
  }

  @Test
  public void testMergeRptWithSomeIllegal() throws Exception {
    String cmd = String.format("-c merge_report -f %s %s %s -o %s",
        testResourceRoot + "/merge_report/rpt1.json",
        testResourceRoot + "/gen_data/success_example.json",
        testResourceRoot + "/merge_report/rpt2.json",
        TEST_MERGE_REPORT_OUT);
    processCmd(cmd);
    File[] mergeReport = TEST_MERGE_REPORT_OUT.listFiles();

    Assert.assertNotNull(mergeReport);
    Assert.assertEquals("merge report will be generated, but only contain some result", 2, mergeReport.length);
  }

  @Test
  public void testMergeRptWithAllIllegal() throws Exception {
    String cmd = String.format("-c merge_report -f %s %s -o %s",
        testResourceRoot + "/gen_data/success_example.json",
        testResourceRoot + "/gen_data/success_example.json",
        TEST_MERGE_REPORT_OUT);
    processCmd(cmd);

    Assert.assertTrue("no merge report will be generated", !TEST_MERGE_REPORT_OUT.exists());
  }
}
