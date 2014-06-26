/*
 * CJ.h
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

enum class RV {
    Z, // jboolean
    B, // jbyte
    C, // jchar
    S, // jshort
    I, // jint
    J, // jlong
    F, // jfloat
    D, // jdouble
    L, // jobject
    VV // VOID
};

extern JNIEnv* env;
extern JavaVM* jvm;

typedef long clong;

class SignatureBase {
public:
	std::string descriptor;
    bool isStatic;
    RV rv;
    jmethodID mid;
    SignatureBase(std::string, bool, const char*);
    SignatureBase();
    virtual ~SignatureBase();
};

typedef std::map<std::string, VM::SignatureBase*> signature_t;

class CJ {
protected:
    signature_t m;

    std::string className;

    jclass clazz;
    jobject obj;
public:
    static jint JNI_VERSION;
    void setSignature(std::string, std::string, bool);
    void printSignatures();
    jclass getClass();
    jobject getObj();
    signature_t getMap();
    std::string getDescriptor(std::string);
    jmethodID getMid(std::string);
    int getSizeSignatures();
    void createVM(std::vector<std::string>);
    void destroyVM();
    void setClass(std::string);
    VM::SignatureBase* getSignatureObj(std::string);
    void callClassConstructor_(int, ...);
    //jobject callMethod(std::string, ...);
    template <typename To> To call(std::string, ...);
    JNIEnv* getEnv();
    CJ();
    virtual ~CJ();
};

class ConverterBase {
protected:
    CJ ARRAYLIST;
    CJ MAP;
    CJ NUMBER;
    virtual void initARRAYLIST() = 0;
    virtual void initMAP() = 0;
    virtual void initNUMBER() = 0;
public:
    ConverterBase();
    virtual ~ConverterBase();
};

typedef std::vector<jobject> vec_jobj;

class Converter : public ConverterBase {
protected:
    void initARRAYLIST();
    void initMAP();
    void initNUMBER();
    void init();
public:
    template <typename To, typename From> To j_cast(From);
    template <typename To> To c_cast(jobject);
    template <typename To> std::vector<To> c_cast_number(jobject);
    template <typename To> std::vector<To> c_cast_vector(jobject);
    template <typename To> std::vector<To> c_cast_vector(jobject, int);
    int sizeVector(jobject);
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
