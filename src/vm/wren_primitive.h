#ifndef wren_primitive_h
#define wren_primitive_h

#include "wren_vm.h"

// Enlaza un método primitivo llamado [name] (en Wren) implementado usando la
// función C [fn] a `ObjClass` [cls].
#define PRIMITIVE(cls, name, function)                                         \
    do                                                                         \
    {                                                                          \
      int symbol = wrenSymbolTableEnsure(vm,                                   \
          &vm->methodNames, name, strlen(name));                               \
      Method method;                                                           \
      method.type = METHOD_PRIMITIVE;                                          \
      method.as.primitive = prim_##function;                                   \
      wrenBindMethod(vm, cls, symbol, method);                                 \
    } while (false)

// Enlaza un método primitivo llamado [name] (en Wren) implementado usando la
// función C [fn] a `ObjClass` [cls], pero como llamada FN.
#define FUNCTION_CALL(cls, name, function)                                     \
    do                                                                         \
    {                                                                          \
      int symbol = wrenSymbolTableEnsure(vm,                                   \
          &vm->methodNames, name, strlen(name));                               \
      Method method;                                                           \
      method.type = METHOD_FUNCTION_CALL;                                      \
      method.as.primitive = prim_##function;                                   \
      wrenBindMethod(vm, cls, symbol, method);                                 \
    } while (false)

// Define un método primitivo cuyo nombre de función C es [name]. Esto abstrae
// la firma real del tipo de una función primitiva y hace más claro qué
// funciones C se invocan como primitivas.
#define DEF_PRIMITIVE(name)                                                    \
    static bool prim_##name(WrenVM* vm, Value* args)

#define RETURN_VAL(value)                                                      \
    do                                                                         \
    {                                                                          \
      args[0] = value;                                                         \
      return true;                                                             \
    } while (false)

#define RETURN_OBJ(obj)     RETURN_VAL(OBJ_VAL(obj))
#define RETURN_BOOL(value)  RETURN_VAL(BOOL_VAL(value))
#define RETURN_FALSE        RETURN_VAL(FALSE_VAL)
#define RETURN_NULL         RETURN_VAL(NULL_VAL)
#define RETURN_NUM(value)   RETURN_VAL(NUM_VAL(value))
#define RETURN_TRUE         RETURN_VAL(TRUE_VAL)

#define RETURN_ERROR(msg)                                                      \
    do                                                                         \
    {                                                                          \
      vm->fiber->error = wrenNewStringLength(vm, msg, sizeof(msg) - 1);        \
      return false;                                                            \
    } while (false)

#define RETURN_ERROR_FMT(...)                                                  \
    do                                                                         \
    {                                                                          \
      vm->fiber->error = wrenStringFormat(vm, __VA_ARGS__);                    \
      return false;                                                            \
    } while (false)

// Valida que el [arg] dado sea una función. Devuelve true si lo es. Si no, \
// informa de un error y devuelve false.
bool validateFn(WrenVM* vm, Value arg, const char* argName);

// Valida que el [arg] dado sea un Num. Devuelve true si lo es. Si no,
// informa de un error y devuelve false.
bool validateNum(WrenVM* vm, Value arg, const char* argName);

// Valida que [value] sea un entero. Devuelve true si lo es. Si no, informa de
// un error y devuelve false.
bool validateIntValue(WrenVM* vm, double value, const char* argName);

// Valida que el [arg] dado sea un entero. Devuelve true si lo es. Si no,
// informa de un error y devuelve false.
bool validateInt(WrenVM* vm, Value arg, const char* argName);

// Valida que [arg] sea un objeto válido para usar como clave de mapa. Devuelve
// true si lo es. Si no, informa de un error y devuelve false.
bool validateKey(WrenVM* vm, Value arg);

// Valida que el argumento en [argIndex] sea un entero dentro de `[0, count)`.
// También permite índices negativos que cuentan hacia atrás desde el final.
// Devuelve el valor positivo válido. Si es inválido, informa de un error y
// devuelve `UINT32_MAX`.
uint32_t validateIndex(WrenVM* vm, Value arg, uint32_t count,
                        const char* argName);

// Valida que el [arg] dado sea un String. Devuelve true si lo es. Si no,
// informa de un error y devuelve false.
bool validateString(WrenVM* vm, Value arg, const char* argName);

// Dado un [range] y la [length] del objeto sobre el que se opera, determina la
// serie de elementos que debe elegirse del objeto subyacente.
// Maneja rangos que cuentan desde el final así como rangos negativos.
//
// Devuelve el índice desde el que debe empezar el rango o `UINT32_MAX` si el
// rango es inválido. Después de llamar, [length] se actualizará con el número de
// elementos de la secuencia resultante. [step] indicará la dirección del rango:
// `1` si el rango aumenta desde el índice inicial o `-1` si disminuye.
uint32_t calculateRange(WrenVM* vm, ObjRange* range, uint32_t* length,
                        int* step);

#endif
