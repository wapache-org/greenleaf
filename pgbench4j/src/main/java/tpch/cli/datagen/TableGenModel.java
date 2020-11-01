package tpch.cli.datagen;

import java.util.Collection;

import tpch.cli.Normalizer;
import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import tpch.prestosql.TpchTable;

public class TableGenModel implements Normalizer {

  private static final Logger LOGGER = LoggerFactory.getLogger(TableGenModel.class);

  private String tpchTableName;
  private double scaleupFactor;
  private int filePartCnt;

  public String getTpchTableName() {
    return tpchTableName;
  }

  public void setTpchTableName(String tpchTableName) {
    this.tpchTableName = tpchTableName;
  }

  public double getScaleupFactor() {
    return scaleupFactor;
  }

  public void setScaleupFactor(double scaleupFactor) {
    this.scaleupFactor = scaleupFactor;
  }

  public int getFilePartCnt() {
    return filePartCnt;
  }

  public void setFilePartCnt(int filePartCnt) {
    this.filePartCnt = filePartCnt;
  }

  @Override
  public void normalize() throws IllegalArgumentException {
    if (null == tpchTableName) {
      throw new IllegalArgumentException("'tpchTableName' is required in TableGenModel");
    }
    // validate tableName
    Collection<String> nativeTpchTables = CollectionUtils.collect(TpchTable.getTables(), TpchTable::getTableName);
    tpchTableName = tpchTableName.toLowerCase();
    if (!nativeTpchTables.contains(tpchTableName)) {
      LOGGER.error("表 {} 不是一个 TPCH 的表, 支持的表名有: {}",
              tpchTableName, StringUtils.join(nativeTpchTables, ", "));
      throw new IllegalArgumentException("不支持此表名: " + tpchTableName);
    }

    // todo: validate the others
  }

  @Override
  public String toString() {
    final StringBuffer sb = new StringBuffer("TableGenModel{");
    sb.append("tpchTableName='").append(tpchTableName).append('\'');
    sb.append(", scaleupFactor=").append(scaleupFactor);
    sb.append(", filePartCnt=").append(filePartCnt);
    sb.append('}');
    return sb.toString();
  }

  public static final class TableGenModelBuilder {
    private String tpchTableName;
    private double scaleupFactor;
    private int filePartCnt;

    private TableGenModelBuilder() {
    }

    public static TableGenModelBuilder aTableGenModel() {
      return new TableGenModelBuilder();
    }

    public TableGenModelBuilder withTpchTableName(String tpchTableName) {
      this.tpchTableName = tpchTableName;
      return this;
    }

    public TableGenModelBuilder withScaleupFactor(double scaleupFactor) {
      this.scaleupFactor = scaleupFactor;
      return this;
    }

    public TableGenModelBuilder withFilePartCnt(int filePartCnt) {
      this.filePartCnt = filePartCnt;
      return this;
    }

    public TableGenModel build() {
      TableGenModel tableGenModel = new TableGenModel();
      tableGenModel.setTpchTableName(tpchTableName);
      tableGenModel.setScaleupFactor(scaleupFactor);
      tableGenModel.setFilePartCnt(filePartCnt);
      return tableGenModel;
    }
  }
}
