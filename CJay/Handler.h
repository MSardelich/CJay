/*
 * Handler.h
 *
 *  Created on: May 30, 2014
 *      Author: msn
 */

#ifndef HANDLER_H_
#define HANDLER_H_
#endif /* HANDLER_H_ */

#define callClassConstructor(...) callClassConstructor_(1, __VA_ARGS__)

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <cstdarg>

#include <jni.h>

using namespace std;

namespace VM {

extern JNIEnv* env_;
extern JavaVM* jvm_;

class SignatureBase {
public:
	static string VOID_DESCRIPTION_ENDING;
	string descriptor;
    bool isStatic;
    bool isVoid;
    jmethodID mid;
    //template<typename T> virtual T callMethod(JNIEnv*, jclass*, jobject*, ...) = 0;
    virtual jobject callMethod(JNIEnv*, jclass, jobject, va_list args) =0;

	SignatureBase(string, bool, bool);
    SignatureBase();
    virtual ~SignatureBase();
};

class VoidSignature : public SignatureBase {
public:
	VoidSignature(string, bool, bool);
	VoidSignature();
    virtual ~VoidSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

class StaticVoidSignature : public SignatureBase {
public:
	StaticVoidSignature(string, bool, bool);
	StaticVoidSignature();
    virtual ~StaticVoidSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

class ObjSignature : public SignatureBase {
public:
	ObjSignature(string, bool, bool);
	ObjSignature();
    virtual ~ObjSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

class StaticObjSignature : public SignatureBase {
public:
	StaticObjSignature(string, bool, bool);
	StaticObjSignature();
    virtual ~StaticObjSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

typedef map<string, VM::SignatureBase*> t_signature;

class Handler {
protected:
    t_signature m;

    string className;
    jclass clazz;
    jobject obj;

    JNIEnv* env;
    JavaVM* jvm;
public:
    static string CONTRUCTOR_METHOD_NAME;
    static jint JNI_VERSION;
    void setSignature(string, string, bool);
    void printSignatures();
    jclass getClass();
    jobject getObj();
    t_signature getMap();
    string getDescriptor(string);
    int getSizeSignatures();
    void createVM(vector<string>);
    void destroyVM();
    void setClass(string);
    VM::SignatureBase* getSignatureObj(string);
    void callClassConstructor_(int, ...);
    jobject callMethod(string, ...);
    JNIEnv* getEnv();
    Handler();
    virtual ~Handler();
};


class ConverterBase {
protected:
    Handler CONVERTER;
    JNIEnv* env;
    JavaVM* jvm;
public:
    virtual void initConverter() = 0;
    ConverterBase();
    virtual ~ConverterBase();
};

typedef std::vector<jobject> t_vec_obj;

class Converter : public ConverterBase {
public:
    void initConverter();
    void jString(string, jobject*);
    void jInt(int, jint*);
    void jFloat(float, jfloat*);
    void jDouble(double, jdouble*);
    int szVec(jobject);
    t_vec_obj toVec(jobject);
    string toString(jobject);
    void deleteRef(jobject);
    Converter();
    ~Converter();
};

class HandlerExc: public exception {
private:
    string msg;
public:
    HandlerExc(string m = "Uncategorized exception.") : msg(m) { }
    ~HandlerExc() throw() { }
    const char* what() const throw() { return msg.c_str(); }
};

} /* namespace VM */
