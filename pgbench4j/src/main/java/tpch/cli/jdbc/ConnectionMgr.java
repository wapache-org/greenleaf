package tpch.cli.jdbc;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.concurrent.ConcurrentLinkedQueue;

import tpch.cli.common.Utils;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ConnectionMgr {

  private static final Logger LOGGER = LoggerFactory.getLogger(ConnectionMgr.class);

  private static final ConnectionMgr INSTANCE = new ConnectionMgr();
  private ConcurrentLinkedQueue<Connection> connections;

  private ConnectionMgr() {
    connections = new ConcurrentLinkedQueue<>();
  }

  public static ConnectionMgr getInstance() {
    return INSTANCE;
  }

  public synchronized void init(String driverName, String url, String user, String pwd, int size, String connInitQuery) {
    if (!connections.isEmpty()) {
      LOGGER.warn("Connection pool has already been inited before, will ignore it");
      return;
    }
    try {
      Class.forName(driverName);
    } catch (ClassNotFoundException e) {
      LOGGER.error("Failed to init driver", e);
      throw new IllegalArgumentException("Invalid driver: " + driverName);
    }

    for (int i = 0; i < size; i++) {
      connections.add(newConnection(url, user, pwd, connInitQuery));
    }
  }

  private Connection newConnection(String url, String user, String pwd, String connInitQuery) {
    LOGGER.debug("创建连接: 时间戳=" + System.currentTimeMillis());
    Connection conn = null;
    try {
      if (StringUtils.isBlank(user)) {
        conn = DriverManager.getConnection(url);
      } else {
        conn = DriverManager.getConnection(url, user, pwd);
      }

      if (StringUtils.isNotBlank(connInitQuery)) {
        conn.prepareStatement(connInitQuery).execute();
      }
    } catch (SQLException e) {
      LOGGER.error("创建数据库连接失败: 时间戳=" + System.currentTimeMillis(), e);
    }
    LOGGER.info("创建连接 " + conn);

    return conn;
  }

  public Connection borrowConnection() {
    Connection conn = connections.poll();
    LOGGER.trace("申请链接 {}" , conn);
    return conn;
  }

  public void returnConnection(Connection conn) {
    LOGGER.trace("归还连接 {}" , conn);
    connections.offer(conn);
  }

  public void close() {
    for (Connection conn : connections) {
      LOGGER.debug("关闭连接 " + conn);
      Utils.closeQuietly(conn);
    }
    connections.clear();
  }
}
