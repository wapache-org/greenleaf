package tpch.cli.datagen;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import tpch.prestosql.TpchEntity;
import tpch.prestosql.TpchTable;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

public class AirliftTpchUtil {

  private static final Logger LOGGER = LoggerFactory.getLogger(AirliftTpchUtil.class);

  private static final AirliftTpchUtil INSTANCE = new AirliftTpchUtil();

  private AirliftTpchUtil() {
  }

  public static AirliftTpchUtil getInstance() {
    return INSTANCE;
  }

  public void generateData4PerPart(
      String baseDirectory, String tpchTableName,
      double scalupFactor, int part, int partCnt
  ) throws IOException {

    String destPath = String.format("%s%s%s%s%s-part-%d.dat",
        baseDirectory, File.separator, tpchTableName, File.separator, tpchTableName, part);

    LOGGER.info("开始生成数据文件: {}", destPath);

    Writer writer = null;
    try {

      writer = new FileWriter(destPath);

      for (TpchEntity entity : TpchTable.getTable(tpchTableName).createGenerator(scalupFactor, part, partCnt)) {
        writer.write(entity.toLine());
        writer.write('\n');
      }
    } catch (IOException e) {
      LOGGER.error("Failed to generate data for table " + tpchTableName);
      throw e;
    } finally {
      if (writer != null) {
        try {
          writer.close();
        } catch (IOException e) {
          LOGGER.error("Failed to close writer for table " + tpchTableName);
        }
      }
    }
    LOGGER.info("生成数据文件成功: {}",destPath);
  }
}
