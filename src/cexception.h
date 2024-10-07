/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#ifndef CEXCEPTION_H
#define CEXCEPTION_H

#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #if __STDC_VERSION__ < 202311L
        #include <stdbool.h>
    #endif
#elif !defined(__cplusplus)
    #define bool unsigned char
    #define true 1
    #define false 0
#endif

#define TRY \
    { \
        TryCatchContext _tryCatchContext; \
        tryCatchContextStackPush(&_tryCatchContext); \
        _tryCatchContext.exception.alloc = false; \
        bool _tryCatchExecFinally = false; \
        _tryCatchContext.status = setjmp(_tryCatchContext.env); \
        if (_tryCatchContext.status == TryCatchStatusStart) { \
            _tryCatchContext.status = TryCatchStatusTrying;

#define CATCH(type, variable) \
        } else if (_tryCatchContext.status == TryCatchStatusExceptionOccurred && \
            exceptionInstanceOf(&_tryCatchContext.exception, &(type))) { \
            _tryCatchContext.status = TryCatchStatusCaughtException; \
            const ExceptionInstance* const (variable) = &_tryCatchContext.exception;

#define NOEXCEP \
        } \
        if (_tryCatchContext.status == TryCatchStatusTrying) { \
            _tryCatchContext.status = TryCatchStatusNoException; \
        } \
        if (_tryCatchContext.status == TryCatchStatusNoException) {

#define FINALLY \
        } \
        if (_tryCatchContext.status == TryCatchStatusTrying) { \
            _tryCatchContext.status = TryCatchStatusNoException; \
        } \
        if (!_tryCatchExecFinally) { \
            _tryCatchExecFinally = true;

#define TRY_END \
        } \
        tryCatchContextStackPop(); \
        if (_tryCatchContext.status == TryCatchStatusInterrupted) { \
            longjmp(_tryCatchContext.env, 1); \
        } \
        if (_tryCatchContext.status == TryCatchStatusExceptionOccurred || \
            _tryCatchContext.status == TryCatchStatusExceptionRaised) { \
            throwException(&_tryCatchContext.exception); \
        } \
    } \
    (void) 0

#define INT_TC_WITH(statement) \
    { \
        if (_tryCatchExecFinally) { \
            tryCatchContextStackPop(); \
            statement; \
        } \
        { \
            jmp_buf _tryCatchContextEnv; \
            memcpy(_tryCatchContextEnv, _tryCatchContext.env, sizeof(jmp_buf)); \
            if (setjmp(_tryCatchContext.env) == 0) { \
                longjmp(_tryCatchContextEnv, TryCatchStatusInterrupted); \
            } \
        } \
        statement; \
    } \
    (void) 0

#define BREAK_TC_WITH(statement) \
    { \
        (void) _tryCatchContext; \
        tryCatchContextStackPop(); \
        statement; \
    } \
    (void) 0

#define TC_RETURN(value) INT_TC_WITH(return (value))

#define TC_RETURN_NOW(value) BREAK_TC_WITH(return (value))

#define TC_BREAK INT_TC_WITH(break)

#define TC_BREAK_NOW BREAK_TC_WITH(break)

#define TC_CONTINUE INT_TC_WITH(continue)

#define TC_CONTINUE_NOW BREAK_TC_WITH(continue)

#define TC_GOTO(label) INT_TC_WITH(goto label)

#define TC_GOTO_NOW(label) BREAK_TC_WITH(goto label)

#define THROW_NEW(type, message) \
    { \
        CREATE_EXCEPTION((type), _exception, (message)); \
        THROW(_exception); \
    } \
    (void) 0

#define THROW_NEW_A(type, message) \
    { \
        CREATE_EXCEPTION_A((type), _exception, (message)); \
        THROW(_exception); \
    } \
    (void) 0

#define THROW(exception) \
    throwException(exception)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #ifdef CEXCEPTION_DISABLE_FILE_INFO
        #define CREATE_EXCEPTION(type, variable, message) \
            const ExceptionInstance* (variable) = &((ExceptionInstance) {&(type), (message), false, NULL, 0})

        #define CREATE_EXCEPTION_A(type, variable, message) \
            const ExceptionInstance* (variable) = &((ExceptionInstance) {&(type), (message), true, NULL, 0})
    #else
        #ifdef CEXCEPTION_DISABLE_LINE_INFO
            #define CREATE_EXCEPTION(type, variable, message) \
                const ExceptionInstance* (variable) = &((ExceptionInstance) {&(type), (message), false, __FILE__, 0})

            #define CREATE_EXCEPTION_A(type, variable, message) \
                const ExceptionInstance* (variable) = &((ExceptionInstance) {&(type), (message), true, __FILE__, 0})
        #else
            #define CREATE_EXCEPTION(type, variable, message) \
                const ExceptionInstance* (variable) =  &((ExceptionInstance) {&(type), (message), false, __FILE__, __LINE__})

            #define CREATE_EXCEPTION_A(type, variable, message) \
                const ExceptionInstance* (variable) =  &((ExceptionInstance) {&(type), (message), true, __FILE__, __LINE__})
        #endif
    #endif
#else
    #ifdef CEXCEPTION_DISABLE_FILE_INFO
        #define CREATE_EXCEPTION(type, variable, message) \
            const ExceptionInstance (variable)[] = {{&(type), (message), false, NULL, 0}}

        #define CREATE_EXCEPTION_A(type, variable, message) \
            const ExceptionInstance (variable)[] = {{&(type), (message), true, NULL, 0}}
    #else
        #ifdef CEXCEPTION_DISABLE_LINE_INFO
            #define CREATE_EXCEPTION(type, variable, message) \
                const ExceptionInstance (variable)[] = {{&(type), (message), false, __FILE__, 0}}

            #define CREATE_EXCEPTION_A(type, variable, message) \
                const ExceptionInstance (variable)[] = {{&(type), (message), true, __FILE__, 0}}
        #else
            #define CREATE_EXCEPTION(type, variable, message) \
                const ExceptionInstance (variable)[] = {{&(type), (message), false, __FILE__, __LINE__}}

            #define CREATE_EXCEPTION_A(type, variable, message) \
                const ExceptionInstance (variable)[] = {{&(type), (message), true, __FILE__, __LINE__}}
        #endif
    #endif
#endif

#define DECLARE_EXCEPTION(name, parnet) \
    extern const ExceptionType (name)

#define DEFINE_EXCEPTION(name, parent) \
    const ExceptionType (name) = {#name, &(parent)}

typedef struct ExceptionType {
    const char* const name;
    const struct ExceptionType* const parent;
} ExceptionType;

typedef struct ExceptionInstance {
    const ExceptionType* type;
    const char* message;
    bool alloc;
    const char* file;
    unsigned int line;
} ExceptionInstance;

typedef enum TryCatchStatus {
    TryCatchStatusStart,
    TryCatchStatusTrying,
    TryCatchStatusNoException,
    TryCatchStatusExceptionOccurred,
    TryCatchStatusCaughtException,
    TryCatchStatusExceptionRaised,
    TryCatchStatusInterrupted
} TryCatchStatus;

typedef struct TryCatchContext {
    TryCatchStatus status;
    jmp_buf env;
    ExceptionInstance exception;
    struct TryCatchContext* link;
} TryCatchContext;

typedef void UncaughtExceptionHandler(const ExceptionInstance*);

#ifdef CEXCEPTION_SHARED
    #if defined(_WIN32) || defined(__CYGWIN__)
        #ifdef CEXCEPTION_BUILDING
            #define CEXCEPTION_API __declspec(dllexport)
        #else
            #define CEXCEPTION_API __declspec(dllimport)
        #endif
    #elif defined(__GNUC__) && __GNUC__ >= 4
        #define CEXCEPTION_API __attribute__((visibility("default")))
    #else
        #define CEXCEPTION_API
    #endif
#else
    #define CEXCEPTION_API
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L || \
    (defined(__cplusplus) && (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L || __cplusplus >= 201103L))
    #define NORETURN_ [[noreturn]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define NORETURN_ _Noreturn
#elif defined(__GNUC__)
    #define NORETURN_ __attribute__((noreturn))
#elif defined(_MSC_VER)
    #define NORETURN_ __declspec(noreturn)
#else
    #define NORETURN_
#endif

#ifdef __cplusplus
extern "C" {
#endif

CEXCEPTION_API
extern const ExceptionType Exception;

NORETURN_ CEXCEPTION_API
void throwException(const ExceptionInstance* exception);

CEXCEPTION_API
void printExceptionInfo(const ExceptionInstance* exception, FILE* stream);

CEXCEPTION_API
bool exceptionInstanceOf(const ExceptionInstance* exception, const ExceptionType* type);

CEXCEPTION_API
void tryCatchContextStackPush(TryCatchContext* context);

CEXCEPTION_API
void tryCatchContextStackPop(void);

CEXCEPTION_API
TryCatchContext* tryCatchContext(void);

CEXCEPTION_API
UncaughtExceptionHandler* getUncaughtExceptionHandler(void);

CEXCEPTION_API
void setUncaughtExceptionHandler(UncaughtExceptionHandler* handler);

NORETURN_ CEXCEPTION_API
void defaultUncaughtExceptionHandler(const ExceptionInstance* exception);

#ifdef __cplusplus
}
#endif

#undef NORETURN_

#endif
