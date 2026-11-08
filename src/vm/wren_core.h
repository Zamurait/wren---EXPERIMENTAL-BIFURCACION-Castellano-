#ifndef wren_core_h
#define wren_core_h

#include "wren_vm.h"

// Este módulo define las clases integradas y sus métodos primitivos que se
// implementan directamente en código C. Algunos lenguajes intentan implementar
// tanto como sea posible el módulo principal en el lenguaje principal en lugar
// del lenguaje de host.
//
// Con Wren, intentamos hacer tanto como sea posible en C. Los métodos
// primitivos son siempre más rápidos que el código escrito en Wren, y reducen
// el tiempo de inicio porque no tenemos que analizar, compilar y ejecutar código
// Wren.
//
// Hay una limitación, sin embargo. Los métodos escritos en C no pueden llamar a
// los de Wren. Solo pueden estar en la parte superior de la pila de llamadas y
// devolver de inmediato. Esto hace difícil que los métodos primitivos dependan
// de comportamiento polimórfico. Por ejemplo, `System.print` debería llamar a
// `toString` sobre su argumento, incluyendo los métodos `toString`
// definidos por el usuario en clases definidas por el usuario.

void wrenInitializeCore(WrenVM* vm);

#endif
