/***************************************************************************
 * Copyright 2014 Marcelo Sardelich <MSardelich@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/
package cjay.converter;

import java.util.*;

public class Util {
  // Cast Object to Primitive Wrapper classes
  static Byte FromObjectToByte(Object o) {
    return (Byte) o;
  }
  
  static Short FromObjectToShort(Object o) {
    return (Short) o;
  }
  
  static Integer FromObjectToInteger(Object o) {
    return (Integer) o;
  }
  
  static Long FromObjectToLong(Object o) {
    return (Long) o;
  }
  
  static Float FromObjectToFloat(Object o) {
    return (Float) o;
  }
  
  static Double FromObjectToDouble(Object o) {
    return (Double) o;
  }
  
  static Character FromObjectToCharacter(Object o) {
    return (Character) o;
  }
  
  static Boolean FromObjectToBoolean(Object o) {
    return (Boolean) o;
  }
  
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
