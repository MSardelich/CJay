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

namespace VM {

extern JNIEnv* env_;
extern JavaVM* jvm_;

typedef long clong;

class SignatureBase {
public:
	static std::string VOID_DESCRIPTION_ENDING;
	std::string descriptor;
    bool isStatic;
    bool isVoid;
    jmethodID mid;
    virtual jobject callMethod(JNIEnv*, jclass, jobject, va_list args) =0;
    SignatureBase(std::string, bool, bool);
    SignatureBase();
    virtual ~SignatureBase();
};

class VoidSignature : public SignatureBase {
public:
	VoidSignature(std::string, bool, bool);
	VoidSignature();
    virtual ~VoidSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

class StaticVoidSignature : public SignatureBase {
public:
	StaticVoidSignature(std::string, bool, bool);
	StaticVoidSignature();
    virtual ~StaticVoidSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

class ObjSignature : public SignatureBase {
public:
	ObjSignature(std::string, bool, bool);
	ObjSignature();
    virtual ~ObjSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

class StaticObjSignature : public SignatureBase {
public:
	StaticObjSignature(std::string, bool, bool);
	StaticObjSignature();
    virtual ~StaticObjSignature();

    jobject callMethod(JNIEnv*, jclass, jobject, va_list args);
};

typedef std::map<std::string, VM::SignatureBase*> t_signature;

class Handler {
protected:
    t_signature m;

    std::string className;

    jclass clazz;
    jobject obj;
    JNIEnv* env;
    JavaVM* jvm;
public:
    static std::string CONTRUCTOR_METHOD_NAME;
    static jint JNI_VERSION;
    void setSignature(std::string, std::string, bool);
    void printSignatures();
    jclass getClass();
    jobject getObj();
    t_signature getMap();
    std::string getDescriptor(std::string);
    jmethodID getMid(std::string);
    int getSizeSignatures();
    void createVM(std::vector<std::string>);
    void destroyVM();
    void setClass(std::string);
    VM::SignatureBase* getSignatureObj(std::string);
    void callClassConstructor_(int, ...);
    jobject callMethod(std::string, ...);
    JNIEnv* getEnv();
    Handler();
    virtual ~Handler();
};

class ConverterBase {
protected:
    Handler ARRAYLIST;
    Handler MAP;

    JNIEnv* env;
    JavaVM* jvm;
public:
    virtual void initConverter() = 0;
    ConverterBase();
    virtual ~ConverterBase();
};

typedef std::vector<jobject> vec_jobj;

class Converter : public ConverterBase {
public:
    void initConverter();
    template <typename To, typename From> To j_cast(From);
    //template <typename To, typename From> To c_cast(From);
    //void jString(std::string, jobject*);
    int szVec(jobject);
    vec_jobj toVecObject(jobject);
    template <typename To> std::vector<To> toVec(jobject);
    //std::string toString(jobject);
    void deleteRef(jobject);
    Converter();
    ~Converter();
};

class HandlerExc: public std::exception {
private:
    std::string msg;
public:
    HandlerExc(std::string m = "Uncategorized exception.") : msg(m) { }
    ~HandlerExc() throw() { }
    const char* what() const throw() { return msg.c_str(); }
};

} /* namespace VM */
