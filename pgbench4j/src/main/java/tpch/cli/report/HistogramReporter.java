package tpch.cli.report;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import tpch.cli.common.Utils;
import tpch.cli.jdbc.QueryResult;
import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HistogramReporter {

  private static final Logger LOGGER = LoggerFactory.getLogger(HistogramReporter.class);

  public static final String JSON_RPT_SUFFIX = "_rpt.json";
  public static final String TBL_RPT_SUFFIX = "_rpt.txt";

  public static String statistic(List<QueryResult> results, String reportStore) {
    List<QueryHistogram> histograms = new ArrayList<>();

    Set<String> queryTypes =
        results.stream().map(r -> r.getQuerySlice().getType()).collect(Collectors.toSet());
    queryTypes.add("ALL");
    // for each type of query, get the query statistic
    for (String type : queryTypes) {
      List<Long> durations = results.stream()
          .filter(r -> type.equals("ALL") || r.getQuerySlice().getType().equals(type))
          .map(QueryResult::getDuration).collect(Collectors.toList());
      QueryHistogram histogram = QueryHistogram.statisticList(type, durations);
      histograms.add(histogram);
    }

    return processHistogram(histograms, reportStore);
  }

  private static String processHistogram(List<QueryHistogram> histograms, String reportStore) {
    if (CollectionUtils.isEmpty(histograms)) {
      return "Empty histograms provided, will skip generating report";
    }
    histograms.sort(new Comparator<QueryHistogram>() {
      @Override
      public int compare(QueryHistogram o1, QueryHistogram o2) {
        return o1.getQuery().compareToIgnoreCase(o2.getQuery());
      }
    });

    TableFormatter tableFormatter = new TableFormatter(true);
    tableFormatter.setTitle(QueryHistogram.getTitle());

    for (QueryHistogram histogram : histograms) {
      tableFormatter.addRow(histogram.getRawValue());
    }

    String prettyStr = tableFormatter.toPrettyString();

    if (StringUtils.isNotBlank(reportStore)) {
      try {
        // generate json / pretty table format report for these histograms
        FileUtils.forceMkdir(FileUtils.getFile(reportStore));
        long ts = System.nanoTime();
        File jsonFile = FileUtils.getFile(reportStore + File.separator + ts + JSON_RPT_SUFFIX);
        File tableFile = FileUtils.getFile(reportStore + File.separator + ts + TBL_RPT_SUFFIX);
        FileUtils.deleteQuietly(jsonFile);
        FileUtils.deleteQuietly(tableFile);
        FileUtils.touch(jsonFile);
        FileUtils.touch(tableFile);
        Gson gson = new Gson();
        String histogramStr = gson.toJson(histograms);
        FileUtils.write(jsonFile, histogramStr, "utf-8");
        FileUtils.write(tableFile, prettyStr, "utf-8");
        LOGGER.info(String.format("Test reports are generated in path %s and %s",
            jsonFile.getAbsolutePath(), tableFile.getAbsolutePath()));
      } catch (IOException e) {
        LOGGER.error("Failed to write report to file", e);
      }
    }

    return prettyStr;
  }

  public static String mergeStatisticFromFile(String storePath, String... jsonStatFiles) {
    List<QueryHistogram> queryHistograms = new ArrayList<>();
    for (String file : jsonStatFiles) {
      queryHistograms.addAll(loadHistogram(file));
    }
    return processHistogram(queryHistograms, storePath);
  }

  private static List<QueryHistogram> loadHistogram(String jsonStatFile) {
    List<QueryHistogram> queryHistograms;
    Gson gson = new Gson();
    Reader reader = null;
    try {
      reader = new FileReader(jsonStatFile);

      queryHistograms = gson.fromJson(reader,
          new TypeToken<List<QueryHistogram>>() {
          }.getType());
      LOGGER.info("Loaded histogram: " + StringUtils.join(queryHistograms, ", "));
      return queryHistograms;
    } catch (Exception e) {
      LOGGER.error("Failed to load histogram from path " + jsonStatFile, e);
      queryHistograms = new ArrayList<>();
    } finally {
      Utils.closeQuietly(reader);
    }
    return queryHistograms;
  }
}
