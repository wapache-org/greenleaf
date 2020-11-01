package tpch.cli.report;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

import org.apache.commons.lang3.StringUtils;

public final class TableFormatter {
  private static final String VERTICAL = "|";
  private static final String PLUS = "+";
  private static final String PAD_STR = " ";
  private static final int EXTRAL_SPACE_4_PAD = 2;

  private boolean isBuildByRow;
  private List<String> title;
  // store the values in vertical or horizontal and can transpose between them
  private List<List<String>> columns;
  private List<List<String>> rows;
  private List<Integer> maxWidthPerColumn;
  private int maxColumnNumber = 0;
  private int maxRowNumber = 0;

  public TableFormatter(boolean isBuildByRow) {
    this.isBuildByRow = isBuildByRow;
    if (isBuildByRow) {
      this.rows = new ArrayList<>();
    } else {
      this.columns = new ArrayList<>();
    }
    this.maxWidthPerColumn = new ArrayList<>();
  }

  public synchronized TableFormatter addRow(int index, List<String> row) {
    if (!isBuildByRow) {
      transpose();
    }
    if (row == null) {
      rows.add(index, new ArrayList<>());
    } else {
      rows.add(index, row);
      maxColumnNumber = maxColumnNumber > row.size() ? maxColumnNumber : row.size();

      // update the maxWidthPerColumn
      if (maxColumnNumber > maxWidthPerColumn.size()) {
        maxWidthPerColumn
            .addAll(Collections.nCopies(maxColumnNumber - maxWidthPerColumn.size(), 0));
      }
      List<Integer> thisWidthes =
          row.stream().map(String::length).collect(Collectors.toList());
      for (int i = 0; i < maxColumnNumber; i++) {
        if (maxWidthPerColumn.get(i) < thisWidthes.get(i)) {
          maxWidthPerColumn.set(i, thisWidthes.get(i));
        }
      }
    }
    maxRowNumber++;
    return this;
  }

  public TableFormatter addRow(List<String> row) {
    return addRow(rows.size(), row);
  }

  public synchronized TableFormatter addColumn(int index, List<String> column) {
    if (isBuildByRow) {
      transpose();
    }
    if (column == null) {
      columns.add(index, new ArrayList<>());
    } else {
      columns.add(index, column);
      maxRowNumber = maxRowNumber > column.size() ? maxRowNumber : column.size();
    }
    maxColumnNumber++;

    // update the maxWidthPerColumn
    if (maxColumnNumber > maxWidthPerColumn.size()) {
      maxWidthPerColumn.addAll(Collections.nCopies(maxColumnNumber - maxWidthPerColumn.size(), 0));
    }
    if (column != null) {
      Optional<Integer> maxLength =
          column.stream().map(String::length).max(new Comparator<Integer>() {
            @Override
            public int compare(Integer o1, Integer o2) {
              return o1.compareTo(o2);
            }
          });
      assert maxLength.isPresent();
      if (maxWidthPerColumn.get(index) < maxLength.get()) {
        maxWidthPerColumn.set(index, maxLength.get());
      }
    }
    return this;
  }

  public TableFormatter addColumn(List<String> column) {
    return addColumn(columns.size(), column);
  }

  public void setTitle(List<String> title) {
    if (null == title) {
      this.title = new ArrayList<>();
    } else {
      this.title = title;
    }
  }

  public synchronized String toPrettyString() {
    if (!isBuildByRow) {
      transpose();
    }

    // for title line
    addRow(0, title);

    final StringBuilder sb = new StringBuilder(System.lineSeparator());

    String sep4WholeLine = buildSeperatorLine();
    sb.append(PLUS).append(sep4WholeLine).append(PLUS).append(System.lineSeparator());

    for (int i = 0; i < maxRowNumber; i++) {
      List<String> value4PerCol = new ArrayList<>();
      for (int j = 0; j < maxColumnNumber; j++) {
        value4PerCol.add(
            padChars(rows.get(i).get(j), PAD_STR, maxWidthPerColumn.get(j) + EXTRAL_SPACE_4_PAD));
      }
      String valueLine = StringUtils.join(value4PerCol, VERTICAL);
      sb.append(VERTICAL).append(valueLine).append(VERTICAL).append(System.lineSeparator());
      // to make it more compact, we skip the separate line between each line
      if (i == 0 || i == maxRowNumber - 1) {
        sb.append(PLUS).append(sep4WholeLine).append(PLUS).append(System.lineSeparator());
      }
    }
    return sb.toString();
  }

 private synchronized void transpose() {
    if (isBuildByRow) {
      columns = new ArrayList<>();
      for (int colIdx = 0; colIdx < maxColumnNumber; colIdx++) {
        List<String> col = new ArrayList<>();
        for (int rowIdx = 0; rowIdx < maxRowNumber; rowIdx++) {
          col.add(rows.get(rowIdx).get(colIdx));
        }
        columns.add(col);
      }
      isBuildByRow = false;
    } else {
      rows = new ArrayList<>();
      for (int rowIdx = 0; rowIdx < maxRowNumber; rowIdx++) {
        List<String> row = new ArrayList<>();
        for (int colIdx = 0; colIdx < maxColumnNumber; colIdx++) {
          row.add(columns.get(colIdx).get(rowIdx));
        }
        rows.add(row);
      }
      isBuildByRow = true;
    }
 }

  private String buildSeperatorLine() {
    List<String> sep4EachCol = new ArrayList<>();
    for (int i = 0; i < maxWidthPerColumn.size(); i++) {
      sep4EachCol.add(duplicateChars("-", maxWidthPerColumn.get(i) + EXTRAL_SPACE_4_PAD));
    }
    return StringUtils.join(sep4EachCol, PLUS);
  }

  /**
   * pad `ch` to `str` to get `width` charaters
   */
  private String padChars(String str, String ch, int width) {
    assert str.length() <= width;
    int rightPad = (width - str.length()) / 2;
    int leftPad = width - rightPad - str.length();
    return duplicateChars(ch, leftPad) + str + duplicateChars(ch, rightPad);
  }

  private String duplicateChars(String ch, int num) {
    if (num <= 0) {
      return "";
    }

    StringBuffer sb = new StringBuffer();
    for (int i = 0; i < num; i++) {
      sb.append(ch);
    }
    return sb.toString();
  }
}
