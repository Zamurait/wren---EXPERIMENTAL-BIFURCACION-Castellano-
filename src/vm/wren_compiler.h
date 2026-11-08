#ifndef wren_compiler_h
#define wren_compiler_h

#include "wren.h"
#include "wren_value.h"

typedef struct sCompiler Compiler;

// Este módulo define el compilador de Wren. Toma una cadena de código fuente
// y la lexa, analiza y compila. Wren usa un compilador de una sola pasada. No
// construye un AST real durante el análisis y luego lo consume para generar
// código. En su lugar, el analizador emite directamente bytecode.
//
// Esto impone algunas restricciones sobre la gramática y la semántica del
// lenguaje. Cosas como referencias hacia adelante y búsqueda arbitraria son mucho
// más difíciles. Sin embargo, a cambio obtenemos mucho.
//
// La implementación es mucho más simple porque no necesitamos definir un
// montón de estructuras de datos AST. Más aún, no tenemos que lidiar con la
// gestión de memoria de los objetos AST. El compilador casi no hace asignación
// dinámica mientras se ejecuta.
//
// La compilación también es más rápida porque no creamos un montón de
// estructuras temporales y las destruimos después de generar el código.

// Compila [source], una cadena de código fuente Wren ubicada en [module], a un
// [ObjFn] que ejecutará ese código cuando se invoque. Devuelve `NULL` si el
// código fuente contiene errores de sintaxis.
//
// Si [isExpression] es `true`, [source] debe ser una única expresión y este
// compilador la convierte en una función que evalúa y devuelve esa expresión.
// En caso contrario, [source] debe ser una serie de declaraciones de nivel
// superior.
//
// Si [printErrors] es `true`, cualquier error de compilación se muestra en
// stderr. En caso contrario, se descarta silenciosamente.
ObjFn* wrenCompile(WrenVM* vm, ObjModule* module, const char* source,
                    bool isExpression, bool printErrors);

// Cuando se define una clase, su superclase no se conoce hasta el tiempo de
// ejecución, ya que las definiciones de clase son solo declaraciones
// imperativas. La mayor parte del bytecode para un método no importa, pero hay
// dos lugares donde sí:
//
//   - Para cargar o almacenar un campo, necesitamos saber el índice del campo
//     en la matriz de campos de la instancia. Debemos ajustarlo para que los
//     campos de subclases queden después de los de superclases, y no sabemos
//     esto hasta que se conoce la superclase.
//
//   - Las llamadas a superclase necesitan saber a qué superclase despachar.
//
// Podríamos manejar esto dinámicamente, pero eso añade sobrecarga. En su lugar,
// cuando un método se enlaza, recorremos el bytecode de la función y lo
// corregimos.
void wrenBindMethodCode(ObjClass* classObj, ObjFn* fn);

// Recorre todos los objetos asignados en el heap que estén en uso por
// [compiler] (y todos sus padres) para que no sean recogidos por el GC.
void wrenMarkCompiler(WrenVM* vm, Compiler* compiler);

#endif
