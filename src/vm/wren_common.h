#ifndef wren_common_h
#define wren_common_h

// Este encabezado contiene macros y definiciones usadas en toda la
// implementación de Wren. En particular, contiene definiciones de
// "configuración" que controlan cómo funciona Wren. Algunas solo se usan al
// trabajar directamente en Wren mismo.
//
// Este encabezado no está pensado para incluirse en código fuera de Wren.

// Wren usa ampliamente los tipos enteros de C99 (uint16_t, etc.) junto con
// algunas constantes asociadas (UINT32_MAX, etc.). Las constantes no forman
// parte del estándar de C++, así que no se incluyen por defecto en compiladores
// de C++ al incluir <stdint> a menos que se defina __STDC_LIMIT_MACROS.
#define __STDC_LIMIT_MACROS
#include <stdint.h>

// Estas banderas te permiten controlar algunos detalles de la implementación
// del intérprete. Normalmente, equilibran portabilidad con velocidad. Por
// defecto, usan el comportamiento más eficiente.

// Si es true, Wren usa un double etiquetado con NaN para su representación
// principal de valores. En caso contrario, usa una estructura más grande y más
// convencional. La primera opción es significativamente más rápida y compacta.
// La segunda es útil para depuración y puede ser más portable.
//
// Por defecto, está activado.
#ifndef WREN_NAN_TAGGING
  #define WREN_NAN_TAGGING 1
#endif

// Si es true, el bucle intérprete de la VM usa gotos computados. Consulta esto
// para más información:
// http://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Labels-as-Values.html
// Activarlo acelera un poco el bucle principal de despacho, pero requiere apoyo
// del compilador.
// see https://bullno1.com/blog/switched-goto for alternative
// Por defecto, está activado en compiladores compatibles.
#ifndef WREN_COMPUTED_GOTO
  #if defined(_MSC_VER) && !defined(__clang__)
    // No hay gotos computados en Visual Studio.
    #define WREN_COMPUTED_GOTO 0
  #else
    #define WREN_COMPUTED_GOTO 1
  #endif
#endif

// La VM incluye varios módulos opcionales. Puedes elegir incluirlos o no. Por
// defecto, todos están disponibles. Para deshabilitar uno, define
// `WREN_OPT_<name>` a `0`.
#ifndef WREN_OPT_META
  #define WREN_OPT_META 1
#endif

#ifndef WREN_OPT_RANDOM
  #define WREN_OPT_RANDOM 1
#endif

// Estas banderas son útiles para depurar y trabajar en Wren mismo. No están
// pensadas para usarse en código de producción. Por defecto están desactivadas.

// Pon esto a true para poner a prueba el GC. Realizará una recolección antes de
// cada asignación. Esto es útil para asegurar que la memoria siempre sea
// correctamente alcanzable.
#define WREN_DEBUG_GC_STRESS 0

// Pon esto a true para registrar operaciones de memoria según ocurren.
#define WREN_DEBUG_TRACE_MEMORY 0

// Pon esto a true para registrar recolecciones de basura según ocurren.
#define WREN_DEBUG_TRACE_GC 0

// Pon esto a true para imprimir el bytecode compilado de cada función.
#define WREN_DEBUG_DUMP_COMPILED_CODE 0

// Pon esto a true para rastrear cada instrucción mientras se ejecuta.
#define WREN_DEBUG_TRACE_INSTRUCTIONS 0

// El número máximo de variables a nivel de módulo que pueden definirse a la vez.
// Esta limitación viene de los 16 bits usados para los argumentos a
// `CODE_LOAD_MODULE_VAR` y `CODE_STORE_MODULE_VAR`.
#define MAX_MODULE_VARS 65536

// El número máximo de argumentos que se pueden pasar a un método. Tenga en
// cuenta que esta limitación está codificada en otros lugares de la VM, en
// particular, las instrucciones `CODE_CALL_XX` asumen un máximo determinado.
#define MAX_PARAMETERS 16

// El nombre máximo de un método, sin incluir la firma. Este es un máximo
// arbitrario pero forzado simplemente para saber cuánto necesitan medir las
// cadenas del nombre del método en el analizador.
#define MAX_METHOD_NAME 64

// La longitud máxima de una firma de método. Las firmas se ven así:
//
//     foo        // Getter.
//     foo()      // Método sin argumentos.
//     foo(_)     // Método con un argumento.
//     foo(_,_)   // Método con dos argumentos.
//     init foo() // Inicializador del constructor.
//
// La longitud máxima de la firma tiene en cuenta el nombre de método más largo,
// el número máximo de parámetros con separadores entre ellos, "init " y "()".
#define MAX_METHOD_SIGNATURE (MAX_METHOD_NAME + (MAX_PARAMETERS * 2) + 6)

// La longitud máxima de un identificador. La única razón real para esta
// limitación es que los mensajes de error que mencionan variables puedan
// asignarse en la pila.
#define MAX_VARIABLE_NAME 64

// El número máximo de campos que una clase puede tener, incluidas las
// propiedades heredadas. Esto es explícito en el bytecode ya que `CODE_CLASS` y
// `CODE_SUBCLASS` toman un único byte para el número de campos. Tenga en cuenta
// que es 255 y no 256 porque crear una clase toma el *número* de campos, no el
// índice más alto del campo.
#define MAX_FIELDS 255

// El número máximo de métodos que puede manejar la VM.
#define MAX_METHODS 65535

// Usa el asignador de la VM para asignar un objeto de [type].
#define ALLOCATE(vm, type)                                                     \
    ((type*)wrenReallocate(vm, NULL, 0, sizeof(type)))

// Usa el asignador de la VM para asignar un objeto de [mainType] que contiene
// una matriz flexible de [count] objetos de [arrayType].
#define ALLOCATE_FLEX(vm, mainType, arrayType, count)                          \
    ((mainType*)wrenReallocate(vm, NULL, 0,                                    \
        sizeof(mainType) + sizeof(arrayType) * (count)))

// Usa el asignador de la VM para asignar una matriz de [count] elementos de
// [type].
#define ALLOCATE_ARRAY(vm, type, count)                                        \
    ((type*)wrenReallocate(vm, NULL, 0, sizeof(type) * (count)))

// Usa el asignador de la VM para liberar la memoria previamente asignada en
// [pointer].
#define DEALLOCATE(vm, pointer) wrenReallocate(vm, pointer, 0, 0)

// El compilador de Microsoft no admite el modificador "inline" al compilar
// como C simple.
#if defined( _MSC_VER ) && !defined(__cplusplus)
  #define inline _inline
#endif

// Esto se usa para marcar claramente las matrices de tamaño flexible que
// aparecen al final de algunos structs dinámicamente asignados, conocidas como
// "struct hack".
#if __STDC_VERSION__ >= 199901L
  // En C99, una matriz flexible es simplemente "[]".
  #define FLEXIBLE_ARRAY
#else
  // En otros casos, usa una matriz de tamaño cero. Es técnicamente un
  // comportamiento indefinido, pero funciona de forma fiable en la mayoría de
  // compiladores conocidos.
  #define FLEXIBLE_ARRAY 0
#endif

// Las aserciones se usan para validar invariantes del programa. Indican cosas
// que el programa espera que sean verdaderas sobre su estado interno durante la
// ejecución. Si una aserción falla, hay un bug en Wren.
//
// Las aserciones añaden un coste significativo, por lo que solo se habilitan en
// compilaciones de depuración.
#ifdef DEBUG

  #include <stdio.h>

  #define ASSERT(condition, message)                                           \
      do                                                                       \
      {                                                                        \
        if (!(condition))                                                      \
        {                                                                      \
          fprintf(stderr, "[%s:%d] Assert failed in %s(): %s\n",               \
              __FILE__, __LINE__, __func__, message);                          \
          abort();                                                             \
        }                                                                      \
      } while (false)

  // Indica que sabemos que la ejecución nunca debería llegar a este punto en el
  // programa. En modo depuración, afirmamos este hecho porque es un error
  // llegar aquí.
  //
  // En modo de lanzamiento, usamos funciones incorporadas específicas del
  // compilador para indicar que el código no puede alcanzarse. Esto evita
  // advertencias de "missing return" en algunos casos y también permite que el
  // compilador haga optimizaciones asumiendo que el código nunca se alcanza.
  #define UNREACHABLE()                                                        \
      do                                                                       \
      {                                                                        \
        fprintf(stderr, "[%s:%d] This code should not be reached in %s()\n",   \
            __FILE__, __LINE__, __func__);                                     \
        abort();                                                               \
      } while (false)

#else

  #define ASSERT(condition, message) do { } while (false)

  // Indica al compilador que esta parte del código nunca será alcanzada.
  #if defined( _MSC_VER )
    #define UNREACHABLE() __assume(0)
  #elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
    #define UNREACHABLE() __builtin_unreachable()
  #else
    #define UNREACHABLE()
  #endif

#endif

#endif
