package tpch.cli.jdbc;

import java.util.Arrays;

public class DataLoadModel {

    String path;

    boolean truncate;

    String[] tables;

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public boolean isTruncate() {
        return truncate;
    }

    public void setTruncate(boolean truncate) {
        this.truncate = truncate;
    }

    public String[] getTables() {
        return tables;
    }

    public void setTables(String[] tables) {
        this.tables = tables;
    }

    @Override
    public String toString() {
        return "DataLoadModel{" +
            "path='" + path + '\'' +
            ", truncate=" + truncate +
            ", tables=" + Arrays.toString(tables) +
            '}';
    }
}
