/***************************************************************************
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
#define DEFAULT_JNI_VERSION JNI_VERSION_1_8
#define CONSTRUCTOR_METHOD_NAME "<init>"

#include "CJay.hpp"

namespace VM {

JNIEnv* env = NULL;
JavaVM* jvm = NULL;

// SignatureBase Members
SignatureBase::SignatureBase(std::string descriptor, bool isStatic) :
    descriptor(descriptor), isStatic(isStatic), mid(NULL) { }

SignatureBase::SignatureBase() : descriptor(""), isStatic(true), mid(NULL) { }
SignatureBase::~SignatureBase() { }

// Signature Members
template <class To> Signature<To>::Signature(std::string descriptor, bool isStatic, RV rv):
        SignatureBase(descriptor, isStatic) {

    this->rv = rv;

    // Assign pCall based on method type.
    // It can be static or non-static.
    if (isStatic) {
        this->pCall = &CJ::callStatic<To>;
    } else {
        this->pCall = &CJ::callNonStatic<To>;
    }
}

template <class To> Signature<To>::Signature() : SignatureBase() {
    this->rv = RV::L;
    this->pCall = NULL;
}

template <class To> Signature<To>::~Signature() { }

// CJ Members

CJ::CJ() : clazz(NULL), obj(NULL) {
    this->m.clear();
}

CJ::~CJ() {
    // avoid memory leaks
    for (auto& kv : this->m) {
        delete kv.second;
        kv.second = NULL;
    }
}

void CJ::setSignature(std::string key, std::string descriptor, bool isStatic) {
    SignatureBase* signature;
	std::string rv;

	// Extract the return value of method
	// based on its descriptor
	std::size_t pos = descriptor.find(")");
	rv = descriptor[pos+1];
    if (rv == "[") { rv = descriptor[pos+2]; } // array cases

    // Assign the type of return variable.
    switch (*rv.c_str())
    {
    case 'Z' : signature = new Signature<jboolean>(descriptor, isStatic, RV::Z); break;
    case 'B' : signature = new Signature<jbyte>(descriptor, isStatic, RV::B); break;
    case 'C' : signature = new Signature<jchar>(descriptor, isStatic, RV::C); break;
    case 'S' : signature = new Signature<jshort>(descriptor, isStatic, RV::S); break;
    case 'I' : signature = new Signature<jint>(descriptor, isStatic, RV::I); break;
    case 'J' : signature = new Signature<jlong>(descriptor, isStatic, RV::J); break;
    case 'F' : signature = new Signature<jfloat>(descriptor, isStatic, RV::F); break;
    case 'D' : signature = new Signature<jdouble>(descriptor, isStatic, RV::D); break;
    case 'L' : signature = new Signature<jobject>(descriptor, isStatic, RV::L); break;
    case 'V' : signature = new Signature<void>(descriptor, isStatic, RV::VV); break;
    default :
        throw HandlerExc("Malformed method descriptor. Please review syntax.");
    }
    this->m.insert(signature_t::value_type(key, signature));
}

void CJ::printSignatures() {
    for (auto& it : m) {
        std::string key = it.first;
        std::cout <<
                "<" <<
                "Method:" << key <<
                ", Descriptor:" << this->m[key]->descriptor <<
                ", isStatic:" << this->m[key]->isStatic <<
                //", RV:" << this->m[key]->rv <<
                ">" <<
                std::endl;
    }
}

jclass CJ::getClass() {
    return this->clazz;
}

jobject CJ::getObj() {
    return this->obj;
}

signature_t CJ::getMap() {
    return this->m;
}

std::string CJ::getDescriptor(std::string key) {
    return this->getSignatureObj(key)->descriptor;
}

jmethodID CJ::getMid(std::string key) {
    return this->getSignatureObj(key)->mid;
}

int CJ::getSizeSignatures() {
    return m.size();
}

void CJ::createVM(std::vector<std::string> vmOption) {
    if (env == NULL || jvm == NULL) {
        int nOptions = vmOption.size();

        JavaVMInitArgs vm_args;
        JavaVMOption* options = new JavaVMOption[nOptions];

        for (std::vector<std::string>::iterator it = vmOption.begin(); it != vmOption.end(); ++it) {
            options[it-vmOption.begin()].optionString = (char*) it->c_str();
        }

        vm_args.version = VM::CJ::JNI_VERSION;
        vm_args.nOptions = nOptions;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = JNI_FALSE;
        int status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args); // create only once with global variables
        if(status != JNI_OK) {
            delete options;
            throw HandlerExc("Unable to Launch JVM");
        }

        delete options; // clean memory leaks
    }
}

void CJ::destroyVM() {
    jvm->DestroyJavaVM();
}

void CJ::setClass(std::string className) {
    if (env == NULL || jvm == NULL) {
    	throw HandlerExc("JVM was not instantiated. Please call member createJVM.");
    }

	this->className = className;
    this->clazz = env->FindClass(className.c_str());
    if (this->clazz == NULL) {
        jthrowable exc = env->ExceptionOccurred();
        if (exc) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            throw HandlerExc("JVM: Can't find class: " + className);
        }
    }

    VM::SignatureBase* obj;
    jmethodID mid;
    for (auto& it : this->m) {
        std::string key = it.first;
        obj = it.second;
        // assign values to be updated
        if (obj->isStatic) {
        	mid = env->GetStaticMethodID(this->clazz, key.c_str(), obj->descriptor.c_str());
        } else {
        	mid = env->GetMethodID(this->clazz, key.c_str(), obj->descriptor.c_str());
        }
        if (mid == NULL) {
            jthrowable exc;
            exc = env->ExceptionOccurred();
            if (exc) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                throw HandlerExc(
                        "Failed to get method ID. Please check the syntax of method: " +
                        key +
                        ", which has the descriptor: " +
                        obj->descriptor
                        );
            }
        }
        // update map
        it.second->mid = mid;
    }

}

VM::SignatureBase* CJ::getSignatureObj(std::string key) {
    if ( this->m.find(key) == this->m.end() ) {
        throw HandlerExc("Key does not exit. Use setSignature member beforehand.");
    }
    return this->m[key];
}

void CJ::callClassConstructor_(int mangledVar, ...) {
    // Get Method Id (Constructor)
    VM::SignatureBase* sig = this->getSignatureObj(CONSTRUCTOR_METHOD_NAME);
    jmethodID mid = sig->mid;
    jobject obj;

    if(mid == NULL) {
        throw HandlerExc("MethodID not set. Probably set class was not set.");
    }

    va_list args;
    va_start(args, mangledVar);

    obj = env->NewObjectV(this->clazz, mid, args);

    va_end(args);

    this->obj = obj;
}

template <typename To> To CJ::call(std::string methodName, ...) {
    SignatureBase* sigSuper = this->getSignatureObj(methodName);
    Signature<To>* sigChild = dynamic_cast<Signature<To>*>(sigSuper);
    jmethodID mid = sigChild->mid;
    To (CJ::*pCall) (jmethodID, va_list);
    pCall = sigChild->pCall;
    To jobj;

    va_list args;
    va_start(args, methodName);

    jobj = (this->*pCall) (mid, args);

    va_end(args);

    return jobj;
}

template <> void CJ::call(std::string methodName, ...) {
    SignatureBase* sigSuper = this->getSignatureObj(methodName);
    Signature<void>* sigChild = dynamic_cast<Signature<void>*>(sigSuper);
    jmethodID mid = sigChild->mid;
    void (CJ::*pCall) (jmethodID, va_list);
    pCall = sigChild->pCall;

    va_list args;
    va_start(args, methodName);

    (this->*pCall) (mid, args);

    va_end(args);
}

template jboolean CJ::call(std::string methodName, ...);
template jbyte CJ::call(std::string methodName, ...);
template jchar CJ::call(std::string methodName, ...);
template jshort CJ::call(std::string methodName, ...);
template jint CJ::call(std::string methodName, ...);
template jlong CJ::call(std::string methodName, ...);
template jfloat CJ::call(std::string methodName, ...);
template jdouble CJ::call(std::string methodName, ...);
template jobject CJ::call(std::string methodName, ...);
template void CJ::call(std::string methodName, ...);

template <> jboolean CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticBooleanMethodV(this->clazz, mid, args);
}

template <> jbyte CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticByteMethodV(this->clazz, mid, args);
}

template <> jchar CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticCharMethodV(this->clazz, mid, args);
}

template <> jshort CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticShortMethodV(this->clazz, mid, args);
}

template <> jint CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticIntMethodV(this->clazz, mid, args);
}

template <> jlong CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticLongMethodV(this->clazz, mid, args);
}

template <> jfloat CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticFloatMethodV(this->clazz, mid, args);
}

template <> jdouble CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticDoubleMethodV(this->clazz, mid, args);
}

template <> jobject CJ::callStatic(jmethodID mid, va_list args) {
    return env->CallStaticObjectMethodV(this->clazz, mid, args);
}

template <> void CJ::callStatic(jmethodID mid, va_list args) {
    env->CallStaticVoidMethodV(this->clazz, mid, args);
}

template jboolean CJ::callStatic(jmethodID, va_list);
template jbyte CJ::callStatic(jmethodID, va_list);
template jchar CJ::callStatic(jmethodID, va_list);
template jshort CJ::callStatic(jmethodID, va_list);
template jint CJ::callStatic(jmethodID, va_list);
template jlong CJ::callStatic(jmethodID, va_list);
template jfloat CJ::callStatic(jmethodID, va_list);
template jdouble CJ::callStatic(jmethodID, va_list);
template jobject CJ::callStatic(jmethodID, va_list);
template void CJ::callStatic(jmethodID, va_list);

template <> jboolean CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallBooleanMethodV(this->obj, mid, args);
}

template <> jbyte CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallByteMethodV(this->obj, mid, args);
}

template <> jchar CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallCharMethodV(this->obj, mid, args);
}

template <> jshort CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallShortMethodV(this->obj, mid, args);
}

template <> jint CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallIntMethodV(this->obj, mid, args);
}

template <> jlong CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallLongMethodV(this->obj, mid, args);
}

template <> jfloat CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallFloatMethodV(this->obj, mid, args);
}

template <> jdouble CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallDoubleMethodV(this->obj, mid, args);
}

template <> jobject CJ::callNonStatic(jmethodID mid, va_list args) {
    return env->CallObjectMethodV(this->obj, mid, args);
}

template <> void CJ::callNonStatic(jmethodID mid, va_list args) {
    env->CallVoidMethodV(this->obj, mid, args);
}

template jboolean CJ::callNonStatic(jmethodID, va_list);
template jbyte CJ::callNonStatic(jmethodID, va_list);
template jchar CJ::callNonStatic(jmethodID, va_list);
template jshort CJ::callNonStatic(jmethodID, va_list);
template jint CJ::callNonStatic(jmethodID, va_list);
template jlong CJ::callNonStatic(jmethodID, va_list);
template jfloat CJ::callNonStatic(jmethodID, va_list);
template jdouble CJ::callNonStatic(jmethodID, va_list);
template jobject CJ::callNonStatic(jmethodID, va_list);
template void CJ::callNonStatic(jmethodID, va_list);

JNIEnv* CJ::getEnv() {
    return env;
}


// ConverterBase Members (super class)
ConverterBase::ConverterBase() { }

ConverterBase::~ConverterBase() { }

/**
 ** Converter Members (child class)
 **/
Converter::Converter(): ConverterBase() { this->init(); }
Converter::~Converter() { }

void Converter::initCORE() {
    CORE.setSignature( "FromMapToArrayListOfKeys" , "(Ljava/util/Map;)Ljava/util/ArrayList;" , true );
    CORE.setSignature( "FromMapToArrayListOfValues" , "(Ljava/util/Map;)Ljava/util/ArrayList;" , true );

    CORE.setClass("cjay/CJay");
}

void Converter::initARRAYLIST() {
    ARRAYLIST.setSignature( "toString", "()Ljava/lang/String;", false );
    ARRAYLIST.setSignature( "get", "(I)Ljava/lang/Object;", false );
    ARRAYLIST.setSignature( "size", "()I", false );

    ARRAYLIST.setClass("java/util/ArrayList");
}

void Converter::initSET() {
    SET.setSignature( "iterator", "()Ljava/util/Iterator;", false );
    SET.setSignature( "size", "()I", false );

    SET.setClass("java/util/Set");
}

void Converter::initCOLLECTION() {
    COLLECTION.setSignature( "iterator", "()Ljava/util/Iterator;", false );
    COLLECTION.setSignature( "size", "()I", false );

    COLLECTION.setClass("java/util/Collection");
}
/*
void Converter::initITERATOR() {
    ITERATOR_INTEGER.setSignature( "hasNext", "()Z", false );
    //ITERATOR.setSignature( "next", "()I", false );

    ITERATOR.setClass("java/util/Iterator");
}
*/
void Converter::initMAP() {
    MAP.setSignature( "keySet", "()Ljava/util/Set;", false );
    MAP.setSignature( "values", "()Ljava/util/Collection;", false );
    MAP.setSignature( "size", "()I", false );

    MAP.setClass("java/util/Map");
}

void Converter::initNUMBER() {
    NUMBER.setSignature( "intValue", "()I", false );
    NUMBER.setSignature( "longValue", "()J", false );
    NUMBER.setSignature( "floatValue", "()F", false );
    NUMBER.setSignature( "doubleValue", "()D", false );
    NUMBER.setSignature( "shortValue", "()S", false );

    NUMBER.setClass("java/lang/Number");
}

void Converter::initBOOLEAN() {
    BOOLEAN.setSignature( "booleanValue", "()Z", false );
    BOOLEAN.setSignature( "valueOf", "(Z)Ljava/lang/Boolean;", true );

    BOOLEAN.setClass("java/lang/Boolean");
}

void Converter::initBYTE() {
    BYTE.setSignature( "byteValue", "()B", false );
    BYTE.setSignature( "valueOf", "(B)Ljava/lang/Byte;", true );

    BYTE.setClass("java/lang/Byte");
}

void Converter::initSHORT() {
    SHORT.setSignature( "shortValue", "()S", false );
    SHORT.setSignature( "valueOf", "(S)Ljava/lang/Short;", true );

    SHORT.setClass("java/lang/Short");
}

void Converter::initLONG() {
    LONG.setSignature( "longValue", "()J", false );
    LONG.setSignature( "valueOf", "(J)Ljava/lang/Long;", true );

    LONG.setClass("java/lang/Long");
}

void Converter::initINTEGER() {
    INTEGER.setSignature( "intValue", "()I", false );
    INTEGER.setSignature( "valueOf", "(I)Ljava/lang/Integer;", true );

    INTEGER.setClass("java/lang/Integer");
}

void Converter::initFLOAT() {
    FLOAT.setSignature( "floatValue", "()F", false );
    FLOAT.setSignature( "valueOf", "(F)Ljava/lang/Float;", true );

    FLOAT.setClass("java/lang/Float");
}

void Converter::initDOUBLE() {
    DOUBLE.setSignature( "doubleValue", "()D", false );
    DOUBLE.setSignature( "valueOf", "(D)Ljava/lang/Double;", true );

    DOUBLE.setClass("java/lang/Double");
}

void Converter::initCHARACTER() {
    CHARACTER.setSignature( "charValue", "()C", false );
    CHARACTER.setSignature( "valueOf", "(C)Ljava/lang/Character;", true );

    CHARACTER.setClass("java/lang/Character");
}

void Converter::init() {
    this->initCORE();

    this->initARRAYLIST();
    this->initSET();
    this->initCOLLECTION();
    this->initMAP();

    this->initNUMBER();

    this->initBOOLEAN();
    this->initBYTE();
    this->initSHORT();
    this->initLONG();
    this->initINTEGER();
    this->initFLOAT();
    this->initDOUBLE();
    this->initCHARACTER();
}

template <> jobject Converter::j_cast(jboolean x) {
    return BOOLEAN.call<jobject>("valueOf", x);
}

template <> jobject Converter::j_cast(jbyte x) {
    return BYTE.call<jobject>("valueOf", x);
}

template <> jobject Converter::j_cast(jshort x) {
    return SHORT.call<jobject>("valueOf", x);
}

template <> jobject Converter::j_cast(jlong x) {
    return LONG.call<jobject>("valueOf", x);
}

template <> jobject Converter::j_cast(jint x) {
    return INTEGER.call<jobject>("valueOf", x);
}

template <> jobject Converter::j_cast(jfloat x) {
    return FLOAT.call<jobject>("valueOf", x);
}

template <> jobject Converter::j_cast(jdouble x) {
    return DOUBLE.call<jobject>("valueOf", x);
}

template <> jobject Converter::j_cast(jchar x) {
    return CHARACTER.call<jobject>("valueOf", x);
}

template <> jstring Converter::j_cast(std::string str) {
    return env->NewStringUTF(str.c_str());
}

template <> jstring Converter::j_cast(const char* str) {
    return env->NewStringUTF(str);
}

/* c_cast<> specialization */
template <> jbyte Converter::c_cast(jobject jobj) {
    jmethodID mid = BYTE.getSignatureObj("byteValue")->mid;
    return env->CallByteMethod(jobj, mid, NULL);
}

template <> jint Converter::c_cast(jobject jobj) {
    jmethodID mid = INTEGER.getSignatureObj("intValue")->mid;
    return env->CallIntMethod(jobj, mid, NULL);
}

template <> jlong Converter::c_cast(jobject jobj) {
    jmethodID mid = LONG.getSignatureObj("longValue")->mid;
    return env->CallLongMethod(jobj, mid, NULL);
}

template <> jshort Converter::c_cast(jobject jobj) {
    jmethodID mid = SHORT.getSignatureObj("shortValue")->mid;
    return env->CallShortMethod(jobj, mid, NULL);
}

template <> jfloat Converter::c_cast(jobject jobj) {
    jmethodID mid = FLOAT.getSignatureObj("floatValue")->mid;
    return env->CallFloatMethod(jobj, mid, NULL);
}

template <> jdouble Converter::c_cast(jobject jobj) {
    jmethodID mid = DOUBLE.getSignatureObj("doubleValue")->mid;
    return env->CallDoubleMethod(jobj, mid, NULL);
}

template <> jboolean Converter::c_cast(jobject jobj) {
    jmethodID mid = BOOLEAN.getSignatureObj("booleanValue")->mid;
    return env->CallBooleanMethod(jobj, mid, NULL);
}

template <> jchar Converter::c_cast(jobject jobj) {
    jmethodID mid = CHARACTER.getSignatureObj("charValue")->mid;
    return env->CallCharMethod(jobj, mid, NULL);
}

template <> std::string Converter::c_cast(jobject jobj) {
    return std::string(env->GetStringUTFChars((jstring) jobj, JNI_FALSE));
}

template <> jobject Converter::c_cast(jobject jobj) {
    return jobj;
}

int Converter::sizeVector(jobject jobj) {
    VM::SignatureBase* sig = ARRAYLIST.getSignatureObj("size");

    return env->CallIntMethod(jobj, sig->mid, NULL);
}

template <typename To> std::vector<To> Converter::c_cast_vector(jobject jobj, int size) {
    jmethodID mid = ARRAYLIST.getSignatureObj("get")->mid;
    jobject e;
    std::vector<To> v;

    for (int i = 0 ; i < size ; i++) {
        e = env->CallObjectMethod(jobj, mid, (jint) i); // get element
        v.push_back( this->c_cast<To>(e) ); // convert to primitive
    }

    return v;
}

template <typename To> std::vector<To> Converter::c_cast_vector(jobject jobj) {
    int size = this->sizeVector(jobj);
    return this->c_cast_vector<To>(jobj, size);
}

template std::vector<jint> Converter::c_cast_vector(jobject, int);
template std::vector<jshort> Converter::c_cast_vector(jobject, int);
template std::vector<jlong> Converter::c_cast_vector(jobject, int);
template std::vector<jfloat> Converter::c_cast_vector(jobject, int);
template std::vector<jdouble> Converter::c_cast_vector(jobject, int);
template std::vector<jbyte> Converter::c_cast_vector(jobject, int);
template std::vector<jchar> Converter::c_cast_vector(jobject, int);
template std::vector<jobject> Converter::c_cast_vector(jobject, int);
template std::vector<std::string> Converter::c_cast_vector(jobject, int);

template std::vector<jint> Converter::c_cast_vector(jobject);
template std::vector<jshort> Converter::c_cast_vector(jobject);
template std::vector<jlong> Converter::c_cast_vector(jobject);
template std::vector<jfloat> Converter::c_cast_vector(jobject);
template std::vector<jdouble> Converter::c_cast_vector(jobject);
template std::vector<jbyte> Converter::c_cast_vector(jobject);
template std::vector<jchar> Converter::c_cast_vector(jobject);
template std::vector<jobject> Converter::c_cast_vector(jobject);
template std::vector<std::string> Converter::c_cast_vector(jobject);

jobject Converter::getKeysOfMap(jobject jmap) {
    jmethodID mid = CORE.getSignatureObj("FromMapToArrayListOfKeys")->mid;
    jobject arrayListOfKeys = env->CallStaticObjectMethod(CORE.getClass(), mid, jmap);

    return arrayListOfKeys;
}

jobject Converter::getValuesOfMap(jobject jmap) {
    jmethodID mid = CORE.getSignatureObj("FromMapToArrayListOfValues")->mid;
    jobject arrayListOfValues = env->CallStaticObjectMethod(CORE.getClass(), mid, jmap);

    return arrayListOfValues;
}

template <typename K, typename V> std::map<K, V> Converter::c_cast_map(jobject jmap) {
    std::map<K, V> cmap;
    std::vector<K> vKeys = this->c_cast_vector<K>(this->getKeysOfMap(jmap));
    std::vector<V> vValues = this->c_cast_vector<V>(this->getValuesOfMap(jmap));

    std::size_t size = vKeys.size();

    for (std::size_t i = 0 ; i < size ; i++) {
        cmap.insert(std::pair<K, V>( vKeys[i], vValues[i] ));
    }

    return cmap;
}

/* <jboolean, V> */
template std::map<jboolean, jboolean> Converter::c_cast_map(jobject);
template std::map<jboolean, jbyte> Converter::c_cast_map(jobject);
template std::map<jboolean, jchar> Converter::c_cast_map(jobject);
template std::map<jboolean, jshort> Converter::c_cast_map(jobject);
template std::map<jboolean, jint> Converter::c_cast_map(jobject);
template std::map<jboolean, jlong> Converter::c_cast_map(jobject);
template std::map<jboolean, jfloat> Converter::c_cast_map(jobject);
template std::map<jboolean, jdouble> Converter::c_cast_map(jobject);
template std::map<jboolean, jobject> Converter::c_cast_map(jobject);
template std::map<jboolean, std::string> Converter::c_cast_map(jobject);

/* <jbyte, V> */
template std::map<jbyte, jboolean> Converter::c_cast_map(jobject);
template std::map<jbyte, jbyte> Converter::c_cast_map(jobject);
template std::map<jbyte, jchar> Converter::c_cast_map(jobject);
template std::map<jbyte, jshort> Converter::c_cast_map(jobject);
template std::map<jbyte, jint> Converter::c_cast_map(jobject);
template std::map<jbyte, jlong> Converter::c_cast_map(jobject);
template std::map<jbyte, jfloat> Converter::c_cast_map(jobject);
template std::map<jbyte, jdouble> Converter::c_cast_map(jobject);
template std::map<jbyte, jobject> Converter::c_cast_map(jobject);
template std::map<jbyte, std::string> Converter::c_cast_map(jobject);

/* <jchar, V> */
template std::map<jchar, jboolean> Converter::c_cast_map(jobject);
template std::map<jchar, jbyte> Converter::c_cast_map(jobject);
template std::map<jchar, jchar> Converter::c_cast_map(jobject);
template std::map<jchar, jshort> Converter::c_cast_map(jobject);
template std::map<jchar, jint> Converter::c_cast_map(jobject);
template std::map<jchar, jlong> Converter::c_cast_map(jobject);
template std::map<jchar, jfloat> Converter::c_cast_map(jobject);
template std::map<jchar, jdouble> Converter::c_cast_map(jobject);
template std::map<jchar, jobject> Converter::c_cast_map(jobject);
template std::map<jchar, std::string> Converter::c_cast_map(jobject);

/* <jshort, V> */
template std::map<jshort, jboolean> Converter::c_cast_map(jobject);
template std::map<jshort, jbyte> Converter::c_cast_map(jobject);
template std::map<jshort, jchar> Converter::c_cast_map(jobject);
template std::map<jshort, jshort> Converter::c_cast_map(jobject);
template std::map<jshort, jint> Converter::c_cast_map(jobject);
template std::map<jshort, jlong> Converter::c_cast_map(jobject);
template std::map<jshort, jfloat> Converter::c_cast_map(jobject);
template std::map<jshort, jdouble> Converter::c_cast_map(jobject);
template std::map<jshort, jobject> Converter::c_cast_map(jobject);
template std::map<jshort, std::string> Converter::c_cast_map(jobject);

/* <jint, V> */
template std::map<jint, jboolean> Converter::c_cast_map(jobject);
template std::map<jint, jbyte> Converter::c_cast_map(jobject);
template std::map<jint, jchar> Converter::c_cast_map(jobject);
template std::map<jint, jshort> Converter::c_cast_map(jobject);
template std::map<jint, jint> Converter::c_cast_map(jobject);
template std::map<jint, jlong> Converter::c_cast_map(jobject);
template std::map<jint, jfloat> Converter::c_cast_map(jobject);
template std::map<jint, jdouble> Converter::c_cast_map(jobject);
template std::map<jint, jobject> Converter::c_cast_map(jobject);
template std::map<jint, std::string> Converter::c_cast_map(jobject);

/* <jlong, V> */
template std::map<jlong, jboolean> Converter::c_cast_map(jobject);
template std::map<jlong, jbyte> Converter::c_cast_map(jobject);
template std::map<jlong, jchar> Converter::c_cast_map(jobject);
template std::map<jlong, jshort> Converter::c_cast_map(jobject);
template std::map<jlong, jint> Converter::c_cast_map(jobject);
template std::map<jlong, jlong> Converter::c_cast_map(jobject);
template std::map<jlong, jfloat> Converter::c_cast_map(jobject);
template std::map<jlong, jdouble> Converter::c_cast_map(jobject);
template std::map<jlong, jobject> Converter::c_cast_map(jobject);
template std::map<jlong, std::string> Converter::c_cast_map(jobject);

/* <jfloat, V> */
template std::map<jfloat, jboolean> Converter::c_cast_map(jobject);
template std::map<jfloat, jbyte> Converter::c_cast_map(jobject);
template std::map<jfloat, jchar> Converter::c_cast_map(jobject);
template std::map<jfloat, jshort> Converter::c_cast_map(jobject);
template std::map<jfloat, jint> Converter::c_cast_map(jobject);
template std::map<jfloat, jlong> Converter::c_cast_map(jobject);
template std::map<jfloat, jfloat> Converter::c_cast_map(jobject);
template std::map<jfloat, jdouble> Converter::c_cast_map(jobject);
template std::map<jfloat, jobject> Converter::c_cast_map(jobject);
template std::map<jfloat, std::string> Converter::c_cast_map(jobject);

/* <jdouble, V> */
template std::map<jdouble, jboolean> Converter::c_cast_map(jobject);
template std::map<jdouble, jbyte> Converter::c_cast_map(jobject);
template std::map<jdouble, jchar> Converter::c_cast_map(jobject);
template std::map<jdouble, jshort> Converter::c_cast_map(jobject);
template std::map<jdouble, jint> Converter::c_cast_map(jobject);
template std::map<jdouble, jlong> Converter::c_cast_map(jobject);
template std::map<jdouble, jfloat> Converter::c_cast_map(jobject);
template std::map<jdouble, jdouble> Converter::c_cast_map(jobject);
template std::map<jdouble, jobject> Converter::c_cast_map(jobject);
template std::map<jdouble, std::string> Converter::c_cast_map(jobject);

/* <jobject, V> */
template std::map<jobject, jboolean> Converter::c_cast_map(jobject);
template std::map<jobject, jbyte> Converter::c_cast_map(jobject);
template std::map<jobject, jchar> Converter::c_cast_map(jobject);
template std::map<jobject, jshort> Converter::c_cast_map(jobject);
template std::map<jobject, jint> Converter::c_cast_map(jobject);
template std::map<jobject, jlong> Converter::c_cast_map(jobject);
template std::map<jobject, jfloat> Converter::c_cast_map(jobject);
template std::map<jobject, jdouble> Converter::c_cast_map(jobject);
template std::map<jobject, jobject> Converter::c_cast_map(jobject);
template std::map<jobject, std::string> Converter::c_cast_map(jobject);

/* <std::string, V> */
template std::map<std::string, jboolean> Converter::c_cast_map(jobject);
template std::map<std::string, jbyte> Converter::c_cast_map(jobject);
template std::map<std::string, jchar> Converter::c_cast_map(jobject);
template std::map<std::string, jshort> Converter::c_cast_map(jobject);
template std::map<std::string, jint> Converter::c_cast_map(jobject);
template std::map<std::string, jlong> Converter::c_cast_map(jobject);
template std::map<std::string, jfloat> Converter::c_cast_map(jobject);
template std::map<std::string, jdouble> Converter::c_cast_map(jobject);
template std::map<std::string, jobject> Converter::c_cast_map(jobject);
template std::map<std::string, std::string> Converter::c_cast_map(jobject);

void Converter::deleteRef(jobject jobj) {
    env->DeleteLocalRef(jobj);
}

} /* namespace VM */
