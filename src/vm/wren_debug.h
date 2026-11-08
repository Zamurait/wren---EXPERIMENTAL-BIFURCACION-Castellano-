#ifndef wren_debug_h
#define wren_debug_h

#include "wren_value.h"
#include "wren_vm.h"

// Imprime el seguimiento de pila para la fibra actual.
//
// Se usa cuando una fibra lanza un error de ejecución que no se captura.
void wrenDebugPrintStackTrace(WrenVM* vm);

// Las funciones "dump" se usan para depurar Wren mismo. Las rutas normales del
// código no las llamarán salvo que se habilite alguna de las varias banderas
// DEBUG_.

// Imprime una representación de [value] en stdout.
void wrenDumpValue(Value value);

// Imprime una representación del bytecode para [fn] en la instrucción [i].
int wrenDumpInstruction(WrenVM* vm, ObjFn* fn, int i);

// Imprime el código desensamblado para [fn] en stdout.
void wrenDumpCode(WrenVM* vm, ObjFn* fn);

// Imprime el contenido de la pila actual para [fiber] en stdout.
void wrenDumpStack(ObjFiber* fiber);

#endif
