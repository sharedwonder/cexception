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

#include "cexception.h"

#include <stdlib.h>

#ifdef __STDC_VERSION__
    #if __STDC_VERSION__ >= 201103L
        #define C_11_OR_LATER
    #endif

    #if __STDC_VERSION__ >= 202311L
        #define C_23_OR_LATER
    #endif
#endif

#if defined(C_23_OR_LATER) && !defined(__STDC_NO_THREADS__)
    #define THREAD_LOCAL thread_local
#elif defined(C_11_OR_LATER) && !defined(__STDC_NO_THREADS__)
    #define THREAD_LOCAL _Thread_local
#elif defined(__GNUC__)
    #define THREAD_LOCAL __thread
#elif defined(_MSC_VER)
    #define THREAD_LOCAL __declspec(thread)
#else
    #define THREAD_LOCAL
#endif

#ifdef C_23_OR_LATER
    #define NORETURN [[noreturn]]
#elif defined(C_11_OR_LATER)
    #define NORETURN _Noreturn
#elif defined(__GNUC__)
    #define NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
    #define NORETURN __declspec(noreturn)
#else
    #define NORETURN
#endif

#ifndef C_23_OR_LATER
    #define nullptr NULL
    #if defined(__clang__) || defined(__GNUC__)
        #define assert_unreachable() __builtin_unreachable()
    #elif defined(_MSC_VER)
        #define assert_unreachable() __assume(false)
    #else
        #define assert_unreachable()
    #endif
#else
    #define assert_unreachable() unreachable()
#endif

const ExceptionType Exception = {"Exception", nullptr};

static THREAD_LOCAL TryCatchContext* tryCatchContextStack = nullptr;

static THREAD_LOCAL UncaughtExceptionHandler* uncaughtExceptionHandler = defaultUncaughtExceptionHandler;

NORETURN
void throwException(const ExceptionInstance* exception) {
    if (tryCatchContextStack == nullptr) {
        uncaughtExceptionHandler(exception);
        assert_unreachable();
    } else {
        tryCatchContextStack->exception = *exception;
        longjmp(tryCatchContextStack->env, tryCatchContextStack->status == TryCatchStatusTrying ?
            TryCatchStatusExceptionOccurred : TryCatchStatusExceptionRaised);
    }
}

void printExceptionInfo(const ExceptionInstance* exception, FILE* stream) {
    if (exception->file == nullptr) {
        fprintf(stream, "%s (unknown source)", exception->type->name);
    } else if (exception->line == 0) {
        fprintf(stream, "%s (file \"%s\")", exception->type->name, exception->file);
    } else {
        fprintf(stream, "%s (file \"%s\", line %u)", exception->type->name, exception->file, exception->line);
    }
    if (exception->message != nullptr) {
        fputs(": ", stream);
        fputs(exception->message, stream);
    }
    fputc('\n', stream);
}

bool exceptionInstanceOf(const ExceptionInstance* exception, const ExceptionType* type) {
    if (exception->type == type || type == &Exception) {
        return true;
    }

    const ExceptionType* parent = exception->type->parent;
    while (parent != nullptr) {
        if (parent == type) {
            return true;
        }
        parent = parent->parent;
    }
    return false;
}

void tryCatchContextStackPush(TryCatchContext* context) {
    context->link = tryCatchContextStack;
    tryCatchContextStack = context;
}

void tryCatchContextStackPop(void) {
    if (tryCatchContextStack->exception.alloc) {
        free((void*) tryCatchContextStack->exception.message);
    }
    tryCatchContextStack = tryCatchContextStack->link;
}

TryCatchContext* tryCatchContext(void) {
    return tryCatchContextStack;
}

UncaughtExceptionHandler* getUncaughtExceptionHandler(void) {
    return uncaughtExceptionHandler;
}

void setUncaughtExceptionHandler(UncaughtExceptionHandler* handler) {
    uncaughtExceptionHandler = handler;
}

NORETURN
void defaultUncaughtExceptionHandler(const ExceptionInstance* exception) {
    printExceptionInfo(exception, stderr);
    abort();
}
