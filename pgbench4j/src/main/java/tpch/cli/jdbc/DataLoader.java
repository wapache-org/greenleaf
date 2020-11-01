package tpch.cli.jdbc;

import tpch.prestosql.*;

import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.sql.*;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.atomic.AtomicLong;

public class DataLoader {

    // jdbc batch

    public void truncate(Connection conn, String... tables) throws SQLException {
        Set<String> tableSet = new HashSet<>(Arrays.asList(tables));
        for (TpchTable<?> table : TpchTable.getTables()) {
            if(tables.length>0 && !tableSet.contains(table.getTableName())){
                continue;
            }
            try(Statement stmt = conn.createStatement()){
                stmt.execute("truncate table " + table.getTableName());
            }
        }
    }

    public void load(Connection conn, String path, String... tables) throws IOException, SQLException {
        load(conn, path, true, tables);
    }

    public void load(Connection conn, String path, boolean commit, String... tables) throws IOException, SQLException {
        Set<String> tableSet = new HashSet<>(Arrays.asList(tables));
        for (TpchTable<?> table : TpchTable.getTables()) {
            if(tables.length>0 && !tableSet.contains(table.getTableName())){
                continue;
            }
            StringBuilder sql = new StringBuilder();
            TpchColumnType.Base[] types = buildSql(
                table.getTableName(), table.getColumns().toArray(new TpchColumn<?>[]{}), sql
            );
            conn.setAutoCommit(false);
            try (PreparedStatement stmt = conn.prepareStatement(sql.toString())){
                loadTableData(stmt, types, Paths.get(path, table.getTableName()), commit);
            }
            if(commit){
                conn.commit();
            }
        }
    }

    private TpchColumnType.Base[] buildSql(String table, TpchColumn<?>[] columns, StringBuilder sql){
        TpchColumnType.Base[] types = new TpchColumnType.Base[columns.length];
        sql.append("insert into ").append(table).append(" (");
        for (int i = 0; i < columns.length; i++) {
            sql.append(i == 0 ? "" : ",").append(columns[i].getColumnName());
            types[i] = columns[i].getType().getBase();
        }
        sql.append(") values (");
        for (int i = 0; i < columns.length; i++) {
            sql.append(i == 0 ? "?" : ",?");
        }
        sql.append(")");
        return types;
    }

    void loadTableData(PreparedStatement stmt, TpchColumnType.Base[] columns, Path path, boolean commit) throws IOException, SQLException {
        if(Files.isDirectory(path)){
            Files.walkFileTree(path, new SimpleFileVisitor<Path>() {
                @Override
                public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                    try {
                        loadTableData(stmt, columns, file, commit);
                    } catch (SQLException e) {
                        throw new IOException(e);
                    }
                    return FileVisitResult.CONTINUE;
                }
            });
        }else if(Files.exists(path)){
            AtomicLong counter = new AtomicLong();
            Files.lines(path).forEach(line->{
                try {
                    String[] values = line.split("\\|");
                    if(columns.length!=values.length){
                        throw new RuntimeException(String.format(
                            "列值不一致, 列数: %d, 值数: %d, 数据行内容: %s",
                            columns.length, values.length, line
                        ));
                    }
                    for (int i = 0; i < columns.length; i++) {
                        setValue(stmt, i+1, columns[i], values[i]);
                    }
                    stmt.addBatch();
                    if(counter.incrementAndGet()>=10000){
                        stmt.executeBatch();
                        if(commit){
                            stmt.getConnection().commit();
                        }
                        counter.set(0);
                    }
                } catch (SQLException e) {
                    throw new RuntimeException(e);
                }
            });

            if(counter.get()>0){
                stmt.executeBatch();
                if(commit){
                    stmt.getConnection().commit();
                }
            }
        }
    }

    private void setValue(
        PreparedStatement stmt, int index, TpchColumnType.Base type, String value
    ) throws SQLException {
        switch(type){
        case VARCHAR:
            if(value.isEmpty()){
                stmt.setNull(index, Types.VARCHAR);
            }else{
                if(value.charAt(0)=='"'){
                    if(value.charAt(value.length()-1)=='"'){
                        stmt.setString(index, value.substring(1, value.length()-1));
                    }else{
                        stmt.setString(index, value);
                    }
                }else{
                    stmt.setString(index, value);
                }
            }
            break;
        case INTEGER:
            if(value.isEmpty()){
                stmt.setNull(index, Types.INTEGER);
            }else{
                stmt.setInt(index,Integer.parseInt(value));
            }
            break;
        case IDENTIFIER:
            if(value.isEmpty()){
                stmt.setNull(index, Types.BIGINT);
            }else{
                stmt.setLong(index,Long.parseLong(value));
            }
            break;
        case DOUBLE:
            if(value.isEmpty()){
                stmt.setNull(index, Types.DOUBLE);
            }else{
                stmt.setDouble(index, Double.parseDouble(value));
            }
            break;
        case DATE:
            if(value.isEmpty()){
                stmt.setNull(index, Types.DATE);
            }else{
                stmt.setDate(index, Date.valueOf(value));
            }
            break;
        }
    }

    // postgres copy

    // gpfdist | gds

}
