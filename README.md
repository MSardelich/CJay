CJay -- Java&trade; Native Interface made easy
----------------------------------------------

Seamlessly call Java classes (here the "*jay*") from C++. The ``Cjay`` C++ library abstracts the use of Java&trade; Native Interface.

Why?
----

* Although ``JNI`` is a mature library, its method caller entry points depend on the method description/signature i.e ``CallStaticVoidMethod``, ``CallVoidMethod``, ``CallObjectMethod``, and many others.
  On the other hand, ``CJay`` has **only one call method** (``CJ::call<T>``) for all types of description/signature.
* ``CJay`` comes with a **conversion class** (``Convert``) that straightforwardly **cast types** from C++ to Java and **vice versa**. The conversion class can, for exmaple, convert from Java ``Arraylist<T>`` to C++ ``Vector<T>``. See ``CJ::c_cast_vector<T>`` and ``CJ::c_cast<T>`` for general primitive types.
* Transparent interface **method caching**. Register your Java methods only once, use them around the code.
* ``CJay`` emulates **reflection** using standard C++ Maps (you can instantiate Java classes and invoke methods by string name).   
* You can still **use** functions in ``jni.h``. Just get the Java Virtual Machine enviroment pointer: ``VM::env``.
* Only **one header file**: ``CJay.hpp``
* **Exception handler** with clear and informative error messages.

Life Made Easy!
---------------

For illustration purposes, suppose you have the following ``Example.java`` class, which you want to call from your C++ code:

```java
package example;
import java.util.*;

public class Example {
  
  // Construtor
  public Example() { }
  
  // ArrayList<Integer> method
  static ArrayList<Integer> parseArrayListInteger(int x, int y) {
    ArrayList<Integer> result = new ArrayList<Integer>();
    result.add(x);
    result.add(y);
    return result;
  }
  
  // Main method
  public static void main(String[] args) { }
  
}
```

The code below shows a typical C++ ``Cjay`` library implementation:

```cpp
#include <string>
#include <vector>

#include "CJay.hpp"

using namespace VM;

// Define your JDK version
jint CJ::JNI_VERSION = JNI_VERSION_1_8; 

int main (int argc, char* argv[]) {
    // Instantiate CJay
    CJ CJ;
    
    // Get Java CLASSPATH environment variable
    std::string paramPath = std::string("-Djava.class.path=") + std::string(getenv("CLASSPATH"));
    
    // Set vector with all JVM options
    std::vector<std::string> vmOption(paramPath);
    
    // Create JVM
    CJ.createVM(vmOption);
    
    // Register Java methods
    //
    // Important:
    // Run the command "$ javap -s -p emxample.class" to get all the information you need.
    // "setSignature" method has the arguments:
    // (<method_name>, <method_description>, <method_is_static>).
    //
    CJ.setSignature( "parseArrayListInteger", "(II)Ljava/util/ArrayList;", true );
    CJ.setSignature( "<init>", "()V", false ); // class constructor always use <init> signature
    
    // Instantiate a converter
    // It allows to cast from Java Virtual Machine to C++ and vice versa
    Converter cnv;
    
    // Set Java class
    CJ.setClass("example/Example");
    
    // Call Java class constructor
    CJ.callClassConstructor(NULL); // this constructor does not require any argument, so NULL.
    
    // Main Routine:
    // 1) Cast from C++ to Java types
    // 2) Call Java method
    // 3) Cast back from Java to C++
    // Important:
    // The "call" member function is templated based on
    // the return value of Java method 
    //
    // -- Java Method (ArrayList<Integer>)
    //
    
    // cast FROM C++ "int" TO Java "int"
    jint arg_i_1 = (jint) 123;
    jint arg_i_2 = (jint) 456;
    
    // call Java method (template dependes on returned value)
    L = CJ.call<jobject>( "parseArrayListInteger", arg_i_1, arg_i_2 ); 
    
    // cast FROM Java "ArrayList<Integer>" TO "vector<long>"
    // the caster works like magic. ONE LINE OF CODE!
    std::vector<jint> v = cnv.c_cast_vector<jint>(L, 2); 
    
    // Destroy JVM
    CJ.destroyVM();
}    
```

General Implementation Steps
----------------------------

A standard implementation should follow the steps below.

* Include library **header file**: ``CJay.hpp``.
* **Assign ``JNI_VERSION``** variable (it must be compatible with the versions described in your ``<jni.h>``).
* Define **JVM path**, additional flags and **create JVM**.
* Obtain the **signatures/descriptions** of your Java class:

  ```bash
  $ javap -s -p <your_java_class>.class
  ```

* **Set the signatures** you just obtained calling ``setSignature``.
* **Load/Set** the Java **class**.
* **Call** Java class **constructor** (if you have to call non-static methods).
* **Call** Java **method**
  
  *Here another example.*
  
  Consider we have a Java method ``parseString`` that recieves type ``java.lang.String`` and returns ``java.lang.String``.
  
  **IMPORATNT:** *See we only have one ``call<T>`` entry point, regardless the method descriptor. It is a variadic member. The member function ``call<T>`` is temaplated based on the method return type.*
  
  ```cpp
  // cast FROM C++ "string" TO Java "java.lang.String"
  jstring arg = cnv.j_cast<jstring>("foo");
  // call Java method
  jobject L = CJ.call<jobject>( "parseString", cnv.j_cast<jstring>("foo") ); 
  // cast FROM "java.lang.String" TO C++ "string"
  std::string str = cnv.c_cast<std::string>(L);
  ```

* **Destroy** JVM when you are done

Compiler, Linker and System Variables
-------------------------------------

First of all, you need to install [Oracle Java&trade; Development Kit (JDK)] (http://www.oracle.com/technetwork/java/javase/downloads/index.html?ssSourceSiteId=ocomen>).

Make sure compiler toolchain includes (-I option) your [Java&trade; Development Kit (JDK)] (http://www.oracle.com/technetwork/java/javase/downloads/index.html?ssSourceSiteId=ocomen>) ``include`` sub-folder . On window you MUST include ``include\win32`` sub-folder too.

You must link (-L option) ``jvm`` file in [Java&trade; Development Kit (JDK)] (http://www.oracle.com/technetwork/java/javase/downloads/index.html?ssSourceSiteId=ocomen>) ``lib`` folder, and set the system ``path`` to this folder.

``CJay`` is **C++11** compatible, so add ``-std=c++11`` flag to compiler.

Make sure your `CLASSPATH` system enviroment variable includes path to your local copy of ``java/bin`` repository folder and to java class you want to call from C++.

``CJay`` library was extensevely tested with the configuration: ``g++ (GCC) 4.8.1`` and `Java(TM) SE Runtime Environment 1.8`

Unit tests
----------

The `java/bin` folder has sub-folder `example` with the class library `Example.class`. Its source code can be found in `java/src` folder.

The source code exaustevely covers many methods with different signatures. Maybe it is the best way to review the seamless integration of ``CJay`` C++ library.

Compile and run ``UnitTest.cpp``.

TODO
----

* Improve ``Converter`` class, including, for example, a caster from ``java.util.Map<T>`` to C++ ``Map<T>``
* Add methods to main ``CJ`` class in order to acess Java class *fields*.
* Write documentation.

Questions?
----------

Please hit me up at MSardelich@gmail.com

Want to Help?
-------------

Request pulls! An open source project is expected to be built using thousand hands...

License
-------

``CJay`` is licensed under [Apache Version 2.0] (http://www.apache.org/licenses/>).

Copyright (c) 2014, Marcelo Sardelich <MSardelich@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
