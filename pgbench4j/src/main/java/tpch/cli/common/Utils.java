package tpch.cli.common;

import java.util.Random;

public class Utils {
  public static boolean closeQuietly(AutoCloseable conn) {
    boolean flag = true;
    try {
      if (null != conn) {
        conn.close();
      }
    } catch (Exception e) {
      flag = false;
    }
    return flag;
  }

  public static <T> void shuffleArray(T[] arr) {
    int i = arr.length;
    int j;
    T temp;

    if (i == 0) {
      return;
    }

    while (--i > 0) {
      int random = new Random().nextInt();
      random = random >= 0 ? random : -1 * random;
      j = random % (i + 1);
      temp = arr[i];
      arr[i] = arr[j];
      arr[j] = temp;
    }
  }
}
