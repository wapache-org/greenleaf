package tpch.cli.common;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import tpch.cli.datagen.DataGenModel;
import tpch.cli.datagen.TableGenModel;
import tpch.cli.jdbc.QueryModel;
import tpch.cli.jdbc.QuerySlice;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TestUtil {

  private static final Logger LOGGER = LoggerFactory.getLogger(TestUtil.class);

  public static DataGenModel prepareGeneratorModel(String dataGenTargetPath) throws IOException {
    // 生成 customer表 数据
    TableGenModel tableGenModel1 = TableGenModel.TableGenModelBuilder
        .aTableGenModel()
        .withTpchTableName("customer")
        .withScaleupFactor(0.1)
        .withFilePartCnt(2)
        .build();
    // 生成 lineitem表 数据
    TableGenModel tableGenModel2 = TableGenModel.TableGenModelBuilder
        .aTableGenModel()
        .withTpchTableName("lineitem")
        .withScaleupFactor(0.1)
        .withFilePartCnt(3)
        .build();

    List<TableGenModel> tableGenModelList = new ArrayList<>();
    tableGenModelList.add(tableGenModel1);
    tableGenModelList.add(tableGenModel2);

    DataGenModel dataGenModel = DataGenModel.DataGenModelBuilder
        .aDataGenModel()
        .withTargetDirectory(dataGenTargetPath)
        .withTableGenModels(tableGenModelList)
        .build();
    return dataGenModel;
  }

  public static QueryModel prepareQueryModel(String reportStore) throws Exception {
    // 查询语句1
    QuerySlice querySlice1 = QuerySlice.QuerySliceBuilder.aQuerySlice()
        .withId("id1")
        .withSql("show tables")
        .withThreads(1)
        .withIsConsumeResult(true)
        .withIsCountInStatistics(true)
        .withType("type1")
        .build();
    // 查询语句2
    QuerySlice querySlice2 = QuerySlice.QuerySliceBuilder.aQuerySlice()
        .withId("id2")
        .withSql("show databases")
        .withThreads(5)
        .withIsConsumeResult(true)
        .withIsCountInStatistics(true)
        .withType("type2")
        .build();
    // 查询语句3
    QuerySlice querySlice3 = QuerySlice.QuerySliceBuilder.aQuerySlice()
        .withId("id3")
        .withSql("show tables;show databases")
        .withThreads(2)
        .withIsConsumeResult(true)
        .withIsCountInStatistics(false)
        .withType("type3")
        .build();

    List<QuerySlice> querySliceList = new ArrayList<>();
    querySliceList.add(querySlice1);
    querySliceList.add(querySlice2);
    querySliceList.add(querySlice3);

    QueryModel queryModel = QueryModel.QueryModelBuilder.aQueryModel()
        .withJdbcDriver("org.h2.Driver")
        .withJdbcUrl("jdbc:h2:~/test")
        .withJdbcUser("sa")
        .withJdbcPwd("")
        .withJdbcPoolSize(3)
        .withExecInterval(2)
        .withExecInterval(2)
        .withExecConcurrentSize(2)
        .withShuffleExecute(true)
        .withReportStore(reportStore)
        .withQuerySlices(querySliceList)
        .build();

    return queryModel;
  }

  public static void writeToFile(String content, String filePath) throws IOException {
    File file = FileUtils.getFile(filePath);
    file.getAbsoluteFile().mkdirs();
//    FileUtils.forceMkdirParent(file);
    if (file.exists()) {
      FileUtils.forceDelete(file);
    }
    FileUtils.touch(file);
    FileUtils.write(file, content, "utf-8");
  }
}
