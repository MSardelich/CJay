/*
 * Test.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: msn
 */

/*
 * StanfordAPI.cpp
 *
 *  Created on: May 23, 2014
 *      Author: msn
 */

#include <iostream>
#include <string>
#include <exception>
#include <stdexcept>
#include <vector>
#include <map>
#include <cassert>

#include "Handler.h"

using namespace std;

jint VM::Handler::JNI_VERSION = JNI_VERSION_1_8;

int main (int argc, char* argv[]) {
    VM::Handler handler;

    // Set path to java classes
    string jarsPath(getenv("CLASSPATH")); // get CLASSPATH system environment variable
    string paramPath = string("-Djava.class.path=") + jarsPath;

    vector<string> vmOption;
    vmOption.push_back(paramPath);
    vmOption.push_back("-Xcheck:jni"); // "-Xnoclassgc"
    vmOption.push_back("-ea"); // enable java assertion

    try {
        handler.createVM(vmOption); // creates JVM
    }
    catch(exception& e) {
        cout << e.what() << endl;
        return EXIT_FAILURE;
    }

    handler.setSignature( string("<init>"), string("()V"), false );
    handler.setSignature( string("parseBoolean"), string("(Z)Z"), false );
    handler.setSignature( string("parseChar"), string("(C)C"), false );
    handler.setSignature( string("parseInt"), string("(I)I"), false );
    handler.setSignature( string("parseLong"), string("(J)J"), false );
    handler.setSignature( string("parseFloat"), string("(F)F"), false );
    handler.setSignature( string("parseDouble"), string("(D)D"), false );
    handler.setSignature( string("parseString"), string("(Ljava/lang/String;)Ljava/lang/String;"), true );
    handler.setSignature( string("parseArray"), string("(II)Ljava/util/ArrayList;"), true );
    handler.setSignature( string("parseSimpleMap"), string("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Map;"), true );
    handler.setSignature( string("parseMap"), string("([Ljava/lang/String;[I)Ljava/util/Map;"), false );

    handler.printSignatures();

    VM::Converter converter;

    // Set class
    string className = "Example";
    try {
        handler.setClass(className);
    } catch(exception& e) {
        cout << e.what() << endl;
        handler.destroyVM();
        return EXIT_FAILURE;
    }

    // Call constructor
    jobject jobj;
    try {
        handler.callClassConstructor(NULL);
    } catch(exception& e) {
        cout << e.what() << endl;
        handler.destroyVM();
        return EXIT_FAILURE;
    }

    // Call methods
    jobject jobj;
    try {
        jobj = handler.callMethod("parseBoolean", (char) 'A');
        std::assert ((char) (jchar) jobj == 'A');
    } catch(exception& e) {
        cout << e.what() << endl;
        handler.destroyVM();
        return EXIT_FAILURE;
   }
/*
   methodName.assign("getTokens");
   VM::t_vec_obj v_outter, v_inner;
   vector<vector<string>> vv;
   vector<string> v;
   try {
        jobj = handler.callMethod(methodName, NULL); // return jobj == ArrayList<ArrayList<String>>
        v_outter = converter.toVec(jobj);
        vv.clear();
        for (auto elem_outter : v_outter) {
            v_inner = converter.toVec(elem_outter);
            v.clear();
            for (auto elem_inner: v_inner) { v.push_back(converter.toString(elem_inner)); }
            cout << v << endl;
            vv.push_back(v);
        }
    } catch(exception& e) {
        cout << e.what() << endl;
        handler.destroyVM();
        return EXIT_FAILURE;
    }
*/
    return EXIT_SUCCESS;
}



