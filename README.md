# CException

This repository provides an simple implementation of exception handling in the C language.

Released into the public domain, see [LICENSE](LICENSE) for more information.

## Notes

- Multithreading support depends on the build environment.
- Variables that are modified in TRY-CATCH blocks need to qualify with `volatile` because of compiler optimizations.
- Do not directly use statements that jump out of the TRY-CATCH block, use `INT_TC_WITH`/`BREAK_TC_WITH` wrap them or use the corresponding macro instead, otherwise the behavior is undefined.
- If a TRY-CATCH block does not terminate after executing a statement wrapped by `INT_TC_WITH`/`BREAK_TC_WITH`, the behavior is undefined.
- Throwing exceptions have limitations (from `longjmp`): if jump out the scope that contains any variable-length array, variably-modified type object or C++ automatic object with non-trivial destructor, the behavior is undefined.
- The default uncaught exception handler prints the exception information to the standard error stream and aborts the process.
- An uncaught exception handler cannot return, otherwise the behavior is undefined.
- Do not set the uncaught exception handler to a null pointer, otherwise the behavior is undefined.
- To disable line information in exceptions, define `CEXCEPTION_DISABLE_LINE_INFO` before including `cexception.h`.
- To disable file information in exceptions, define `CEXCEPTION_DISABLE_FILE_INFO` before including `cexception.h` (implies `CEXCEPTION_DISABLE_LINE_INFO`).

## All Identifiers

### Macros

#### TRY-CATCH Block

| Identifier | Parameters         | Description                                                                |
| ---------- | ------------------ | -------------------------------------------------------------------------- |
| `TRY`      | -                  | Represents the beginning of a TRY-CATCH block.                             |
| `CATCH`    | `(type, variable)` | Catches an exception of the specific type and its subtypes, matching once. |
| `NOEXCEP`  | -                  | Executes when no exception was thrown.                                     |
| `FINALLY`  | -                  | Always executes unless the TRY-CATCH block has terminated.                 |
| `TRY_END`  | -                  | Represents the end of a TRY-CATCH block.                                   |

#### Code Jumping

| Identifier        | Parameters        | Description                                                                                                  |
| ----------------- | ----------------- | ------------------------------------------------------------------------------------------------------------ |
| `INT_TC_WITH`     | `(statement)`     | Interrupts the TRY-CATCH block: terminates it with a statement after trying to execute the `FINALLY` clause. |
| `BREAK_TC_WITH`   | `(statement)`     | Terminates the TRY-CATCH block with a statement (without executing the `FINALLY` clause).                    |
| `TC_RETURN`       | `(value)`         | Interrupts the TRY-CATCH block with `return`.                                                                |
| `TC_RETURN_NOW`   | `(value)`         | Terminates the TRY-CATCH block with `return`.                                                                |
| `TC_BREAK`        | -                 | Interrupts the TRY-CATCH block with `break`.                                                                 |
| `TC_BREAK_NOW`    | -                 | Terminates the TRY-CATCH block with `break`.                                                                 |
| `TC_CONTINUE`     | -                 | Interrupts the TRY-CATCH block with `continue`.                                                              |
| `TC_CONTINUE_NOW` | -                 | Terminates the TRY-CATCH block with `continue`.                                                              |
| `TC_GOTO`         | `(label)`         | Interrupts the TRY-CATCH block with `goto`.                                                                  |
| `TC_GOTO_NOW`     | `(label)`         | Terminates the TRY-CATCH block with `goto`.                                                                  |
| `THROW_NEW`       | `(type, message)` | Creates a new exception instance (with a static message) and throws it.                                      |
| `THROW_NEW_A`     | `(type, message)` | Creates a new exception instance (with an allocated message) and throws it.                                  |
| `THROW`           | `(exception)`     | Throws an exception.                                                                                         |

#### Others

| Identifier                | Parameters                  | Description                                                                                                                           |
| ------------------------- | --------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- |
| `CREATE_EXCEPTION`        | `(type, variable, message)` | Defines a pointer that references (C99 and later) or an array that holds (C89/90) a new exception instance with a static message.     |
| `CREATE_EXCEPTION_A`      | `(type, variable, message)` | Defines a pointer that references (C99 and later) or an array that holds (C89/90) a new exception instance with an allocated message. |
| `DECLARE_EXCEPTION`       | `(name, parent)`            | Declares a new exception type (usually used in a header file).                                                                        |
| `DEFINE_EXCEPTION`        | `(name, parent)`            | Defines a declared exception type (usually used in a source file).                                                                    |

### Exception Types

- Actually constants of the type `ExceptionType`.

| Identifier   | Parent | Description                                                         |
| ------------ | ------ | ------------------------------------------------------------------- |
| `Exception`  | -      | Top level exception type, the parent of all other exceptions types. |

### Structure Types

- All structure types have a typedef with the same name.

#### ExceptionType

- The type of exception types.

| Member   | Type                         | Description                |
| -------- | ---------------------------- | -------------------------- |
| `name`   | `const char* const`          | The name of exception.     |
| `parent` | `const ExceptionType* const` | The parent exception type. |

#### ExceptionInstance

- The exception instance type.

| Member    | Type                   | Description                                                  |
| --------- | ---------------------- | ------------------------------------------------------------ |
| `type`    | `const ExceptionType*` | The exception type.                                          |
| `message` | `const char*`          | The message of the exception.                                |
| `alloc`   | `bool`                 | Whether the object referenced by the `message` is allocated. |
| `file`    | `const char*`          | The file where the exception created.                        |
| `line`    | `int`                  | The line where the exception created.                        |

#### TryCatchContext

- The TRY-CATCH context type.

| Member      | Type                | Description                                                    |
| ----------- | ------------------- | -------------------------------------------------------------- |
| `status`    | `TryCatchStatus`    | The status in the TRY-CATCH block.                             |
| `env`       | `jmp_buf`           | Stores the execution context.                                  |
| `exception` | `ExceptionInstance` | The exception instance if there is an exception occurred.      |
| `link`      | `TryCatchContext*`  | The previous TRY-CATCH context in the TRY-CATCH context stack. |

### Enumeration Types

- All enumeration types have a typedef with the same name.

#### TryCatchStatus

- Represents the statuses in TRY-CATCH blocks.

| Constant                          | Value | Description                                                               |
| --------------------------------- | ----- | ------------------------------------------------------------------------- |
| `TryCatchStatusStart`             | `0`   | Started executing a TRY-CATCH block.                                      |
| `TryCatchStatusTrying`            | `1`   | Executing the `TRY` clause.                                               |
| `TryCatchStatusNoException`       | `2`   | No exception occurred after executing the `TRY` clause.                   |
| `TryCatchStatusExceptionOccurred` | `3`   | An exception occurred in the `TRY` clause.                                |
| `TryCatchStatusCaughtException`   | `4`   | The exception has been caught.                                            |
| `TryCatchStatusExceptionRaised`   | `5`   | An exception occurred in the TRY-CATCH block but not in the `TRY` clause. |
| `TryCatchStatusInterrupted`       | `6`   | The TRY-CATCH block is interrupted.                                       |

### Typedefs

| Identifier                 | Type                              | Description                               |
| -------------------------- | --------------------------------- | ----------------------------------------- |
| `UncaughtExceptionHandler` | `void (const ExceptionInstance*)` | Represents an uncaught exception handler. |

### Functions

| Identifier                        | Signature                                                              | Attributes | Description                                                                      |
| --------------------------------- | ---------------------------------------------------------------------- | ---------- | -------------------------------------------------------------------------------- |
| `throwException`                  | `void (const ExceptionInstance* exception)`                            | `noreturn` | Throws an exception instance.                                                    |
| `printExceptionInfo`              | `void (const ExceptionInstance* exception, FILE* stream)`              | -          | Prints the exception information to the specified stream.                        |
| `exceptionInstanceOf`             | `bool (const ExceptionInstance* exception, const ExceptionType* type)` | -          | Determines the exception instance is an instance of the specific exception type. |
| `tryCatchContextStackPush`        | `void (TryCatchContext* context)`                                      | -          | Adds the TRY-CATCH context to the TRY-CATCH context stack.                       |
| `tryCatchContextStackPop`         | `void (void)`                                                          | -          | Removes the last TRY-CATCH context from the TRY-CATCH context stack.             |
| `tryCatchContext`                 | `TryCatchContext* (void)`                                              | -          | Gets the current TRY-CATCH context.                                              |
| `getUncaughtExceptionHandler`     | `UncaughtExceptionHandler* (void)`                                     | -          | Gets the uncaught exception handler (of the current thread).                     |
| `setUncaughtExceptionHandler`     | `void (UncaughtExceptionHandler* handler)`                             | -          | Sets the uncaught exception handler (of the current thread).                     |
| `defaultUncaughtExceptionHandler` | `void (const ExceptionInstance*)`                                      | `noreturn` | The default uncaught exception handler.                                          |

## Example

Source code:

```c
#include <stdio.h>
#include <cexception.h>

int main() {
    volatile int number = 1;
    printf("Number: %d\n", number);

    TRY {
        printf("TRY\n");
        number = 2;
        THROW_NEW(Exception, "exeption message");
        number = 3;
    } CATCH (Exception, exception) {
        printf("CATCH\n");
        PRINT_EXCEPTION_INFO(exception);
    } NOEXCEP {
        printf("NOEXCEP\n");
    } FINALLY {
        printf("FINALLY\n");
    } TRY_END;

    printf("Number: %d\n", number);

    return 0;
}
```

Output:

```
Number: 1
TRY
CATCH
Exception (file "example.c", line 11): exeption message
FINALLY
Number: 2
```
