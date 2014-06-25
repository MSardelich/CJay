/*
 * Test.cpp
 *
 *  Created on: Jun 23, 2014
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

jint VM::Handler::JNI_VERSION = JNI_VERSION_1_8; // Assign static member. Depends on doenloaded JDK

int main (int argc, char* argv[]) {
    VM::Handler handler;

    // Set path to java classes
    string jarsPath(getenv("CLASSPATH")); // Get CLASSPATH system environment variable
    string paramPath = string("-Djava.class.path=") + jarsPath;

    vector<string> vmOption;
    vmOption.push_back(paramPath);
    vmOption.push_back("-Xcheck:jni"); // "-Xnoclassgc"
    vmOption.push_back("-ea"); // enable java assertion

    // Create JVM
    try {
        handler.createVM(vmOption); // creates JVM
    }
    catch(exception& e) {
        cout << e.what() << endl;
        return EXIT_FAILURE;
    }

    // Set member signatures
    handler.setSignature( string("<init>"), string("()V"), false );
    handler.setSignature( string("parseBoolean"), string("(Z)Z"), false );
    handler.setSignature( string("parseByte"), string("(B)B"), false );
    handler.setSignature( string("parseChar"), string("(C)C"), false );
    handler.setSignature( string("parseShort"), string("(S)S"), false );
    handler.setSignature( string("parseInt"), string("(I)I"), false );
    handler.setSignature( string("parseLong"), string("(J)J"), false );
    handler.setSignature( string("parseFloat"), string("(F)F"), false );
    handler.setSignature( string("parseDouble"), string("(D)D"), false );
    handler.setSignature( string("parseString"), string("(Ljava/lang/String;)Ljava/lang/String;"), true );
    handler.setSignature( string("parseArray"), string("(II)Ljava/util/ArrayList;"), true );
    handler.setSignature( string("parseSimpleMap"), string("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Map;"), true );
    handler.setSignature( string("parseMap"), string("([Ljava/lang/String;[I)Ljava/util/Map;"), false );

    // Print signatures
    handler.printSignatures();

    // Instantiate converter
    VM::Converter cnv;

    // Set class
    string className = "example/Example";
    try {
        handler.setClass(className);
    } catch(exception& e) {
        cout << e.what() << endl;
        handler.destroyVM();
        return EXIT_FAILURE;
    }

    // Call constructor
    try {
        handler.callClassConstructor(NULL);
    } catch(exception& e) {
        cout << e.what() << endl;
        handler.destroyVM();
        return EXIT_FAILURE;
    }

    // Assertions
    jobject jobj; _jobject
    try {
        jobj = handler.callMethod( "parseBoolean", cnv.j_cast<jboolean>(false) );
        assert ((bool) jobj == false);
        jobj = handler.callMethod( "parseByte", cnv.j_cast<jbyte>(123) );
        assert ((jint) jobj == 123);
        jobj = handler.callMethod( "parseChar", cnv.j_cast<jchar>('a') ); // single 16-bit unicode
        assert ((jint) jobj == (__int64) 'a');
        jobj = handler.callMethod( "parseShort", cnv.j_cast<jshort>((short) -123) );
        assert ((jint) jobj == -123); // coversion to int avoids lost precision
        jobj = handler.callMethod("parseInt", cnv.j_cast<jint>(123) );
        assert ((jint) jobj == 123);
        jobj = handler.callMethod( "parseLong", cnv.j_cast<jlong>((long long) 123) );
        assert ((jlong) jobj == 123);
        jobj = handler.callMethod( "parseFloat", cnv.j_cast<jfloat>((float) 123.0) );
        jfloat test_conv = (jfloat) (jobj);

        /*
        Test jfloat directly
        JNIEnv* env = handler.getEnv();
        jmethodID mid = handler.getMid("parseFloat");
        jobject jobj = handler.getObj();
        jfloat res = env->CallFloatMethod( jobj, mid, cnv.j_cast<jfloat>((float) 123.0) );
        */

        //assert ((float) jobj == 123.0);
        jobj = handler.callMethod( "parseDouble", cnv.j_cast<jdouble>((double) 123.0) );
        //assert ((double) jobj == (double) 123.0);
        jobj = handler.callMethod( "parseString", cnv.j_cast<jstring>("CJay is cool!") );
        //assert (())
        jobj = handler.callMethod( "parseArray", cnv.j_cast<jint>(123), cnv.j_cast<jint>(456) );
        VM::vec_jobj v_jobj = cnv.toVec<jobject>(jobj);
        vector<int> v = cnv.toVec<int>(jobj);
    } catch(exception& e) {
        cout << e.what() << endl;
        handler.destroyVM();
        return EXIT_FAILURE;
    }

    handler.destroyVM();

    return EXIT_SUCCESS;
}
