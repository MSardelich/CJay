/**************************************************************************
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

    // Create JVM
    try {
        CJ.createVM(vmOption); // creates JVM
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Set member signatures
    CJ.setSignature( "<init>", "()V", false );
    CJ.setSignature( "parseBoolean", "(Z)Z", false );
    CJ.setSignature( "parseByte", "(B)B", false );
    CJ.setSignature( "parseChar", "(C)C", false );
    CJ.setSignature( "parseShort", "(S)S", false );
    CJ.setSignature( "parseInt", "(I)I", false );
    CJ.setSignature( "parseLong", "(J)J", false );
    CJ.setSignature( "parseFloat", "(F)F", false );
    CJ.setSignature( "parseDouble", "(D)D", false );
    CJ.setSignature( "parseString", "(Ljava/lang/String;)Ljava/lang/String;", true );

    CJ.setSignature( "parseArrayListByte", "(BB)Ljava/util/ArrayList;", true );
    CJ.setSignature( "parseArrayListShort", "(SS)Ljava/util/ArrayList;", true );
    CJ.setSignature( "parseArrayListLong", "(JJ)Ljava/util/ArrayList;", true );
    CJ.setSignature( "parseArrayListInteger", "(II)Ljava/util/ArrayList;", true );
    CJ.setSignature( "parseArrayListFloat", "(FF)Ljava/util/ArrayList;", true );
    CJ.setSignature( "parseArrayListDouble", "(DD)Ljava/util/ArrayList;", true );
    CJ.setSignature( "parseArrayListString", "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/ArrayList;", true );

    CJ.setSignature( "parseSimpleMap", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/util/Map;", true );
    CJ.setSignature( "parseMap", "([Ljava/lang/String;[I)Ljava/util/Map;", false );

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
