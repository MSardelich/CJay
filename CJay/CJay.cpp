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

/**
 ** VM implementation
 **/

JNIEnv* env = NULL;
JavaVM* jvm = NULL;

inline std::string getParmPath() {
    char* pPath = getenv("CLASSPATH");
    if(pPath == NULL) {
        throw HandlerExc("CJay: You have to set system environment variable CLASSPATH. Don't forget to add the path to java class you want to call from C++");
    }
    std::string parmPath = std::string("-Djava.class.path=") + std::string(pPath);
    return parmPath;
}

inline jint createJavaVM(JavaVMInitArgs& vm_args) {
    return JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args); // create only once with global variables
}

inline char* TOCHAR (std::string str) {
    char * writable = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), writable);
    writable[str.size()] = '\0'; // don't forget the terminating 0

    return writable;
}

void createVM(std::vector<std::string>& vmOption) {
    if (env == NULL || jvm == NULL) {
        int nOptions = vmOption.size();

        JavaVMInitArgs vm_args;
        JavaVMOption* options = new JavaVMOption[nOptions + 1];

        // Assign options
        for (std::vector<std::string>::iterator it = vmOption.begin(); it != vmOption.end(); ++it) {
            options[it-vmOption.begin()].optionString = const_cast<char*>(it->c_str()); //(char*) it->c_str();
        }
        // Add CLASSPATH parameter
        char* paramPath = TOCHAR(getParmPath());
        options[nOptions].optionString = paramPath;
        // Assign vm_args
        vm_args.version = CJ::JNI_VERSION;
        vm_args.nOptions = nOptions + 1;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = JNI_FALSE;
        // Create JavaVM
        jint status = createJavaVM(vm_args);
        //int status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args); // create only once with global variables
        // Check Status
        if(status != JNI_OK) {
            delete[] paramPath;
            delete[] options;
            throw HandlerExc("JNI: Unable to launch JVM. JNI_CreateJavaVM call failed.");
        }
        delete[] paramPath;
        delete options; // clean memory leaks
    }
}

void createVM() {
    if (env == NULL || jvm == NULL) {
        JavaVMInitArgs vm_args;
        JavaVMOption* options = new JavaVMOption[1];

        // Add CLASSPATH parameter to options
        char* paramPath = TOCHAR(getParmPath());
        options[0].optionString = paramPath;
        // Assign vm_args
        vm_args.version = CJ::JNI_VERSION;
        vm_args.nOptions = 1;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = JNI_FALSE;
        // Create JavaVM
        jint status = createJavaVM(vm_args);
        //int status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args); // create only once with global variables
        // Check status
        if(status != JNI_OK) {
            delete[] paramPath;
            delete[] options;
            throw HandlerExc("JNI: Unable to launch JVM. JNI_CreateJavaVM call failed.");
        }
        delete[] paramPath;
        delete[] options; // clean memory leaks
    }
}

void destroyVM() {
    jvm->DestroyJavaVM();
}

template <> std::string FromJavaObjectToCpp(jobject x) {
    std::string str = std::string(env->GetStringUTFChars((jstring) x, JNI_FALSE));
    return str;
}

template <> bool FromJavaObjectToCpp(jobject x) {
    //jclass UTIL = env->FindClass("cjay/converter/Util");
    //jmethodID midCastBoolean = env->GetStaticMethodID(UTIL, "FromObjectToBoolean", "(Ljava/lang/Object;)Ljava/lang/Boolean;");
    jclass BOOLEAN = env->FindClass("java/lang/Boolean");
    jmethodID midBooleanValue = env->GetMethodID(BOOLEAN, "booleanValue", "()Z");

    //jobject jWCBoolean = env->CallStaticObjectMethod(UTIL, midCastBoolean, x);
    //jboolean jBoolean = env->CallBooleanMethod(jWCBoolean, midBooleanValue);
    jboolean jBoolean = env->CallBooleanMethod(x, midBooleanValue);

    return (bool) jBoolean;
}

template <typename To> std::vector<To> FromALToVector(jobject arrayList) {
    jclass ARRAYLIST = env->FindClass("java/util/ArrayList");
    jmethodID midGet = env->GetMethodID(ARRAYLIST, "get", "(I)Ljava/lang/Object;");
    jmethodID midSize = env->GetMethodID(ARRAYLIST, "size", "()I");

    std::vector<To> cVec;

    jint size = env->CallIntMethod(arrayList, midSize);
    for(jint i = 0; i < size ; i++) {
        jobject jobj = env->CallObjectMethod(arrayList, midGet, i);
        cVec.push_back(FromJavaObjectToCpp<To>(jobj));
    }

    return cVec;
}

template std::vector<std::string> FromALToVector(jobject);
template std::vector<bool> FromALToVector(jobject);

/**
 ** JavaMethodReflect implementation
 **/
JavaMethodReflect::JavaMethodReflect(std::string name, std::string descriptor, bool isStatic) :
    name(name), descriptor(descriptor), isStatic(isStatic) { }

JavaMethodReflect::JavaMethodReflect() : name(""), descriptor(""), isStatic(false) { }
JavaMethodReflect::~JavaMethodReflect() { }

/**
 ** SignatureBase implementation
 **/
SignatureBase::SignatureBase(std::string name, std::string descriptor, bool isStatic) :
    name(name), descriptor(descriptor), isStatic(isStatic) , mid(NULL) { }

SignatureBase::SignatureBase() : name(""), descriptor(""), isStatic(false), mid(NULL) { }
SignatureBase::~SignatureBase() { }

/**
 ** Signature implementation
 **/
template <class To> Signature<To>::Signature(
        std::string name, std::string descriptor, bool isStatic, RV rv) :
        SignatureBase(name, descriptor, isStatic) {

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

/**
 ** CJ implementation
 **/
CJ::CJ() : clazz(NULL), obj(NULL) {
    this->methodLinkage.clear();
}

CJ::~CJ() {
    // avoid memory leaks
    for (auto& kv : this->methodLinkage) {
        delete kv.second;
        kv.second = NULL;
    }
}

void CJ::assignMethodReflectCollection() {
    jclass clazzReflect = env->FindClass("cjay/reflect/Signature");
    // Reflect methodIDs
    jmethodID midConstructor = env->GetMethodID(clazzReflect, "<init>", "(Ljava/lang/Class;)V");
    jmethodID midNames = env->GetMethodID(clazzReflect, "getAllMembersNames", "()Ljava/util/ArrayList;");
    jmethodID midDescriptors = env->GetMethodID(clazzReflect, "getAllMembersDescriptors", "()Ljava/util/ArrayList;");
    jmethodID midIsStatic = env->GetMethodID(clazzReflect, "getAllMembersIsStatic", "()Ljava/util/ArrayList;");

    // Call constructor
    jobject oReflect = env->NewObject(
            clazzReflect,
            midConstructor,
            this->clazz
            );

    jobject ALNames = env->CallObjectMethod(oReflect, midNames);
    jobject ALDescriptors = env->CallObjectMethod(oReflect, midDescriptors);
    jobject ALIsStatic = env->CallObjectMethod(oReflect, midIsStatic);

    // Convert from Java Array List to C++ STL vetcor
    std::vector<std::string> names = FromALToVector<std::string>(ALNames);
    std::vector<std::string> descriptors = FromALToVector<std::string>(ALDescriptors);
    std::vector<bool> isStatic = FromALToVector<bool>(ALIsStatic);

    // Create unique keys based on method names.
    // IMPORTANT: Overloaded java methods have the same name with different signatures.
    // We need to accord on how to uniquely refer to these method.
    // The convention: unique_key = <original_method_name>_<a_number>

    // Search what method were overloaded,
    // in other words, the methods that have the same name.
    this->isNonUnique.clear();
    for(size_t i = 0 ; i < names.size(); i++) {
        for(size_t j = 0 ; j < names.size(); j++) {
            if(names[i] == names[j] && i != j) {
                this->isNonUnique.insert(isNonUniqueCollection::value_type(names[i], 0)); // Store non-unique method name
            }
        }
    }

    // Assign keys, taking overloaded methods convention into account
    std::vector<std::string> keys;
    std::string key;
    int timesNameRepeat;
    std::string intString;
    for(auto& name : names) {
        if (this->isNonUnique.find(name) == this->isNonUnique.end() ) { // current name is unique
            key.assign(name);
        } else { // current method name is non-unqiue
            timesNameRepeat = this->isNonUnique[name] + 1;
            // add line below because gcc complier complains with standard C++11 "std::to_string" instruction.
            intString.assign(static_cast<std::ostringstream*>( &(std::ostringstream() << timesNameRepeat) )->str());
            key.assign(name + "_" + intString);
            this->isNonUnique[name] = timesNameRepeat;
        }
        keys.push_back(key);
    }

    // Assign method reflect
    this->methodReflect.clear();
    for(size_t i = 0; i < keys.size(); i++) {
        JavaMethodReflect methodR(names[i], descriptors[i], isStatic[i]);
        this->methodReflect.insert(methodReflectCollection::value_type(keys[i], methodR));
    }
}

void CJ::assignMethodLinkageCollection() {
    SignatureBase* signature;
    std::string rv; // method return value
    std::string key; // the unique identifier of method
    JavaMethodReflect methodR; // method details (java reflection)
    std::string name;
    std::string descriptor;
    bool isStatic;

    for(auto& kv : this->methodReflect) {
        key = kv.first;
        methodR = kv.second;
        name.assign(methodR.name);
        descriptor.assign(methodR.descriptor);
        isStatic = methodR.isStatic;

        // Extract method return value
        std::size_t pos = descriptor.find(")");
        rv = descriptor[pos+1];
        if (rv == "[") { rv = descriptor[pos+2]; } // array case

        // Assign signature
        switch (*rv.c_str())
        {
        case 'Z' : signature = new Signature<jboolean>(name, descriptor, isStatic, RV::Z); break;
        case 'B' : signature = new Signature<jbyte>(name, descriptor, isStatic, RV::B); break;
        case 'C' : signature = new Signature<jchar>(name, descriptor, isStatic, RV::C); break;
        case 'S' : signature = new Signature<jshort>(name, descriptor, isStatic, RV::S); break;
        case 'I' : signature = new Signature<jint>(name, descriptor, isStatic, RV::I); break;
        case 'J' : signature = new Signature<jlong>(name, descriptor, isStatic, RV::J); break;
        case 'F' : signature = new Signature<jfloat>(name, descriptor, isStatic, RV::F); break;
        case 'D' : signature = new Signature<jdouble>(name, descriptor, isStatic, RV::D); break;
        case 'L' : signature = new Signature<jobject>(name, descriptor, isStatic, RV::L); break;
        case 'V' : signature = new Signature<void>(name, descriptor, isStatic, RV::VV); break;
        default :
            throw HandlerExc("CJay Error: Malformed method descriptor.");
        }

        // Store linkage
        this->methodLinkage.insert(methodLinkageCollection::value_type(key, signature));
    }
}

void CJ::assignCollections() {
    this->assignMethodReflectCollection();
    this->assignMethodLinkageCollection();
}

void CJ::printSignatures() {
    for (auto& it : methodReflect) {
        std::string key = it.first;
        std::cout <<
                "<" <<
                "Unique Key:" << key <<
                ", Name: " << this->methodLinkage[key]->name <<
                ", Descriptor: " << this->methodLinkage[key]->descriptor <<
                ", isStatic: " << this->methodLinkage[key]->isStatic <<
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

std::string CJ::getUniqueKey(std::string name, std::string descriptor) {
    std::string keyMatch = "";
    for(auto& kv : this->methodLinkage) {
        if( (name == kv.second->name) && (descriptor == kv.second->descriptor) ) {
            keyMatch.assign(kv.first);
            break;
        }
    }
    if(keyMatch == "") {
        throw HandlerExc("CJay: There is no java method with name equal to " + name + " and descriptor equal to " + descriptor);
    }

    return keyMatch;
}

methodLinkageCollection CJ::getMap() {
    return this->methodLinkage;
}

std::string CJ::getDescriptor(std::string key) {
    return this->getSignatureObj(key)->descriptor;
}

jmethodID CJ::getMid(std::string key) {
    return this->getSignatureObj(key)->mid;
}

int CJ::getSizeSignatures() {
    return this->methodLinkage.size();
}

void CJ::setClass(std::string className) {
    if (env == NULL || jvm == NULL) {
    	throw HandlerExc("CJay: No Java Virtual Machine instance. Please, call VM::createVM beforehand.");
    }

    jclass clazz = env->FindClass(className.c_str());
	if (clazz == NULL) {
        jthrowable exc = env->ExceptionOccurred();
        if (exc) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            throw HandlerExc("JNI: Can't find class " + className);
        }
    }
	this->className = className;
	this->clazz = clazz;

	// Assign: Java Reflect Collection & Method Linkage
	this->assignCollections();

	// Set methodID of signatures
    VM::SignatureBase* signature;
    jmethodID mid;
    for (auto& it : this->methodLinkage) {
        std::string key = it.first;
        signature = it.second;
        // get methodID
        if (signature->isStatic) {
        	mid = env->GetStaticMethodID(this->clazz, signature->name.c_str(), signature->descriptor.c_str());
        } else {
        	mid = env->GetMethodID(this->clazz, signature->name.c_str(), signature->descriptor.c_str());
        }
        if (mid == NULL) {
            jthrowable exc;
            exc = env->ExceptionOccurred();
            if (exc) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                throw HandlerExc(
                    "JNI: Failed to get method ID of " + key + " with descriptor: " + signature->descriptor);
            }
        }
        // update signature
        it.second->mid = mid;
    }

}

VM::SignatureBase* CJ::getSignatureObj(std::string key) {
    if ( this->methodLinkage.find(key) == this->methodLinkage.end() ) {
        throw HandlerExc("Key does not exit. Use setSignature member beforehand.");
    }
    return this->methodLinkage[key];
}

void CJ::Constructor(std::string key, ...) {
    // Get Method Id (Constructor)
    VM::SignatureBase* sig = this->getSignatureObj(key);
    jmethodID mid = sig->mid;
    jobject obj;

    if(mid == NULL) {
        throw HandlerExc("MethodID not set. Probably set class was not set.");
    }

    va_list args;
    va_start(args, key);

    obj = env->NewObjectV(this->clazz, mid, args);

    va_end(args);

    this->obj = obj;
}

template <typename To> To CJ::call(std::string key, ...) {
    SignatureBase* sigSuper = this->getSignatureObj(key);
    Signature<To>* sigChild = dynamic_cast<Signature<To>*>(sigSuper);
    jmethodID mid = sigChild->mid;
    To (CJ::*pCall) (jmethodID, va_list);
    pCall = sigChild->pCall;
    To jobj;

    va_list args;
    va_start(args, key);

    jobj = (this->*pCall) (mid, args);

    va_end(args);

    return jobj;
}

template <> void CJ::call(std::string key, ...) {
    SignatureBase* sigSuper = this->getSignatureObj(key);
    Signature<void>* sigChild = dynamic_cast<Signature<void>*>(sigSuper);
    jmethodID mid = sigChild->mid;
    void (CJ::*pCall) (jmethodID, va_list);
    pCall = sigChild->pCall;

    va_list args;
    va_start(args, key);

    (this->*pCall) (mid, args);

    va_end(args);
}

template jboolean CJ::call(std::string, ...);
template jbyte CJ::call(std::string, ...);
template jchar CJ::call(std::string, ...);
template jshort CJ::call(std::string, ...);
template jint CJ::call(std::string, ...);
template jlong CJ::call(std::string, ...);
template jfloat CJ::call(std::string, ...);
template jdouble CJ::call(std::string, ...);
template jobject CJ::call(std::string, ...);
template void CJ::call(std::string, ...);

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
 ** Converter implementation
 **/
Converter::Converter(): ConverterBase() { this->init(); }
Converter::~Converter() { }

void Converter::initUTIL() {
    UTIL.setClass("cjay/converter/Util");
}

void Converter::initARRAYLIST() {
    ARRAYLIST.setClass("java/util/ArrayList");
}

void Converter::initSET() {
    SET.setClass("java/util/Set");
}

void Converter::initCOLLECTION() {
    COLLECTION.setClass("java/util/Collection");
}

void Converter::initMAP() {
    MAP.setClass("java/util/Map");
}

void Converter::initNUMBER() {
    NUMBER.setClass("java/lang/Number");
}

void Converter::initBOOLEAN() {
    BOOLEAN.setClass("java/lang/Boolean");
}

void Converter::initBYTE() {
    BYTE.setClass("java/lang/Byte");
}

void Converter::initSHORT() {
    SHORT.setClass("java/lang/Short");
}

void Converter::initLONG() {
    LONG.setClass("java/lang/Long");
}

void Converter::initINTEGER() {
    INTEGER.setClass("java/lang/Integer");
}

void Converter::initFLOAT() {
    FLOAT.setClass("java/lang/Float");
}

void Converter::initDOUBLE() {
    DOUBLE.setClass("java/lang/Double");
}

void Converter::initCHARACTER() {
    CHARACTER.setClass("java/lang/Character");
}

void Converter::init() {
    this->initUTIL();

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

template <> bool Converter::c_cast(jobject jobj) {
    return (bool) this->c_cast<jboolean>(jobj);
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

int Converter::sizeMap(jobject jobj) {
    VM::SignatureBase* sig = MAP.getSignatureObj("size");

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
template std::vector<jboolean> Converter::c_cast_vector(jobject, int);
template std::vector<bool> Converter::c_cast_vector(jobject, int);
template std::vector<jbyte> Converter::c_cast_vector(jobject, int);
template std::vector<jchar> Converter::c_cast_vector(jobject, int);
template std::vector<jobject> Converter::c_cast_vector(jobject, int);
template std::vector<std::string> Converter::c_cast_vector(jobject, int);

template std::vector<jint> Converter::c_cast_vector(jobject);
template std::vector<jshort> Converter::c_cast_vector(jobject);
template std::vector<jlong> Converter::c_cast_vector(jobject);
template std::vector<jfloat> Converter::c_cast_vector(jobject);
template std::vector<jdouble> Converter::c_cast_vector(jobject);
template std::vector<jboolean> Converter::c_cast_vector(jobject);
template std::vector<bool> Converter::c_cast_vector(jobject);
template std::vector<jbyte> Converter::c_cast_vector(jobject);
template std::vector<jchar> Converter::c_cast_vector(jobject);
template std::vector<jobject> Converter::c_cast_vector(jobject);
template std::vector<std::string> Converter::c_cast_vector(jobject);

jobject Converter::getKeysOfMap(jobject jmap) {
    jmethodID mid = UTIL.getSignatureObj("FromMapToArrayListOfKeys")->mid;
    jobject arrayListOfKeys = env->CallStaticObjectMethod(UTIL.getClass(), mid, jmap);

    return arrayListOfKeys;
}

jobject Converter::getValuesOfMap(jobject jmap) {
    jmethodID mid = UTIL.getSignatureObj("FromMapToArrayListOfValues")->mid;
    jobject arrayListOfValues = env->CallStaticObjectMethod(UTIL.getClass(), mid, jmap);

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

/**
 ** Handler implementation
 **/
Handler::Handler(std::string className) {
    this->className = className;
    this->init();
}

Handler::~Handler() { }

void Handler::init() {
    hdl.setClass(this->className);
}

} /* namespace VM */
