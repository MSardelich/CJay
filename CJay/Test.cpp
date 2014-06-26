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

#define MAX_TOLERANCE 1.0e-4

using namespace VM;

jint CJ::JNI_VERSION = JNI_VERSION_1_8; // Depends on installed JDK!

int main (int argc, char* argv[]) {
    CJ CJ;

    // Set path to java classes
    std::string jarsPath(getenv("CLASSPATH")); // Get CLASSPATH system environment variable
    std::string paramPath = std::string("-Djava.class.path=") + jarsPath;

    std::vector<std::string> vmOption;
    vmOption.push_back(paramPath);
    vmOption.push_back("-Xcheck:jni"); // "-Xnoclassgc"
    vmOption.push_back("-ea"); // enable java assertion

    // Create JVM
    try {
        CJ.createVM(vmOption); // creates JVM
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Set member signatures
    CJ.setSignature( std::string("<init>"), std::string("()V"), false );
    CJ.setSignature( std::string("parseBoolean"), std::string("(Z)Z"), false );
    CJ.setSignature( std::string("parseByte"), std::string("(B)B"), false );
    CJ.setSignature( std::string("parseChar"), std::string("(C)C"), false );
    CJ.setSignature( std::string("parseShort"), std::string("(S)S"), false );
    CJ.setSignature( std::string("parseInt"), std::string("(I)I"), false );
    CJ.setSignature( std::string("parseLong"), std::string("(J)J"), false );
    CJ.setSignature( std::string("parseFloat"), std::string("(F)F"), false );
    CJ.setSignature( std::string("parseDouble"), std::string("(D)D"), false );
    CJ.setSignature( std::string("parseString"), std::string("(Ljava/lang/String;)Ljava/lang/String;"), true );
    CJ.setSignature( std::string("parseArray"), std::string("(II)Ljava/util/ArrayList;"), true );
    CJ.setSignature( std::string("parseSimpleMap"), std::string("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Map;"), true );
    CJ.setSignature( std::string("parseMap"), std::string("([Ljava/lang/String;[I)Ljava/util/Map;"), false );

    // Print signatures
    CJ.printSignatures();

    //jclass jclazz = VM::env->FindClass("java/util/ArrayList");

    // Instantiate converter
    Converter cnv;

    // Set class
    std::string className = "example/Example";
    try {
        CJ.setClass(className);
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        CJ.destroyVM();
        return EXIT_FAILURE;
    }

    // Call constructor
    try {
        CJ.callClassConstructor(NULL);
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        CJ.destroyVM();
        return EXIT_FAILURE;
    }

    // Assertions
    try {

        jboolean Z = CJ.call<jboolean>( "parseBoolean", cnv.j_cast<jboolean>(false) );
        assert (Z == false);

        jbyte B = CJ.call<jbyte>( "parseByte", cnv.j_cast<jbyte>(123) );
        assert (B == 123);

        jchar C = CJ.call<jchar>( "parseChar", cnv.j_cast<jchar>('a') ); // single 16-bit unicode
        assert (C == 'a');

        jshort S = CJ.call<jshort>( "parseShort", cnv.j_cast<jshort>((short) -12) );
        assert (S == -12); // coversion to int avoids lost precision

        jint I = CJ.call<jint>("parseInt", cnv.j_cast<jint>(123) );
        assert (I == 123);

        jlong J = CJ.call<jlong>( "parseLong", cnv.j_cast<jlong>((long long) 123456) );
        assert (J == 123456);

        jfloat F = CJ.call<jfloat>( "parseFloat", cnv.j_cast<jfloat>((float) 123.456) );
        assert ( abs(F - (float) 123.456) <= MAX_TOLERANCE );

        jdouble D = CJ.call<jdouble>( "parseDouble", cnv.j_cast<jdouble>((double) 123.456789) );
        assert ( abs(D - (double) 123.456789) <= MAX_TOLERANCE );

        jobject L = CJ.call<jobject>( "parseString", cnv.j_cast<jstring>("CJay is cool!") );
        std::string str = cnv.c_cast<std::string>(L); // convert the object to string
        assert ( str == std::string("CJay is cool!") );

        L = CJ.call<jobject>( "parseArray", cnv.j_cast<jint>(123), cnv.j_cast<jint>(456) );
        //std::vector<int> v = cnv.toVec<int>(L, 2); // covert to vector<int> (fully unwrapped)
        std::vector<int> v = cnv.c_cast_vector<int>(L);
        // or directly: std::vector<int> v = cnv.toVec<int>(L);
        std::cout << v[0] << v[1] << std::endl;
        assert ( v[0] == 123 ); assert( v[1] == 456 );

    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        CJ.destroyVM();
        return EXIT_FAILURE;
    }

    CJ.destroyVM();

    return EXIT_SUCCESS;
}
