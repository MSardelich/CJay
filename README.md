CJay -- Java&trade; Native Interface made easy
----------------------------------------------

Seamlessly call Java classes (here the "*jay*") from C++. The ``Cjay`` C++ library abstracts the use of Java&trade; Native Interface (JNI).

``CJay`` comes with a ``Converter`` class that straightforwardly **cast** from *primitive wrapper classes* and some ``java.util`` classes to some C++ STL objects and vice versa.

The cast is performed using only **one member function call**. See, for example, ``j_cast<T>``, ``c_cast<T>``, ``c_cast_vector<T>``, ``c_cast_map<K,V>``.

Why?
----

* Although ``JNI`` is a mature library, its method caller entry points depend on the method description/signature i.e ``CallStaticVoidMethod``, ``CallVoidMethod``, ``CallObjectMethod``, and many others.
  On the other hand, ``CJay`` has **only one call method** (``CJ::call<T>``) for all types of description/signature.
* ``CJay`` obtains [reflective information] (http://en.wikipedia.org/wiki/Reflection_(computer_programming)) about Java classes and objects at **run-time**. It automatically disassembly Java classes and extract method names and descriptors. **Forget about all messing descriptor strings!**
* ``CJay`` comes with a **conversion class** (``Convert``) that straightforwardly **cast types** from C++ to Java and **vice versa**. The conversion class can, for exmaple, convert from Java ``Arraylist<T>`` to C++ ``Vector<T>``. See ``CJ::c_cast_vector<T>`` and ``CJ::c_cast<T>`` for general primitive types.
* Transparent interface **method caching**. Register your Java methods only once, use them around the code.
* You can still **use** functions in ``jni.h``. Just get the Java&trade; Virtual Machine enviroment pointer: ``VM::env``.
* Only **one header file**: ``CJay.hpp``
* **Exception handler** with clear and informative error messages.

Life Made Easy!
----------------

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
    
    // Create JVM
    std::vector<std::string> paramVM{"-ea", "-Xdebug"};
    VM::createVM(paramVM);
    
    // Set Java class
    CJ.setClass("example/Example");
    
    // Call Java class constructor
    CJ.Constructor("<init>"); // Constructors have <init> signature
    
    // Instantiate a converter
    // It allows to cast from Java Virtual Machine to C++ and vice versa
    Converter cnv;
    
    // Main Routine:
    // 1) Cast from C++ to Java types
    // 2) Call Java method (with variables just cast)
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
    
    // call Java method
    L = CJ.call<jobject>( "parseArrayListInteger", arg_i_1, arg_i_2 ); 
    
    // cast FROM Java "ArrayList<Integer>" TO "vector<long>"
    // the caster works like magic. ONE LINE OF CODE!
    std::vector<jint> v = cnv.c_cast_vector<jint>(L); 
    
    // Destroy JVM
    VM::destroyVM();
}    
```

General Implementation Steps
----------------------------

A standard implementation should follow the steps below.

* Add the path to Java class you want to instantiate to your ``CLASSPATH`` system enviroment variable.
* Include library **header file**: ``CJay.hpp``.
* **Assign ``JNI_VERSION``** static variable (it must be compatible with the versions described in your ``<jni.h>``).
* **Set** the Java **class**.
* **Call** Java class **constructor** (if you would call non-static methods).
* **Call** Java **method**.
  
  *Here another example...*
  
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

**Make sure your `CLASSPATH` system enviroment variable includes path to your local copy of ``java/bin`` repository folder and to java class you want to call from C++.**

``CJay`` library was extensevely tested with the configuration: ``g++ (GCC) 4.8.1`` and `Java(TM) SE Runtime Environment 1.8`

Unit tests
----------

The `java/bin` folder has sub-folder `example` with the class library `Example.class`. Its source code can be found in `java/src` folder.

The source code exaustevely covers many methods with different signatures. Maybe it is the best way to review the seamless integration of ``CJay`` C++ library.

Compile and run ``unittest.cpp``.

Important Note
--------------

You can always check the signatures obtained from your java class by calling member ``printSignatures``.

When Java methods are overloaded they have the same ``name`` with different ``signatures``. In this case, we still have to uniquelly associate a ``key`` to overloaded method, since member ``call<T>`` receives method name.

By convention we decided to add the ``_`` symbol at end of method name together with a number, for each overloaded method.

For example, consider we have a Java class with 2 constructors:

```java
class X {
  X(Object o) { this.o = o} // constructor receives java.lang.Object
  X(int i) { this.i = i} // constructor receives primitive int
  ... 
}   
```

**ONLY IN THE OVERLOADED METHODS CASE**, you have to get unique method ``key`` using member ``getUniqueKey`` and passing the ``name`` and ``descriptor`` of method.

According to the exmaple above, you can check the key assigned to construtor that receives ``java.lang.Obejct`` using the code below:

```cpp
// Output key of constructor that receives java.lang.Object
std::cout << CJ.getUniqueKey("<init>", "(Ljava/lang/Object;)V") << std::endl;   
```

The above line of code is exepect to output ``<init>_1`` or ``<init>_2``.

In order to **call the expected overloaded method at run-time** you shoud code something like this:

```cpp
// Call constructor that receives int
CJ.call<void>(CJ.getUniqueKey("<init>", "(I)V"));
...
```

TODO
----

* ~~Improve ``Converter`` class, including, for example, a caster from ``java.util.Map<T>`` to C++ ``Map<T>``~~
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
