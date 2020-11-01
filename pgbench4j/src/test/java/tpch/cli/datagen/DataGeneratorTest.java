package tpch.cli.datagen;

import java.io.IOException;

import com.google.gson.Gson;
import tpch.cli.common.TestUtil;
import org.apache.commons.io.FileUtils;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class DataGeneratorTest {

  private static final Logger LOGGER = LoggerFactory.getLogger(DataGeneratorTest.class);

  private String dataGeneratorMetaPath = "test/data_gen_meta.json";
  private String dataGeneratorTargetPath = "test/data_gen_target_path";

  @Before
  public void setUp() throws Exception {
    prepareGeneratorModel();
    FileUtils.deleteQuietly(FileUtils.getFile(dataGeneratorTargetPath));
  }

  @After
  public void tearDown() throws Exception {
    FileUtils.deleteQuietly(FileUtils.getFile(dataGeneratorMetaPath));
    FileUtils.deleteQuietly(FileUtils.getFile(dataGeneratorTargetPath));
  }

  private void prepareGeneratorModel() throws IOException {
    DataGenModel dataGenModel = TestUtil.prepareGeneratorModel(dataGeneratorTargetPath);
//    LOGGER.info("Prepare data generator model: " + dataGenModel);
    Gson gson = new Gson();
    String jsonStr = gson.toJson(dataGenModel);
//    LOGGER.info("生成测试配置, 文件名: {}, 文件内容: {}", dataGeneratorMetaPath, jsonStr);
    TestUtil.writeToFile(jsonStr, dataGeneratorMetaPath);
  }

  @Test
  public void testDataGenerator() throws Exception {
    DataGenerator dataGenerator = new DataGenerator();
    dataGenerator.setInputFiles(dataGeneratorMetaPath);
    long start = System.currentTimeMillis();
    dataGenerator.ignite();
    LOGGER.info("生成数据耗时: {} 毫秒", (System.currentTimeMillis() - start));
    dataGenerator.close();
  }

}
