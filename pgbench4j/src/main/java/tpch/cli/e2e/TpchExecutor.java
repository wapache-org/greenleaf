package tpch.cli.e2e;

import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.List;

import com.google.gson.Gson;
import tpch.cli.Procedure;
import tpch.cli.datagen.DataGenerator;
import tpch.cli.jdbc.QueryClient;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TpchExecutor implements Procedure {

  private static final Logger LOGGER = LoggerFactory.getLogger(TpchExecutor.class);

  private String tpchModelPath;

  private TpchModel tpchModel;

  public TpchExecutor() {
  }

  @Override
  public void setInputFiles(String... inputFiles) {
    tpchModelPath = inputFiles[0];
  }

  public void ignite() throws Exception {
    loadTpchModel();

    TpchModel.ModelWrapper dataGenWrapper = tpchModel.getDataGen();
    if (dataGenWrapper != null) {
      LOGGER.debug("Begin to process dataGen {}" , dataGenWrapper);
      DataGenerator dataGenerator = new DataGenerator();
      dataGenWrapper.process(dataGenerator);
    } else {
      LOGGER.debug("Data generate is not configured, will skip it");
    }

    List<TpchModel.ModelWrapper> sqlExecWrappers = tpchModel.getSqlExec();
    if (sqlExecWrappers != null && sqlExecWrappers.size() > 0) {
      for (int i = 0; i < sqlExecWrappers.size(); i++) {
        LOGGER.debug("Begin to process sqlExec {}", sqlExecWrappers);
        QueryClient queryClient = new QueryClient();
        sqlExecWrappers.get(i).process(queryClient);
      }
    } else {
      LOGGER.debug("Sql execution is not configured, will skip it");
    }

  }

  private void loadTpchModel() throws IOException {
    Gson gson = new Gson();
    Reader reader = null;
    try {
      reader = new FileReader(tpchModelPath);
      tpchModel = gson.fromJson(reader, TpchModel.class);
      LOGGER.info("Load tpch model: " + tpchModelPath);
    } catch (IOException e) {
      LOGGER.error("Failed to load tpch model from path " + tpchModelPath, e);
      throw e;
    } finally {
      if (null != reader) {
        reader.close();
      }
    }
  }

  @Override
  public void close() {

  }
}
