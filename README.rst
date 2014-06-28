CJay
====

``Cjay`` is a C++ class abstraction to Java Native Interface (``JNI``). It seamlessly integrates Java class libraries (here the "jay") into C++.

Why?
----

- Although ``JNI`` is a mature library its method caller entry points depend on the method description/signature i.e ``CallStaticVoidMethod``, ``CallVoidMethod``, ``CallObjectMethod`` and others. ``Cjay``, on the other hand, has a ``Conevrsion`` class with only one entry point, the ``call`` member function.
- ``CJay`` has a conversion class to convert from C++ to java and *vice versa*.
- The conversion class can for exmaple convert from Java ``Arraylist<T>`` class to C++ ``Vector<T>`` class directly, see templated ``c_cast`` member function.
- Regiter your Java methods only once, use them around the code.
- You can still call native ``JNI`` functions. Just get the enviroment pointer ``CJ::env``.
- Only one header file: ``Cjay.hpp``
- An exception handler with clear and informative error messages.

Compiler, Linker and System Variables
-------------------------------------

First of all, you need to install Oracle `Java Development Kit (JDK) <http://www.oracle.com/technetwork/java/javase/downloads/index.html?ssSourceSiteId=ocomen>`_.

Make sure compiler toolchain includes (-I option) the JDK ``include`` sub-folder . On window you MUST include ``include\win32`` sub-folder too.

You must link (-L option) ``jvm`` file within `Java Development Kit (JDK) <http://www.oracle.com/technetwork/java/javase/downloads/index.html?ssSourceSiteId=ocomen>`_ ``lib`` folder, and set the system path to this folder.

``Cjay`` is **c++11** compatible, so add ``-std=c++11`` flag.

In case you want to run the unit tests (``UnitTest.cpp``) make sure system enviroment ``CLASSPATH`` variable includes path to ``java/bin`` folder. Any java class that you want to instantiate MUST follow the same procedure

Implementation (Starting Out)
-----------------------------

    A standard implementation should follow the steps below.

- Include library header file (only one):

.. code-block:: cpp

    #include "Cjay.hpp"

- Assign ``JNI_VERSION`` variable (it must be compatible with the versions described in your ``<jni.h>``). For example (version 1.8):

.. code-block:: cpp

    jint VM::CJ::JNI_VERSION = JNI_VERSION_1_8;
    
- Define Java Virtual Machine path and other options and create it:

.. code-block:: cpp
    
    using namespace VM;
    
    jint CJ::JNI_VERSION = JNI_VERSION_1_8; // Depends on installed JDK!
    
    int main (int argc, char* argv[]) {
        CJ CJ;
        
        // Set path to java classes
        std::string jarsPath(getenv("CLASSPATH")); // Get CLASSPATH environment variable
        std::string paramPath = std::string("-Djava.class.path=") + jarsPath;
        
        std::vector<std::string> vmOption;
        vmOption.push_back(paramPath); // add path to class
        vmOption.push_back("-Xcheck:jni"); // debug mode
        vmOption.push_back("-ea"); // enable java assertion
        
        // creates JVM
        CJ.createVM(vmOption);
    
    }


- Obtain the signatures/descriptions of your java class:

.. code-block:: bash

    $ javap -s -p <*your_java_class*>.class

- Set the signatures you just obtained:

    The setSignature member function has the parameters:
    
    - key (**string**). *The name of the java method.*
    
    - descriptor (**string**). *The descriptor of the java method.*
    
    - isStatic  (**boll**). *True if the method is static.*

.. code-block:: cpp
    
    ...
    
    handler.setSignature( string("<init>"), string("<constructor_descriptor>"), false ); // <init> MUST be the name of the class constructor 
    handler.setSignature( string("<merthod_name>"), string("<merthod_descriptor>"), false ); // add each method you want to call
    
    ...

- Set the java class:

.. code-block:: cpp
    
    ...
    
    string className ("<your_class_name>");
    CJ.setClass(className);
    
    ...
    
- Call java class constructor:

    In the example below we consider a class constructor that recieves a Java ``string`` as argument.
    In order to create a Java ``string`` (``java.lang.String``) we need to instantiate a ``conveter``.

.. code-block:: cpp
    ...
    
    // Instantiate converter
    Converter cnv;
    
    // Call constructor
    CJ.callClassConstructor(NULL); // In this example the constructor has no argument.
    
    ...

- Call java method:
  
    In the example below we consider a java method that recieves ``java.lang.String`` argument and returns ``java.lang.String``.
    
    **IMPORATNT:** We have only one entry point, regardless the method descriptor, and ``call`` is a variadic member. It is temaplted based on the method return value.

.. code-block:: cpp

    ...
    jobject L = CJ.call<jobject>( "parseString", cnv.j_cast<jstring>("foo") ); // Call java method. Cast FROM C++ string TO java.lang.String (j_cast)
    std::string str = cnv.c_cast<std::string>(L); // Now, cast back: FROM java.lang.String TO C++ string (c_cast)
    assert ( str == std::string("foo") );
    ...

- Destroy JVM when your are done

.. code-block:: cpp

    CJ.destroyVM();

UNITTEST
--------

Run UnitTest.cpp.

The source code exaustevely cover many methods with differente signatures. Maybe it is the best way to understand the seamless intration of ``CJay`` library. 

TODO
----

- Improve ``Converter`` class, inclusing, for example a caster from ``java.util.Map<T>`` to C++ ``Map<T>``
- Add methods to main ``CJ`` class in order to acess java class fields.

Questions?
----------

Please hit me up at MSardelich@gmail.com

Want to Help?
-------------

Request pulls! An open source project is expected to be built using thousand hands...

License
-------

Copyright (c) 2014, Marcelo Sardelich

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.