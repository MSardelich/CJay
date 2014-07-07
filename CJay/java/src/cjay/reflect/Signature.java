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
package cjay.reflect;

import java.lang.reflect.*;
import java.util.*;

public class Signature {
  private static java.util.HashMap<String, String> primitives;

  static
  {
      primitives = new java.util.HashMap<String, String>();
      primitives.put("boolean", "Z");
      primitives.put("char", "C");
      primitives.put("byte", "B");
      primitives.put("int", "I");
      primitives.put("short", "S");
      primitives.put("long", "J");
      primitives.put("double", "D");
      primitives.put("float", "F");
      primitives.put("void", "V");
  }

  @SuppressWarnings("rawtypes")
  private Class clazz;
  private String clazzName;
  
  private Method methods[];
  @SuppressWarnings("rawtypes")
  private Constructor constructors[];
  
  private int nMethods;
  private int nConstructors;
  
  @SuppressWarnings("rawtypes")
  Signature(Class clazz) {
    this.clazz = clazz;
    this.clazzName = clazz.getName();
    
    this.methods = this.clazz.getDeclaredMethods();
    this.constructors = this.clazz.getDeclaredConstructors();
    
    this.nMethods = this.methods.length;
    this.nConstructors = this.constructors.length;
  }
  
  Signature(String clazz) {
    try {
      this.clazz = Class.forName(clazz);
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    }
    this.clazzName = clazz;
    
    this.methods = this.clazz.getDeclaredMethods();
    this.constructors = this.clazz.getDeclaredConstructors();
    
    this.nMethods = this.methods.length;
    this.nConstructors = this.constructors.length;
  }
  public int getNMethods() {
    return this.nMethods;
  }
  
  public int getNConstructors() {
    return this.nConstructors;
  }
  
  @SuppressWarnings("rawtypes")
  public Class getClazz() {
    return this.clazz;
  }
  
  public String getClazzName() {
    return this.clazzName;
  }
  
  @SuppressWarnings("rawtypes")
  private String convertClassToDescriptor(Class param) {
    if (param.isPrimitive())
      return primitives.get(param.toString());
    else if (param.isArray())
      return "[" + convertClassToDescriptor(param.getComponentType());
    else
      return "L" + (param.getName()).replace('.', '/') + ";";
  }
  
  public ArrayList<String> getMethodNames() {
    ArrayList<String> arrayList = new ArrayList<String>();
    for(Method method : this.methods) {
      arrayList.add(method.getName());
    }
    
    return arrayList;
  }
  
  @SuppressWarnings("rawtypes")
  public ArrayList<String> getConstructorNames() {
    ArrayList<String> arrayList = new ArrayList<String>();
    for(@SuppressWarnings("unused") Constructor constructor : this.constructors) {
      arrayList.add("<init>"); // all constructors have <init> signature
    }
    return arrayList;
  }
  
  public ArrayList<Boolean> getMethodIsStatic() {
    ArrayList<Boolean> arrayList = new ArrayList<Boolean>();
    for(Method method : this.methods) {
      arrayList.add( Modifier.isStatic(method.getModifiers()) );
    }
    
    return arrayList;
  }
  
  @SuppressWarnings("rawtypes")
  public ArrayList<Boolean> getConstructorIsStatic() {
    ArrayList<Boolean> arrayList = new ArrayList<Boolean>();
    for(Constructor constructor : this.constructors) {
      arrayList.add( Modifier.isStatic(constructor.getModifiers()) );
    }
    
    return arrayList;
  }
  
  @SuppressWarnings("rawtypes")
  public ArrayList<String> getMethodDescriptors() {
    ArrayList<String> arrayList = new ArrayList<String>();
    StringBuffer methodDescriptor;
    Class params[];
    Class rtnValue;
    for(Method method : this.methods) {
      // New method descriptor
      methodDescriptor = new StringBuffer();
      
      // Append method parameters types to descriptor
      params = method.getParameterTypes();
      methodDescriptor.append("(");
      for(Class param : params) {
        String descriptor = this.convertClassToDescriptor(param);
        methodDescriptor.append( (descriptor == "V") ? "" : descriptor );
      }
      methodDescriptor.append(")");
      
      // Append method return value type to descriptor
      rtnValue = method.getReturnType();
      methodDescriptor.append( this.convertClassToDescriptor(rtnValue) );
      
      // Add descriptor to current method
      arrayList.add(methodDescriptor.toString());
    }
    
    return arrayList;
  }
  
  @SuppressWarnings("rawtypes")
  public ArrayList<String> getConstructorDescriptors() {
    ArrayList<String> arrayList = new ArrayList<String>();
    StringBuffer methodDescriptor;
    Class params[];
    for(Constructor constructor : this.constructors) {
      // New method descriptor
      methodDescriptor = new StringBuffer();
      
      // Append method parameters types to descriptor
      params = constructor.getParameterTypes();
      methodDescriptor.append("(");
      for(Class param : params) {
        String descriptor = this.convertClassToDescriptor(param);
        methodDescriptor.append( (descriptor == "V") ? "" : descriptor );
      }
      methodDescriptor.append(")");
      
      // All constructors return void
      methodDescriptor.append("V");
      
      // Add descriptor to current method
      arrayList.add(methodDescriptor.toString());
    }
    
    return arrayList;
  }
  
  public ArrayList<String> getAllMembersNames() {
    ArrayList<String> arrayList = new ArrayList<String>();
    arrayList.addAll(this.getMethodNames());
    arrayList.addAll(this.getConstructorNames());
    
    return arrayList;
  }
  
  public ArrayList<Boolean> getAllMembersIsStatic() {
    ArrayList<Boolean> arrayList = new ArrayList<Boolean>();
    arrayList.addAll(this.getMethodIsStatic());
    arrayList.addAll(this.getConstructorIsStatic());
    
    return arrayList;
  }
  
  public ArrayList<String> getAllMembersDescriptors() {
    ArrayList<String> arrayList = new ArrayList<String>();
    arrayList.addAll(this.getMethodDescriptors());
    arrayList.addAll(this.getConstructorDescriptors());
    
    return arrayList;
  }
  
  @SuppressWarnings("rawtypes")
  public static void main(String[] args) {
    Class clazz = null;
    try {
      clazz = Class.forName(args[0]); // example.Example
    } catch(ClassNotFoundException e) {
      e.printStackTrace();
      System.exit(1);
    }
    Signature signature = new Signature(clazz);
    
    ArrayList<String> methodNames = signature.getMethodNames();
    ArrayList<Boolean> methodIsStatic = signature.getMethodIsStatic();
    ArrayList<String> methodDescriptors = signature.getMethodDescriptors();
    
    ArrayList<String> constructorNames = signature.getConstructorNames();
    ArrayList<Boolean> constructorIsStatic = signature.getConstructorIsStatic();
    ArrayList<String> constructorDescriptors = signature.getConstructorDescriptors();
    
    // Print class methods
    for(int i = 0; i < signature.getNMethods() ; i++) {
      System.out.println(
          "<Method: " + methodNames.get(i) + 
          ", Descriptor: " + methodDescriptors.get(i) +
          ", isStatic: " + methodIsStatic.get(i) +
          ">");
    }
    System.out.println();
    // Print class constrcutors
    for(int i = 0; i < signature.getNConstructors() ; i++) {
      System.out.println(
          "<Constructor: " + constructorNames.get(i) + 
          ", Descriptor: " + constructorDescriptors.get(i) +
          ", isStatic: " + constructorIsStatic.get(i) +
          ">");
    }
    System.exit(0);
  }
}
