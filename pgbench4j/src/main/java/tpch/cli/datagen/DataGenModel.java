package tpch.cli.datagen;

import java.io.File;
import java.util.Iterator;
import java.util.List;
import java.util.stream.Collectors;

import tpch.cli.Normalizer;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class DataGenModel implements Normalizer {

  private static final Logger LOGGER = LoggerFactory.getLogger(DataGenModel.class);

  private String targetDirectory;
  private List<TableGenModel> tableGenModels;

  public String getTargetDirectory() {
    return targetDirectory;
  }

  public void setTargetDirectory(String targetDirectory) {
    this.targetDirectory = targetDirectory;
  }

  public List<TableGenModel> getTableGenModels() {
    return tableGenModels;
  }

  public void setTableGenModels(List<TableGenModel> tableGenModels) {
    this.tableGenModels = tableGenModels;
  }

  @Override
  public void normalize() throws IllegalArgumentException {
    if (null == targetDirectory) {
      throw new IllegalArgumentException("'targetDirectory' is required in DataGenModel");
    }

    File targetDir = FileUtils.getFile(targetDirectory);
    if (!targetDir.exists() && !targetDir.mkdirs()) {
      throw new IllegalArgumentException("数据文件保存路径创建失败: " + targetDir.getAbsolutePath());
    }

    if (null == tableGenModels) {
      throw new IllegalArgumentException("'tableGenModel' is required in DataGenModel");
    }

    Iterator<TableGenModel> iterator = tableGenModels.iterator();
    while (iterator.hasNext()) {
      TableGenModel tableGenModel = iterator.next();
      try {
        tableGenModel.normalize();
      } catch (IllegalArgumentException e) {
        LOGGER.error("Illegal tableGenModel is provided ({}), will skip this.", tableGenModel, e);
        iterator.remove();
      }
    }

    int sizeAfterDeDuplicate = tableGenModels.stream()
        .map(TableGenModel::getTpchTableName).collect(Collectors.toSet()).size();
    if (sizeAfterDeDuplicate < tableGenModels.size()) {
      throw new IllegalArgumentException("表名有重复");
    }
  }

  @Override
  public String toString() {
    final StringBuffer sb = new StringBuffer("DataGenModel{");
    sb.append("targetDirectory='").append(targetDirectory).append('\'');
    sb.append(", tableGenModels=").append(tableGenModels);
    sb.append('}');
    return sb.toString();
  }

  public static final class DataGenModelBuilder {
    private String targetDirectory;
    private List<TableGenModel> tableGenModels;

    private DataGenModelBuilder() {
    }

    public static DataGenModelBuilder aDataGenModel() {
      return new DataGenModelBuilder();
    }

    public DataGenModelBuilder withTargetDirectory(String targetDirectory) {
      this.targetDirectory = targetDirectory;
      return this;
    }

    public DataGenModelBuilder withTableGenModels(List<TableGenModel> tableGenModels) {
      this.tableGenModels = tableGenModels;
      return this;
    }

    public DataGenModel build() {
      DataGenModel dataGenModel = new DataGenModel();
      dataGenModel.setTargetDirectory(targetDirectory);
      dataGenModel.setTableGenModels(tableGenModels);
      return dataGenModel;
    }
  }
}
