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

#include "CJay.hpp"
#include "example/Example.hpp"

#define MAX_TOLERANCE 1.0e-4

jint VM::CJ::JNI_VERSION = DEFAULT_JNI_VERSION; // Depends on installed JDK!

using namespace VM;

int main (int argc, char* argv[]) {
    // Create JVM
    std::vector<std::string> paramVM{"-ea", "-Xdebug"};
    VM::createVM(paramVM);
    //VM::createVM();

    CJ CJ;

    // Set class
    try {
        CJ.setClass("example/Example");
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        VM::destroyVM();
        return EXIT_FAILURE;
    }

    // Print signatures
    CJ.printSignatures();

    // Call constructor
    try {
        CJ.Constructor("<init>"); // contructor has no parameters
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        VM::destroyVM();
        return EXIT_FAILURE;
    }

    // Instantiate caster
    Converter cnv;

    // test seamless integration
    /*
    try {
        cjay::example::Example example;
        jboolean test = example.parseBoolean((jboolean) false);
        assert (test == false);
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        VM::destroyVM();
        return EXIT_FAILURE;
    }
    */

    // Assertions
    try {

        std::vector<jboolean> in {true, false};
        jbooleanArray Za = CJ.call<jbooleanArray>( "parseArrayBoolean", cnv.j_cast<jbooleanArray>(in) );
        std::vector<jboolean> vB = cnv.c_cast_array<jboolean>(Za);
        assert (vB[0] == true); assert (vB[1] == false);

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
        //std::cout << vi[0] << " " << vi[1] << std::endl;
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

        L = CJ.call<jobject>( "parseSimpleMap", cnv.j_cast<jstring>("foo") , cnv.j_cast<jstring>("bar"), cnv.j_cast<jstring>("foo.bar"));
        std::map<std::string, std::string> m_str_str = cnv.c_cast_map<std::string, std::string>(L); // From java.util.Map<String, String> To std::map<string, string>
        assert ( m_str_str["arg 1"] == "foo" ); assert ( m_str_str["arg 2"] == "bar" ); assert ( m_str_str["arg 3"] == "foo.bar" );

    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        VM::destroyVM();
        return EXIT_FAILURE;
    }

    // Destroy VM
    VM::destroyVM();

    return EXIT_SUCCESS;
}
