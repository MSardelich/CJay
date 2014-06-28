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

#include "Cjay.hpp"

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

    CJ.setSignature( std::string("parseArrayListByte"), std::string("(BB)Ljava/util/ArrayList;"), true );
    CJ.setSignature( std::string("parseArrayListShort"), std::string("(SS)Ljava/util/ArrayList;"), true );
    CJ.setSignature( std::string("parseArrayListLong"), std::string("(JJ)Ljava/util/ArrayList;"), true );
    CJ.setSignature( std::string("parseArrayListInteger"), std::string("(II)Ljava/util/ArrayList;"), true );
    CJ.setSignature( std::string("parseArrayListFloat"), std::string("(FF)Ljava/util/ArrayList;"), true );
    CJ.setSignature( std::string("parseArrayListDouble"), std::string("(DD)Ljava/util/ArrayList;"), true );
    CJ.setSignature( std::string("parseArrayListString"), std::string("(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;"), true );

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

        jboolean Z = CJ.call<jboolean>( "parseBoolean", (jboolean) false );
        assert (Z == false);

        jbyte B = CJ.call<jbyte>( "parseByte", (jbyte) 123 );
        assert ( (int) B == 123); // convert from signed char to int

        jchar C = CJ.call<jchar>( "parseChar", (jchar) 'a' ); // single 16-bit unicode
        assert (C == 'a');

        jshort S = CJ.call<jshort>( "parseShort", (jshort) -12 );
        assert (S == -12); // coversion to int avoids lost precision

        jint I = CJ.call<jint>("parseInt", (jint) 123 );
        assert (I == 123);

        jlong J = CJ.call<jlong>( "parseLong", (jlong) 123456 );
        assert (J == 123456);

        jfloat F = CJ.call<float>( "parseFloat", (jfloat) 123.456 );
        assert ( abs(F - (float) 123.456) <= MAX_TOLERANCE );

        jdouble D = CJ.call<jdouble>( "parseDouble", (jdouble) 123.456789 );
        assert ( abs(D - (double) 123.456789) <= MAX_TOLERANCE );

        jobject L = CJ.call<jobject>( "parseString", cnv.j_cast<jstring>("foo") );
        std::string str = cnv.c_cast<std::string>(L); // From java.lang.String To string
        assert ( str == std::string("foo") );

        L = CJ.call<jobject>( "parseArrayListByte", (jbyte) 123, (jbyte) -123 );
        std::vector<jbyte> vb = cnv.c_cast_vector<jbyte>(L, 2); // From java.lang.ArrayList<byte> To vector<jbyte> (or vector<signed char>)
        std::cout << (int) vb[0] << " " << (int) vb[1] << std::endl;
        assert ( (int) vb[0] == 123 ); assert( (int) vb[1] == -123 ); // convert signed char to int

        L = CJ.call<jobject>( "parseArrayListInteger", (jint) 123, (jint) 456 );
        std::vector<jint> vi = cnv.c_cast_vector<jint>(L, 2); // From ArrayList<Integer> To vector<jint> (or vector<long>)
        assert ( vi[0] == 123 ); assert( vi[1] == 456 );

        L = CJ.call<jobject>( "parseArrayListLong", (jlong) 123456, (jlong) 123789 );
        std::vector<jlong> vl = cnv.c_cast_vector<jlong>(L, 2); // From ArrayList<Long> To vector<jlong> (or vector<__int64>)
        assert ( vl[0] == 123456 ); assert( vl[1] == 123789 );

        L = CJ.call<jobject>( "parseArrayListShort", (jshort) -12, (jshort) +13 );
        std::vector<jshort> vs = cnv.c_cast_vector<jshort>(L, 2); // From ArrayList<Short> To vector<jshort> (or vector<short>)
        assert ( vs[0] == -12 ); assert( vs[1] == +13 );

        L = CJ.call<jobject>( "parseArrayListFloat", (jfloat) -123.123, (jfloat) 123.145 );
        std::vector<jfloat> vf = cnv.c_cast_vector<jfloat>(L, 2); // From ArrayList<Float> To vector<jfloat> (or vector<float>)
        assert ( abs(vf[0] - (float) -123.123) <= MAX_TOLERANCE ); assert ( abs(vf[1] - (float) 123.145) <= MAX_TOLERANCE );

        L = CJ.call<jobject>( "parseArrayListDouble", (jdouble) 123.1234, (jdouble) -123.4567 );
        std::vector<jdouble> vd = cnv.c_cast_vector<jdouble>(L, 2); // From ArrayList<Double> To vector<jdouble> (or vector<double>)
        assert ( abs(vd[0] - (double) 123.1234) <= MAX_TOLERANCE ); assert ( abs(vd[1] - (double) -123.4567) <= MAX_TOLERANCE );

        L = CJ.call<jobject>( "parseArrayListString", cnv.j_cast<jstring>("foo") , cnv.j_cast<jstring>("bar"));
        std::vector<std::string> v_str = cnv.c_cast_vector<std::string>(L, 2); // From ArrayList<String> To vector<string>
        assert ( v_str[0] == "foo" ); assert( v_str[1] == "bar" );

    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        CJ.destroyVM();
        return EXIT_FAILURE;
    }

    CJ.destroyVM();

    return EXIT_SUCCESS;
}
