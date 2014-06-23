CJay
====

``Cjay`` is a C++ class abstraction to Java Native Interface (``JNI``). It seamlessly integrates Java class libraries (here the "jay") into C++.

Why?
----

- Although ``JNI`` is a mature library its method caller entry points depend on the method description/signatute i.e ``CallStaticVoidMethod``, ``CallVoidMethod``, ``CallObjectMethod`` and others. ``Cjay``, on the other hand, has a ``Conevrsion`` class with only one entry point, the ``CallMethod`` member function.
- ``CJay`` has a conversion class to convert from C++ to java and *vice versa*.
- The conversion class can for exmaple convert from Java ``Arraylist`` class to C++ ``Vector`` class directly, see ``toVec`` member function.
- Regiter your Java methods only once, use them seamless around the code.
- You can still call native JNI functions. Just get the enviroment pointer and procced with JNI standard code.
- Only one header file: ``Handler.h``
- An exception handler with clear and informative error messages.

Compiler and Linker
-------------------

First of all, you need to install Oracle `Java Development Kit (JDK) <http://www.oracle.com/technetwork/java/javase/downloads/index.html?ssSourceSiteId=ocomen>`_.

Make sure compiler toolchain includes (-I option) the JDK ``include`` sub-folder. On window you MUST include ``include\win32`` sub-folder too.

You must link (-L option) ``jvm`` file within `Java Development Kit (JDK) <http://www.oracle.com/technetwork/java/javase/downloads/index.html?ssSourceSiteId=ocomen>`_ ``lib`` folder, and set the system path to this folder.

``Cjay`` is **c++11** compatible, so add ``-std=c++11`` flag.

Implementation (Starting Out)
-----------------------------

    A standard implementation should follow the steps below.

- Include library header file (only one):

.. code-block:: cpp

    #include "Handler.h"

- Assign ``JNI_VERSION`` variable (it must be compatible with the versions described in your ``<jni.h>``). For example (version 1.8):

.. code-block:: cpp

    jint VM::Handler::JNI_VERSION = JNI_VERSION_1_8;
    
- Define Java Virtual Machine path and other options and create it:

.. code-block:: cpp
    
    #define FULL_PATH_TO_JAVA_CLASS "<your_java_class>.class"
    
    #if defined(_WIN32) || defined(_WIN64)
    #define PATH_SEPARATOR ";"
    #else
    #define PATH_SEPARATOR ":"
    #endif

    jint VM::Handler::JNI_VERSION = JNI_VERSION_1_8;

    int main (int argc, char* argv[]) {

        string localClassPath(FULL_PATH_TO_JAVA_CLASS); // assign path to java class to be intregrated
        string jarsPath(getenv("CLASSPATH")); // assign Java CLASSAPTH variable
        string path = string("-Djava.class.path=") + jarsPath + string(PATH_SEPARATOR) + localClassPath;
      
        vector<string> vmOption;
        vmOption.push_back(path);
        vmOption.push_back("-Xcheck:jni"); // set JNI debugging. Be careful It drastically impact JVM performance!
        
        VM::Handler handler; // instantiate Handler
        
        handler.createVM(vmOption); // creates JVM
        
      ...

    }


- Obtain the signatures/descriptions of your java class:

.. code-block:: bash

    $ javap -s -p <your_java_class>

- Set the signatures you just obtained:

    The setSignature memebr function has the parameters:
    - key (**string**). *The name of the java method.*
    - descriptor (**string**). *The descriptor of the java method.*
    - isStatic  (**boll**). *True if the method is static.*

.. code-block:: cpp
    
    int main (int argc, char* argv[]) {

        ...
        
        handler.setSignature( string("<init>"), string("<constructor_descriptor>"), false ); // <init> MUST be the name of the class constructor 
        handler.setSignature( string("<merthod_name>"), string("<merthod_name>"), false );
    
        ...
        
    }

- Set the java class:

.. code-block:: cpp
    
    int main (int argc, char* argv[]) {

        ...
        
        string className ("<your_class_name>");
        handler.setClass(className);
        
    }

- Call java class constructor:

    In the example below we consider a class constructor that recieves a Java ``string`` as argument.
    In order to create a Java ``string`` we need to instantiate a ``conveter``.

.. code-block:: cpp
    
    int main (int argc, char* argv[]) {

        ...
        
        string str("<your_method_argument>");
        jobject jobj;
        converter.jString(str, &jobj); // convert C++ string to Java string
        handler.callClassConstructor(jobj); // call constructor
        converter.deleteRef(jobj); // don't forget to delete the java object reference
    
    }

- Call java method:
  
    In the example below you consider a method that recieves an integer argument equal to 1 and is returns void.
    **IMPORATNT:** We have only one entry point regardless the method descriptor and ``callMethod`` is a variadic member. 


.. code-block:: cpp
    
    int main (int argc, char* argv[]) {

        ...
        
        string methodName("<your_method_name>");
        int arg = 1;
        jint jobj;
        
        converter.jInt(arg, &jobj); // convert from C++ int ro Java integer 
        handler.callMethod(methodName, jobject); // method call
        converter.deleteRef((jobject) jobj); // don't forget to delete the java object reference
    }

- Destroy JVM when your are done

.. code-block:: cpp

    handler.destroyVM();

TODO
----

- Add an example using a simple java class
- Improve to the converter members, for example from Java Map to C++ Map
- Add methods to handler class in order to acess java class fields.

Questions?
----------

Please hit me up at MSardelich@gmail.com

Want to Help?
-------------

Request pulls! An open source project is expected to be built using thousand hands...
