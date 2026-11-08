#ifndef wren_vm_h
#define wren_vm_h

#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_value.h"
#include "wren_utils.h"

// El número máximo de objetos temporales que pueden hacerse visibles para el GC
// a la vez.
#define WREN_MAX_TEMP_ROOTS 8

typedef enum
{
  #define OPCODE(name, _) CODE_##name,
  #include "wren_opcodes.h"
  #undef OPCODE
} Code;

// Un identificador para un valor, básicamente una lista enlazada de raíces GC
// adicionales.
//
// Ten en cuenta que incluso los valores que no son de heap pueden almacenarse
// aquí.
struct WrenHandle
{
  Value value;

  WrenHandle* prev;
  WrenHandle* next;
};

struct WrenVM
{
  ObjClass* boolClass;
  ObjClass* classClass;
  ObjClass* fiberClass;
  ObjClass* fnClass;
  ObjClass* listClass;
  ObjClass* mapClass;
  ObjClass* nullClass;
  ObjClass* numClass;
  ObjClass* objectClass;
  ObjClass* rangeClass;
  ObjClass* stringClass;

  // La fibra que está ejecutándose actualmente.
  ObjFiber* fiber;

  // Los módulos cargados. Cada clave es un ObjString (excepto para el módulo
  // principal, cuya clave es null) con el nombre del módulo y el valor es el
  // ObjModule del módulo.
  ObjMap* modules;
  
  // El módulo importado más recientemente. Más concretamente, el módulo cuyo
  // código terminó de ejecutarse más recientemente.
  //
  // No se trata como raíz del GC porque el módulo ya está en [modules].
  ObjModule* lastModule;

  // Datos de gestión de memoria:

  // El número de bytes que se sabe que están actualmente asignados. Incluye toda
  // la memoria que se ha demostrado que sigue viva tras la última recolección,
  // así como los nuevos bytes asignados desde entonces. No incluye los objetos
  // que fueron liberados desde la última recolección.
  size_t bytesAllocated;

  // El número total de bytes asignados que disparará la siguiente recolección.
  size_t nextGC;

  // El primer objeto en la lista enlazada de todos los objetos actualmente
  // asignados.
  Obj* first;

  // El «conjunto gris» del recolector de basura. Es la pila de objetos aún no
  // procesados mientras se realiza una pasada de recolección.
  Obj** gray;
  int grayCount;
  int grayCapacity;

  // La lista de raíces temporales. Es para objetos temporales o nuevos que no
  // son accesibles de otra forma pero que no deben ser recogidos.
  //
  // Están organizadas como una pila de punteros almacenados en este array. Esto
  // implica que las raíces temporales necesitan semántica de pila: solo la más
  // recientemente empujada puede liberarse.
  Obj* tempRoots[WREN_MAX_TEMP_ROOTS];

  int numTempRoots;
  
  // Puntero al primer nodo de la lista enlazada de identificadores activos o
  // NULL si no hay ninguno.
  WrenHandle* handles;
  
  // Puntero al fondo del rango de ranuras de la pila disponibles para usar desde
  // la API de C. Durante un método externo, esto estará en la pila de la fibra
  // que está ejecutando un método.
  //
  // Si no se está en un método externo, esto es inicialmente NULL. Si el usuario
  // solicita ranuras llamando a wrenEnsureSlots(), se crea una pila y esto se
  // inicializa.
  Value* apiStack;

  WrenConfiguration config;
  
  // Datos del compilador y depurador:

  // El compilador que está compilando código actualmente. Esto se usa para que
  // los objetos asignados en heap utilizados por el compilador puedan ser
  // encontrados si se dispara un GC en mitad de la compilación.
  Compiler* compiler;

  // Existe una única tabla global de símbolos para todos los nombres de método
  // en todas las clases. Las llamadas a métodos se despachan directamente por
  // índice en esta tabla.
  SymbolTable methodNames;
};

// Una función de asignación genérica que maneja toda la gestión explícita de
// memoria. Se usa así:
//
// - Para asignar memoria nueva, [memory] es NULL y [oldSize] es cero. Debe
//   devolver la memoria asignada o NULL si falla.
//
// - Para intentar ampliar una asignación existente, [memory] es la memoria,
//   [oldSize] es su tamaño anterior y [newSize] es el tamaño deseado. Debe
//   devolver [memory] si pudo ampliar la memoria in situ, o un puntero nuevo si
//   tuvo que moverla.
//
// - Para reducir memoria, [memory], [oldSize] y [newSize] son los mismos que
//   arriba, pero siempre devolverá [memory].
//
// - Para liberar memoria, [memory] será la memoria a liberar y [newSize] y
//   [oldSize] serán cero. Debe devolver NULL.
void* wrenReallocate(WrenVM* vm, void* memory, size_t oldSize, size_t newSize);

// Invoca el finalizador para el objeto extranjero referenciado por [foreign].
void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign);

// Crea un nuevo [WrenHandle] para [value].
WrenHandle* wrenMakeHandle(WrenVM* vm, Value value);

// Compila [source] en el contexto de [module] y lo envuelve en una fibra que
// puede ejecutarlo.
//
// Devuelve NULL si se produjo un error de compilación.
ObjClosure* wrenCompileSource(WrenVM* vm, const char* module,
                              const char* source, bool isExpression,
                              bool printErrors);

// Busca una variable de un módulo previamente cargado.
//
// Aborta la fibra actual si el módulo o la variable no se pudo encontrar.
Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName);

// Devuelve el valor de la variable de nivel superior llamada [name] en el
// módulo principal.
Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name);

// Añade una nueva variable implícita de nivel superior llamada [name] a [module]
// basada en un sitio de uso que aparece en [line].
//
// No comprueba si ya existe una variable con ese nombre. Devuelve el símbolo
// para la nueva variable o -2 si hay demasiadas variables definidas.
int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,
                        size_t length, int line);

// Añade una nueva variable de nivel superior llamada [name] a [module], y
// opcionalmente rellena line con la línea del primer uso implícito (line puede
// ser NULL).
//
// Devuelve el símbolo, -1 si el nombre dado ya está definido, o -2 si hay
// demasiadas variables definidas. Devuelve -3 si es una variable de nivel
// superior en minúsculas (nombre local) que se usó antes de definirse.
int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name,
                        size_t length, Value value, int* line);

// Empuja [closure] a la pila de llamadas de [fiber] para invocarla. Espera que
// haya [numArgs] argumentos (incluido el receptor) ya en la parte superior de la
// pila.
static inline void wrenCallFunction(WrenVM* vm, ObjFiber* fiber,
                                     ObjClosure* closure, int numArgs)
{
  // Aumenta el array de marcos de llamada si hace falta.
  if (fiber->numFrames + 1 > fiber->frameCapacity)
  {
    int max = fiber->frameCapacity * 2;
    fiber->frames = (CallFrame*)wrenReallocate(vm, fiber->frames,
        sizeof(CallFrame) * fiber->frameCapacity, sizeof(CallFrame) * max);
    fiber->frameCapacity = max;
  }
  
  // Aumenta la pila si hace falta.
  int stackSize = (int)(fiber->stackTop - fiber->stack);
  int needed = stackSize + closure->fn->maxSlots;
  wrenEnsureStack(vm, fiber, needed);
  
  wrenAppendCallFrame(vm, fiber, closure, fiber->stackTop - numArgs);
}

// Marca [obj] como raíz del GC para que no sea recogido.
void wrenPushRoot(WrenVM* vm, Obj* obj);

// Elimina la raíz temporal empujada más recientemente.
void wrenPopRoot(WrenVM* vm);

// Devuelve la clase de [value].
//
// Se define aquí en lugar de en wren_value.h porque es crítico que esté
// embeedado. Eso significa que debe definirse en el encabezado, pero el
// encabezado wren_value.h no tiene una definición completa de WrenVM aún.
static inline ObjClass* wrenGetClassInline(WrenVM* vm, Value value)
{
  if (IS_NUM(value)) return vm->numClass;
  if (IS_OBJ(value)) return AS_OBJ(value)->classObj;

#if WREN_NAN_TAGGING
  switch (GET_TAG(value))
  {
    case TAG_FALSE:     return vm->boolClass; break;
    case TAG_NAN:       return vm->numClass; break;
    case TAG_NULL:      return vm->nullClass; break;
    case TAG_TRUE:      return vm->boolClass; break;
    case TAG_UNDEFINED: UNREACHABLE();
  }
#else
  switch (value.type)
  {
    case VAL_FALSE:     return vm->boolClass;
    case VAL_NULL:      return vm->nullClass;
    case VAL_NUM:       return vm->numClass;
    case VAL_TRUE:      return vm->boolClass;
    case VAL_OBJ:       return AS_OBJ(value)->classObj;
    case VAL_UNDEFINED: UNREACHABLE();
  }
#endif

  UNREACHABLE();
  return NULL;
}

// Devuelve `true` si [name] es un nombre de variable local (empieza por una
// letra minúscula).
static inline bool wrenIsLocalName(const char* name)
{
  return name[0] >= 'a' && name[0] <= 'z';
}

static inline bool wrenIsFalsyValue(Value value)
{
  return IS_FALSE(value) || IS_NULL(value);
}

#endif
