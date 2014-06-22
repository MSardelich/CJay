/*
 * Handler.cpp
 *
 *  Created on: May 30, 2014
 *      Author: msn
 */

#include "Handler.h"

namespace VM {

JNIEnv* env_ = NULL;
JavaVM* jvm_ = NULL;

// Signature Members
string SignatureBase::VOID_DESCRIPTION_ENDING = ")V";

SignatureBase::SignatureBase(string descriptor, bool isStatic, bool isVoid) :
		descriptor(descriptor), isStatic(isStatic), isVoid(isVoid), mid(NULL) { }

SignatureBase::SignatureBase() : descriptor(""), isStatic(true), isVoid(true), mid(NULL) {}
SignatureBase::~SignatureBase() { }

VoidSignature::VoidSignature(string descriptor, bool isStatic, bool isVoid) : SignatureBase(descriptor,isStatic, isVoid) { }
VoidSignature::VoidSignature() : SignatureBase() { }
VoidSignature::~VoidSignature() { }
jobject VoidSignature::callMethod(JNIEnv* env, jclass jclazz, jobject obj, va_list args) {
    jobject jobj;

    env->CallVoidMethodV(obj, this->mid, args);

    jobj = NULL;
    return jobj;
}

StaticVoidSignature::StaticVoidSignature(string descriptor, bool isStatic, bool isVoid) : SignatureBase(descriptor,isStatic, isVoid) { }
StaticVoidSignature::StaticVoidSignature() : SignatureBase() { }
StaticVoidSignature::~StaticVoidSignature() { }
jobject StaticVoidSignature::callMethod(JNIEnv* env, jclass jclazz, jobject obj, va_list args) {
    jobject jobj;

    env->CallStaticVoidMethodV(jclazz, this->mid, args);

    jobj = NULL;
    return jobj;
}

ObjSignature::ObjSignature(string descriptor, bool isStatic, bool isVoid) : SignatureBase(descriptor,isStatic, isVoid) { }
ObjSignature::ObjSignature() : SignatureBase() { }
ObjSignature::~ObjSignature() { }

jobject ObjSignature::callMethod(JNIEnv* env, jclass jclazz, jobject obj, va_list args) {
    jobject jobj;

    jobj = env->CallObjectMethodV(obj, this->mid, args);

    return jobj;
}

StaticObjSignature::StaticObjSignature(string descriptor, bool isStatic, bool isVoid) : SignatureBase(descriptor,isStatic, isVoid) { }
StaticObjSignature::StaticObjSignature() : SignatureBase() { }
StaticObjSignature::~StaticObjSignature() { }

jobject StaticObjSignature::callMethod(JNIEnv* env, jclass jclazz, jobject obj, va_list args) {
    jobject jobj;

    jobj = env->CallStaticObjectMethodV(jclazz, this->mid, args);

    va_end(args);

    return jobj;
}

// Handler Members
string Handler::CONTRUCTOR_METHOD_NAME = "<init>";

Handler::Handler() : clazz(NULL), obj(NULL), env(env_), jvm(jvm_) {
    this->m.clear();
}

Handler::~Handler() {
    delete jvm;
    delete env;
}

void Handler::setSignature(string key, string descriptor, bool isStatic) {
    string voidEnding(SignatureBase::VOID_DESCRIPTION_ENDING);
	SignatureBase* signature;
    bool isVoid = (0 == descriptor.compare(
			descriptor.length() - voidEnding.length(),
            voidEnding.length(),
            voidEnding
            ));
    if (isStatic) {
    	if (isVoid) {
    		signature = new StaticVoidSignature(descriptor, isStatic, isVoid);
    	} else {
    		signature = new StaticObjSignature(descriptor, isStatic, isVoid);
    	}
    } else {
    	if (isVoid) {
    		signature = new VoidSignature(descriptor, isStatic, isVoid);
    	} else {
    		signature = new ObjSignature(descriptor, isStatic, isVoid);
    	}
    }
	this->m.insert(t_signature::value_type(key, signature));
}

void Handler::printSignatures() {
    for(t_signature::iterator it = this->m.begin(); it != this->m.end(); ++it) {
        string key = it->first;
        cout <<
                "<" <<
                "Method:" << key <<
                ", Descriptor:" << this->m[key]->descriptor <<
                ", isStatic:" << this->m[key]->isStatic <<
                ", isVoid:" << this->m[key]->isVoid <<
                ">" <<
                endl;
    }
}

jclass Handler::getClass() {
    return this->clazz;
}

jobject Handler::getObj() {
    return this->obj;
}

t_signature Handler::getMap() {
    return this->m;
}

string Handler::getDescriptor(string key) {
    return this->getSignatureObj(key)->descriptor;
}

int Handler::getSizeSignatures() {
    return m.size();
}

void Handler::createVM(vector<string> vmOption) {
    if (this->env == NULL || this->jvm == NULL) {
        int nOptions = vmOption.size();

        JavaVMInitArgs vm_args;
        JavaVMOption* options = new JavaVMOption[nOptions + 1];

        for (vector<string>::iterator it = vmOption.begin(); it != vmOption.end(); ++it) {
            options[it-vmOption.begin()].optionString = (char*) it->c_str();
        }
        options[nOptions].optionString = (char*) "-Xcheck:jni"; //-Xnoclassgc -Xcheck:jni

        vm_args.version = this->JNI_VERSION;
        vm_args.nOptions = nOptions + 1;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = JNI_FALSE;
        int status = JNI_CreateJavaVM(&jvm_, (void**)&env_, &vm_args); // create only once with global variables
        if(status != JNI_OK) {
            delete options;
            throw HandlerExc("Unable to Launch JVM");
        }

        // assign obj values
        this->env = env_;
        this->jvm = jvm_;

        delete options; // clean memory leaks
    }
}

void Handler::destroyVM() {
    this->jvm->DestroyJavaVM();
}

void Handler::setClass(string className) {
    if (this->env == NULL || this->jvm == NULL) {
    	throw HandlerExc("JVM was not instantiated. Please call member createJVM.");
    }

	this->className = className;
    this->clazz = this->env->FindClass(className.c_str());
    if (this->clazz == NULL) {
        jthrowable exc = env->ExceptionOccurred();
        if (exc) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            throw HandlerExc("");
        }
    }

    VM::SignatureBase* obj;
    jmethodID mid;
    for (t_signature::iterator it = this->m.begin(); it != this->m.end(); ++it) {
        string key = it->first;
        obj = it->second;
        // assign values to be updated
        if (obj->isStatic) {
        	mid = this->env->GetStaticMethodID(this->clazz, key.c_str(), obj->descriptor.c_str());
        } else {
        	mid = this->env->GetMethodID(this->clazz, key.c_str(), obj->descriptor.c_str());
        }
        //mid = ( (this->env)->*ptrGetMethod )(this->clazz, key.c_str(), obj.descriptor.c_str());
        // check consistency
        if (mid == NULL) {
            jthrowable exc;
            exc = env->ExceptionOccurred();
            if (exc) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                throw HandlerExc("");
            }
        }
        // update map
        it->second->mid = mid;
    }

}

VM::SignatureBase* Handler::getSignatureObj(string key) {
    if ( this->m.find(key) == this->m.end() ) {
        throw HandlerExc("Key does not exit. Use setSignature member beforehand.");
    }
    VM::SignatureBase* obj = this->m[key];

    return obj;
}

void Handler::callClassConstructor_(int mangledVar, ...) {
    // Get Method Id (Constructor)
    VM::SignatureBase* signatureObj = this->getSignatureObj(this->CONTRUCTOR_METHOD_NAME);
    jmethodID mid = signatureObj->mid;
    jobject obj;

    if(mid == NULL) {
        throw HandlerExc("MethodID not set. Probably set class was not set.");
    }

    va_list args;
    va_start(args, mangledVar);

    obj = this->env->NewObjectV(this->clazz, mid, args);

    va_end(args);

    this->obj = obj;
}

jobject Handler::callMethod(string methodName, ...) {
    VM::SignatureBase* signatureObj = this->getSignatureObj(methodName);
    jobject jobj;

    va_list args;
    va_start(args, methodName);

    jobj = signatureObj->callMethod(this->env, this->clazz, this->obj, args);

    va_end(args);

    return jobj;
}

JNIEnv* Handler::getEnv() {
    return this->env;
}

// SuperClass ConverterBase Members
ConverterBase::ConverterBase() : env(env_), jvm(jvm_) {
    Handler convHandler;
    this->CONVERTER = convHandler;
}

ConverterBase::~ConverterBase() { }

/* SubClass Converter Members
*/
Converter::Converter(): ConverterBase() { this->initConverter(); }
Converter::~Converter() {
    delete env;
    delete jvm;
}

void Converter::initConverter() {
    CONVERTER.setSignature( string("toString"), string("()Ljava/lang/String;"), false );
    CONVERTER.setSignature( string("get"), string("(I)Ljava/lang/Object;"), false );
    CONVERTER.setSignature( string("size"), string("()I"), false );

    CONVERTER.setClass("java/util/ArrayList");
}

void Converter::jString(string str, jobject* jobj) {
    *jobj = this->env->NewStringUTF(str.c_str());
}

void Converter::jInt(int i, jint* jobj) {
    *jobj = (jint) i;
}

void Converter::jFloat(float i, jfloat* jobj) {
    *jobj = (jfloat) i;
}

void Converter::jDouble(double i, jdouble* jobj) {
    *jobj = (jdouble) i;
}

int Converter::szVec(jobject jobj) {
    string METHOD_NAME("size");
    VM::SignatureBase* signatureObj = CONVERTER.getSignatureObj(METHOD_NAME);
    jobject jobj_;

    jobj_ = signatureObj->callMethod(this->env, CONVERTER.getClass(), jobj, NULL);

    return (int) (jint) jobj_;
}

inline jobject WRAPPER_METHODV(JNIEnv* env, VM::SignatureBase* signatureObj, jclass jclazz, jobject jobj, ...) {
    jobject jresult;

    va_list args;
    va_start(args, jobj);

    jresult = signatureObj->callMethod(env, jclazz, jobj, args);

    va_end(args);

    return jresult;
}

t_vec_obj Converter::toVec(jobject jobj) {
    string METHOD_NAME("get");
    VM::SignatureBase* signatureObj = CONVERTER.getSignatureObj(METHOD_NAME);
    t_vec_obj cppVec;
    jobject elem;
    int size = this->szVec(jobj);

    for (int i = 0 ; i < size ; i++) {
        elem = WRAPPER_METHODV(this->env, signatureObj, CONVERTER.getClass(), jobj, (jint) i);
        cppVec.push_back(elem);
    }

    return cppVec;
}

string Converter::toString(jobject jobj) {
    return this->env->GetStringUTFChars((jstring) jobj, 0);
}

void Converter::deleteRef(jobject jobj) {
    this->env->DeleteLocalRef(jobj);
}

} /* namespace VM */
