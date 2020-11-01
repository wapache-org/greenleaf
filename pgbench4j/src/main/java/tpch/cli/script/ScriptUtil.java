package tpch.cli.script;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.regex.Pattern;

import org.apache.commons.io.input.BoundedInputStream;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ScriptUtil {
  private static final Logger LOGGER = LoggerFactory.getLogger(ScriptUtil.class);

  public static boolean runBashFile(File file) {
    String cmd = "sh " + file.getAbsolutePath();
    Process execProcess = null;
    StreamGobbler errorGobbler;
    StreamGobbler outputGobbler;

    int execValue;
    try {
      execProcess = getExecRuntimeProcess(cmd);
      if (null == execProcess) {
        LOGGER.error("Failed to execute command " + cmd);
        return false;
      }

      // Any error & output message
      errorGobbler = new StreamGobbler(execProcess.getErrorStream());
      outputGobbler = new StreamGobbler(execProcess.getInputStream());

      new Thread(errorGobbler).start();
      new Thread(outputGobbler).start();

      execValue = execProcess.waitFor();
    } catch (InterruptedException e1) {
      LOGGER.error("Failed to execute command " + cmd, e1);
      return false;
    } finally {
      if (null != execProcess) {
        execProcess.destroy();
      }
    }

    if (execValue != 0) {
      LOGGER.error(
          "Failed to execute command " + cmd + " detailed reason is " + errorGobbler.getResult());
      return false;
    } else {
      LOGGER.info("Succeed to run command " + cmd);
      return true;
    }

  }

  private static Process getExecRuntimeProcess(String cmdStr) {
    if (null == cmdStr) {
      return null;
    }

    cmdStr = cmdStr.replaceAll("\\\\", "/");
    Pattern splitPattern = Pattern.compile("[ ]+");
    String[] cmdArr = splitPattern.split(cmdStr);
    if (!validateCommand(cmdArr)) {
      return null;
    }

    // execute the cmd string
    Runtime rt = Runtime.getRuntime();
    if (null == rt) {
      return null;
    }
    Process rs = null;
    try {
      rs = rt.exec(cmdArr);
    } catch (IOException e) {
      return null;
    }
    return rs;
  }

  private static boolean validateCommand(final String[] cmdArray) {
    boolean result = true;
    if (cmdArray != null) {
      for (int i = 0; i < cmdArray.length; i++) {
        if (cmdArray[i] == null || !cmdArray[i]
            .matches("[\\\\/:0-9a-zA-Z\\s\\-_\\.@&*\\${}\";,()=$]+")) {
          result = false;
          break;
        }
      }
    } else {
      result = false;
    }

    return result;
  }
}

class StreamGobbler implements Runnable {

  private static final Logger LOGGER = LoggerFactory.getLogger(StreamGobbler.class);

  private InputStream is;

  private String result;

  private final int max_stream_length = 10240;

  StreamGobbler(InputStream is) {
    this.is = is;
  }

  public String getResult() {
    return result;
  }

  public void run() {
    BoundedInputStream bis = null;
    InputStreamReader isr = null;
    BufferedReader br = null;
    try {
      bis = new BoundedInputStream(is, max_stream_length);
      isr = new InputStreamReader(bis, "UTF-8");
      br = new BufferedReader(isr, 2048);
      StringBuilder output = new StringBuilder();
      String line;
      while ((line = br.readLine()) != null) {
        output.append(line);
        output.append(System.lineSeparator());
      }
      result = output.toString();
      LOGGER.info(result);
    } catch (IOException e) {
      result = e.getMessage();
    } finally {
      try {
        if (null != bis) {
          bis.close();
        }
        if (null != isr) {
          isr.close();
        }
        if (null != br) {
          br.close();
        }
      } catch (IOException e) {
        LOGGER.warn("error occurs while closing reader", e);
      }
    }
  }
}
