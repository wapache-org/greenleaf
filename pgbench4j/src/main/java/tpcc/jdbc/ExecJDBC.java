/*
 * ExecJDBC - Command line program to process SQL DDL statements, from
 *             a text input file, to any JDBC Data Source
 *
 * Copyright (C) 2004-2016, Denis Lussier
 * Copyright (C) 2016, Jan Wieck
 *
 */
package tpcc.jdbc;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.sql.*;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.postgresql.copy.CopyManager;
import org.postgresql.core.BaseConnection;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import tpcc.jTPCCUtil;


public class ExecJDBC {

    private static final Logger log = LoggerFactory.getLogger(ExecJDBC.class);

    /**
     * 系统属性:
     *  - prop
     *  - commandFile : sql文件
     *
     *
     *
     * @param args
     */
    public static void main(String[] args) {

        Connection conn = null;
        Statement stmt = null;
        String rLine = null;
        StringBuilder sql = new StringBuilder();


        try {

            Properties ini = new Properties();
            ini.load( new FileInputStream(System.getProperty("prop")));

            // Register jdbcDriver
            Class.forName(ini.getProperty( "driver" ));

            // make connection
            conn = DriverManager.getConnection(ini.getProperty("conn"), ini.getProperty("user"),ini.getProperty("password"));
            conn.setAutoCommit(true);

            // Create Statement
            stmt = conn.createStatement();

            // Open inputFile
            String commandFile = jTPCCUtil.getSysProp("commandFile", null);
            BufferedReader in = new BufferedReader(new FileReader(commandFile));

            // loop thru input file and concatenate SQL statement fragments
            int counter = 0;
            long start = System.currentTimeMillis();
            while((rLine = in.readLine()) != null) {

                String line = rLine.trim();

                if (line.length() != 0) {
                    if (line.startsWith("--")) {
//                      log.info(line);  // print comment line
                    } else {
                        if (line.endsWith("\\;")) {
                            sql.append(line.replaceAll("\\\\;", ";"));
                            sql.append("\n");
                        } else {
                            sql.append(line.replaceAll("\\\\;", ";"));
                            if (line.endsWith(";")) {
//                                String query = sql.toString();
                                long startQuery = System.currentTimeMillis();
                                 execJDBC(++counter, stmt, sql.substring(0, sql.length() - 1));

//                                stmt.execute(sql.substring(0, sql.length() - 1));
                                sql.setLength(0);
                            } else {
                                sql.append("\n");
                            }
                        }
                    }
                } //end if
            } //end while

            log.info("执行SQL总耗时: "+(System.currentTimeMillis()-start)+" 毫秒, 文件: "+commandFile);

            in.close();

        } catch(IOException | SQLException ie) {
            log.info(ie.getMessage());
        } catch(Exception e) {
            e.printStackTrace();
        //exit Cleanly
        } finally {
            try {
                if (conn !=null)
                   conn.close();
            } catch(SQLException se) {
                se.printStackTrace();
            } // end finally
        } // end try

    } // end main

    public static void copyFromFile(Connection connection, String tableName, String fields, String filePath, String with, String where) throws SQLException, IOException {
        if(with==null) with = "";
        if(where==null) where = "";
        try (FileInputStream input = new FileInputStream(filePath)){
            CopyManager copyManager = new CopyManager((BaseConnection)connection);
            String sql = "COPY " + tableName + " " + (fields == null ? "" : fields) + " FROM STDIN " + with + " " + where;
            log.info("执行COPY语句: {}", sql);
            copyManager.copyIn(sql, input);
        }
    }

    public static void execJDBC(int index, Statement stmt, String query) {

//        log.info(query + ";");

        try {
            if (query.startsWith("copy ")){
                long start = System.currentTimeMillis();
                String filePath = execCopy(stmt.getConnection(), query);
                long end = System.currentTimeMillis();
                if(filePath!=null){
                    long lines = Files.lines(Paths.get(filePath)).count();
                    long readFileLine = System.currentTimeMillis();

                    log.info("执行第 "+index+" 条SQL, 耗时 "+(end-start)+" 毫秒, 读取行数耗时 "
                        +(readFileLine-end)+" 毫秒, 行数 "+lines+" 行, 每秒处理 "+(lines*1000/(end-start))+" 行");
                }else{
                    log.warn("执行第 "+index+" 条SQL, 耗时 "+(end-start)+" 毫秒, 文件不存在: {}", filePath);
                }
            }else{
                long start = System.currentTimeMillis();
                stmt.execute(query);
                long end = System.currentTimeMillis();
                log.info("执行第 "+index+" 条SQL, 耗时 "+(end-start)+" 毫秒");
            }

        }catch(SQLException | IOException se) {
            log.info(se.getMessage());
        } // end try

    } // end execJDBCCommand

    static final String pattern1 = "copy[ ]+([_\\d\\w]+)[ ]+(\\([ ,_\\-\\d\\w]+\\))?[ ]+from[ ]+'(.+)'[ ]+(with .*)?([ ]+where .*)?.*";
    static final String pattern2 = "copy[ ]+([_\\d\\w]+)[ ]+from[ ]+'(.+)'[ ]+(with .*)?([ ]+where .*)?.*";

    public static String execCopy(Connection conn, String query) {
        try {
            Matcher matcher = Pattern.compile(pattern2).matcher(query);
            if(matcher.find()){
                String filePath = matcher.group(2);
                String with = matcher.group(3);
                String where = matcher.group(4);
                copyFromFile(conn, matcher.group(1), "", filePath, with, where);
                return filePath;
            }else{
                matcher = Pattern.compile(pattern1).matcher(query);
                if(matcher.find()){
                    String fields = matcher.group(2);
                    String filePath = matcher.group(3);
                    String with = matcher.group(4);
                    String where = matcher.group(5);
                    copyFromFile(conn, matcher.group(1), fields, filePath, with, where);
                    return filePath;
                }else {
                    log.info("COPY命令匹配失败: " + query);
                }
            }
        }catch(SQLException | IOException e) {
            log.error(e.getMessage(), e);
        } // end try

        return null;
    } // end execJDBCCommand

} // end ExecJDBC Class
