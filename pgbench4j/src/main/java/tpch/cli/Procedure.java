package tpch.cli;

/**
 * 数据生成器.
 */
public interface Procedure {
  /**
   * 配置文件
   * @param inputFiles
   */
  void setInputFiles(String... inputFiles);

  /**
   * 开始生成数据
   * @throws Exception
   */
  void ignite() throws Exception;

  /**
   * 停止生成数据
   */
  void close();

}
