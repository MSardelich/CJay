package example;

import java.util.*;

public class Example {
  
  // Construtor
  public Example() { }
  
  //Parse boolean
  public boolean parseBoolean(boolean x) {
    return x;
  }
  //Parse byte
  public byte parseByte(byte x) {
    return x;
  }
  //Parse char
  public char parseChar(char x) {
    return x;
  }
  //Parse short
  public short parseShort(short x) {
    return x;
  }
  // Parse int
  public int parseInt(int x) {
    return x;
  }
  // Parse long
  public long parseLong(long x) {
    return x;
  }
  // Parse float
  public float parseFloat(float x) {
    return x;
  }
  // Parse double
  public double parseDouble(double x) {
    return x;
  }
  //Parse String
  static String parseString(String x) {
    return x;
  }
  //Parse ArrayList
  static ArrayList<Integer> parseArray(int x, int y) {
    ArrayList<Integer> result = new ArrayList<Integer>();
    result.add(x);
    result.add(y);
    return result;
  }
  //Parse simple Map
  static Map<String, String> parseSimpleMap(String x, String y, String z) {
    Map<String, String> result = new HashMap<String, String>();
    result.put("arg 1", x);
    result.put("arg 2", y);
    result.put("arg 3", z);
    return result;
  }
  // Parse Map
  Map<String, Integer> parseMap(String[] keys, int[] values) {
    assert keys.length == values.length;
    Map<String, Integer> result = new HashMap<String, Integer>();
    for (int i = 0 ; i < keys.length ; i++) {
      result.put(keys[i], values[i]);
    }
    return result;
  }
  
  public static void main(String[] args) { }
  
}
