package tpch.cli;

import java.io.IOException;

import tpch.cli.datagen.DataGenerator;
import tpch.cli.e2e.TpchExecutor;
import tpch.cli.jdbc.QueryClient;
import tpch.cli.report.HistogramReporter;
import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class CliTool {

  private static final Logger LOGGER = LoggerFactory.getLogger(CliTool.class);

  private Options options;
  private String command;
  private String[] inputFiles;
  private String outputPath;

  public CliTool() {
    this.options = initSupportedOptions();
  }

  private Options initSupportedOptions() {
    Options options = new Options();
    Option help = Option.builder("h")
        .argName("help")
        .hasArg()
        .desc("print this message")
        .optionalArg(true)
        .longOpt("help")
        .build();
    options.addOption(help);

    Option cmd = Option.builder("c")
        .argName("cmd")
        .hasArg()
        .desc("command to execute, supported commands are: tpch, gen_data, exec_sql, merge_report")
        .optionalArg(true)
        .longOpt("command")
        .numberOfArgs(1)
        .build();
    options.addOption(cmd);

    Option files = Option.builder("f")
        .argName("file")
        .hasArg()
        .desc("input files")
        .optionalArg(true)
        .longOpt("file")
        .numberOfArgs(Option.UNLIMITED_VALUES)
        .build();
    options.addOption(files);

    Option outputFile = Option.builder("o")
        .argName("output path")
        .hasArg()
        .desc("output path for the merged reports")
        .optionalArg(true)
        .longOpt("output")
        .numberOfArgs(1)
        .build();
    options.addOption(outputFile);

    return options;
  }

  private boolean parseCmdLine(String[] args) {
    CommandLineParser cmdParser = new DefaultParser();
    CommandLine cmdLine;
    try {
      cmdLine = cmdParser.parse(options, args);
    } catch (ParseException e) {
      LOGGER.error("Failed to parse command line", e);
      System.err.println("Failed to parse command line");
      return false;
    }

    if (cmdLine.hasOption("h")) {
      printHelp();
      return false;
    }

    if (cmdLine.hasOption("c")) {
      this.command = cmdLine.getOptionValue("c");
    }
    if (StringUtils.isBlank(this.command)) {
      LOGGER.error("Should specify a command");
      System.err.println("Should specify a command");
      return false;
    }

    if (cmdLine.hasOption("f")) {
      this.inputFiles = cmdLine.getOptionValues("f");
    }
    if (this.inputFiles == null || this.inputFiles.length == 0) {
      LOGGER.error("Should specify input files");
      System.err.println("Should specify input files");
      return false;
    }

    if (command.equalsIgnoreCase("merge_report")) {
      if (cmdLine.hasOption("o")) {
        this.outputPath = cmdLine.getOptionValue("o");
      }
      if (StringUtils.isBlank(this.outputPath)) {
        LOGGER.error("Should specify output directory for command " + command);
        System.err.println("Should specify output directory for command " + command);
        return false;
      }
    }
    return true;
  }

  private void process() {
    if (command.equalsIgnoreCase("tpch")) {
      for (int i = 0; i < inputFiles.length; i++) {
        try {
          TpchExecutor tpchExecutor = new TpchExecutor();
          tpchExecutor.setInputFiles(inputFiles[0]);
          tpchExecutor.ignite(); // tpch = gen_data + exec_sql
          tpchExecutor.close();
        } catch (Exception e) {
          LOGGER.error("Failed to execute tpch for file " + inputFiles[i] + ", will skip it", e);
        }
      }
    } else if (command.equalsIgnoreCase("gen_data")) {
      for (int i = 0; i < inputFiles.length; i++) {
        try {
          DataGenerator dataGenerator = new DataGenerator();
          dataGenerator.setInputFiles(inputFiles[i]);
          dataGenerator.ignite();
        } catch (IOException e) {
          LOGGER.error("Failed to generate data for file " + inputFiles[i] + ", will skip it", e);
        }
      }
    } else if (command.equalsIgnoreCase("exec_sql")) {
      for (int i = 0; i < inputFiles.length; i++) {
        try {
          QueryClient queryClient = new QueryClient();
          queryClient.setInputFiles(inputFiles[i]);
          queryClient.ignite();
          queryClient.close();
        } catch (Exception e) {
          LOGGER.error("Failed to execute query for file " + inputFiles[i] + ", will skip it", e);
        }
      }
    } else if (command.equalsIgnoreCase("merge_report")) {
      String mergedRpt = HistogramReporter.mergeStatisticFromFile(outputPath, inputFiles);
      LOGGER.info(mergedRpt);
    } else {
      LOGGER.error("Unsupported command " + command);
      System.err.println("Unsupported command " + command);
      printHelp();
    }
  }

  private void printHelp() {
    HelpFormatter helpFormatter = new HelpFormatter();
    helpFormatter.printHelp("java -cp 'tpch-java.jar:jdbc_driver/*' ind.xuchuanyin.tpch.CliTool ", options);
  }

  public static void main(String[] args) {
    CliTool cliTool = new CliTool();
    if (cliTool.parseCmdLine(args)) {
      cliTool.process();
    }
  }
}
