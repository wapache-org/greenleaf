package tpch.cli.e2e;

import java.util.List;

import tpch.cli.Procedure;
import tpch.cli.script.ScriptUtil;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TpchModel {

  public static class ModelWrapper {

    private static final Logger LOGGER = LoggerFactory.getLogger(ModelWrapper.class);

    private String preProcessScript;
    private String processMetaFilePath;
    private String postProcessScript;

    public ModelWrapper() {
    }

    public String getPreProcessScript() {
      return preProcessScript;
    }

    public void setPreProcessScript(String preProcessScript) {
      this.preProcessScript = preProcessScript;
    }

    public String getProcessMetaFilePath() {
      return processMetaFilePath;
    }

    public void setProcessMetaFilePath(String processMetaFilePath) {
      this.processMetaFilePath = processMetaFilePath;
    }

    public String getPostProcessScript() {
      return postProcessScript;
    }

    public void setPostProcessScript(String postProcessScript) {
      this.postProcessScript = postProcessScript;
    }

    public void process(Procedure procedure) throws Exception {

      if (StringUtils.isNotEmpty(preProcessScript)) {
        if (ScriptUtil.runBashFile(FileUtils.getFile(preProcessScript))) {
          LOGGER.info("Successed to run script " + preProcessScript);
        } else {
          LOGGER.error("Failed to run script " + preProcessScript);
        }
      }

      procedure.setInputFiles(processMetaFilePath);
      procedure.ignite();
      procedure.close();

      if (StringUtils.isNotBlank(postProcessScript)) {
        if (ScriptUtil.runBashFile(FileUtils.getFile(postProcessScript))) {
          LOGGER.info("Successed to run script " + postProcessScript);
        } else {
          LOGGER.error("Successed to run script " + postProcessScript);
        }
      }

    }

    @Override
    public String toString() {
      final StringBuffer sb = new StringBuffer("ModelWrapper{");
      sb.append("preProcessScript='").append(preProcessScript).append('\'');
      sb.append(", processMetaFilePath='").append(processMetaFilePath).append('\'');
      sb.append(", postProcessScript='").append(postProcessScript).append('\'');
      sb.append('}');
      return sb.toString();
    }
  }

  private ModelWrapper dataGen;
  private List<ModelWrapper> sqlExec;

  public TpchModel() {
  }

  public ModelWrapper getDataGen() {
    return dataGen;
  }

  public void setDataGen(ModelWrapper dataGen) {
    this.dataGen = dataGen;
  }

  public List<ModelWrapper> getSqlExec() {
    return sqlExec;
  }

  public void setSqlExec(List<ModelWrapper> sqlExec) {
    this.sqlExec = sqlExec;
  }

  @Override
  public String toString() {
    final StringBuffer sb = new StringBuffer("TpchModel{");
    sb.append("dataGen=").append(dataGen);
    sb.append(", sqlExec=").append(sqlExec);
    sb.append('}');
    return sb.toString();
  }

  public static final class ModelWrapperBuilder {

    private String preProcessScript;
    private String processMetaFilePath;
    private String postProcessScript;

    private ModelWrapperBuilder() {
    }

    public static ModelWrapperBuilder aModelWrapper() {
      return new ModelWrapperBuilder();
    }

    public ModelWrapperBuilder withPreProcessScript(String preProcessScript) {
      this.preProcessScript = preProcessScript;
      return this;
    }

    public ModelWrapperBuilder withProcessMetaFilePath(String processMetaFilePath) {
      this.processMetaFilePath = processMetaFilePath;
      return this;
    }

    public ModelWrapperBuilder withPostProcessScript(String postProcessScript) {
      this.postProcessScript = postProcessScript;
      return this;
    }
    public ModelWrapper build() {
      ModelWrapper modelWrapper = new ModelWrapper();
      modelWrapper.setPreProcessScript(preProcessScript);
      modelWrapper.setProcessMetaFilePath(processMetaFilePath);
      modelWrapper.setPostProcessScript(postProcessScript);
      return modelWrapper;
    }
  }

  public static final class TPCHModelBuilder {

    private ModelWrapper dataGen;
    private List<ModelWrapper> sqlExec;

    private TPCHModelBuilder() {
    }

    public static TPCHModelBuilder aTPCHModel() {
      return new TPCHModelBuilder();
    }

    public TPCHModelBuilder withDataGen(ModelWrapper dataGen) {
      this.dataGen = dataGen;
      return this;
    }

    public TPCHModelBuilder withSqlExec(List<ModelWrapper> sqlExec) {
      this.sqlExec = sqlExec;
      return this;
    }

    public TpchModel build() {
      TpchModel tPCHModel = new TpchModel();
      tPCHModel.setDataGen(dataGen);
      tPCHModel.setSqlExec(sqlExec);
      return tPCHModel;
    }
  }
}
