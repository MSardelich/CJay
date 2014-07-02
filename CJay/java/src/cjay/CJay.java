package cjay;

import java.util.*;

public class CJay {
  
  @SuppressWarnings({ "rawtypes", "unchecked" })
  static ArrayList FromMapToArrayListOfKeys(Map m) {
    Set x = m.keySet();
    Object[] array = x.toArray();
    ArrayList arrayList = new ArrayList(Arrays.asList(array));
    
    return arrayList;
  }
  
  @SuppressWarnings({ "rawtypes", "unchecked" })
  static ArrayList FromMapToArrayListOfValues(Map m) {
    Collection x = m.values();
    Object[] array = x.toArray();
    ArrayList arrayList = new ArrayList(Arrays.asList(array));
    
    return arrayList;
  }
  
  public static void main(String[] args) { }

}
