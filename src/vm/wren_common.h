#ifndef wren_common_h
#define wren_common_h

// Este encabezado contiene macros y definiciones usadas en toda la
// implementación de Wren. En particular, contiene definiciones de
// configuración que controlan cómo funciona Wren. Algunas solo se usan al
// trabajar directamente en Wren mismo.
//
// Este encabezado no está pensado para incluirse en código fuera de Wren.

// Wren usa ampliamente los tipos enteros de C99 (uint16_t, etc.) junto con
// algunas constantes asociadas (UINT32_MAX, etc.). Las constantes no forman
// parte del estándar de C++, así que no se incluyen por defecto en compiladores
// de C++ al incluir <stdint.h>, a menos que se defina __STDC_LIMIT_MACROS.
#define __STDC_LIMIT_MACROS
#include <stdint.h>

// Estas banderas permiten controlar algunos detalles de la implementación del
// intérprete. Normalmente, intercambian algo de portabilidad por velocidad. De
// manera predeterminada usan el comportamiento más eficiente.

// Si es true, Wren usa un double etiquetado con NaN para su representación
// principal de valores. De lo contrario, usa una estructura más grande y
// convencional. La primera opción es significativamente más rápida y compacta;
// la segunda es útil para depuración y puede ser más portable.
//
// Desactivado de manera predeterminada.
#ifndef WREN_NAN_TAGGING
  #define WREN_NAN_TAGGING 1
#endif

// Si es true, el bucle intérprete de la VM usa gotos computados. Consulta:
// http://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Labels-as-Values.html
// Activar esta opción acelera un poco el bucle principal de despacho, pero
// requiere compatibilidad del compilador.
// Véase https://bullno1.com/blog/switched-goto para una alternativa.
// Está activado de manera predeterminada en los compiladores compatibles.
#ifndef WREN_COMPUTED_GOTO
  #if defined(_MSC_VER) && !defined(__clang__)
    // Visual Studio no admite gotos computados.
    #define WREN_COMPUTED_GOTO 0
  #else
    #define WREN_COMPUTED_GOTO 1
  #endif
#endif

// La VM incluye varios módulos opcionales. Puedes elegir incluirlos o no. De
// manera predeterminada, todos están disponibles. Para deshabilitar uno, define
// `WREN_OPT_<name>` como `0`.
#ifndef WREN_OPT_META
  #define WREN_OPT_META 1
#endif

#ifndef WREN_OPT_RANDOM
  #define WREN_OPT_RANDOM 1
#endif

// Estas banderas son útiles para depurar y trabajar en Wren mismo. No están
// pensadas para usarse en código de producción. Están desactivadas de manera
// predeterminada.

// Ponlo en true para someter el GC a una prueba de estrés. Realizará una
// recolección antes de cada asignación. Esto ayuda a asegurar que la memoria
// siempre sea accesible correctamente.
#define WREN_DEBUG_GC_STRESS 0

// Ponlo en true para registrar las operaciones de memoria a medida que ocurren.
#define WREN_DEBUG_TRACE_MEMORY 0

// Ponlo en true para registrar las recolecciones de basura a medida que ocurren.
#define WREN_DEBUG_TRACE_GC 0

// Ponlo en true para imprimir el bytecode compilado de cada función.
#define WREN_DEBUG_DUMP_COMPILED_CODE 0

// Ponlo en true para rastrear cada instrucción mientras se ejecuta.
#define WREN_DEBUG_TRACE_INSTRUCTIONS 0

// Número máximo de variables de nivel de módulo que pueden definirse a la vez.
// Esta limitación se debe a los 16 bits usados para los argumentos de
// `CODE_LOAD_MODULE_VAR` y `CODE_STORE_MODULE_VAR`.
#define MAX_MODULE_VARS 65536

// Número máximo de argumentos que pueden pasarse a un método. Esta limitación
// también está codificada en otros lugares de la VM; en particular, las
// instrucciones `CODE_CALL_XX` suponen un máximo determinado.
#define MAX_PARAMETERS 16

// Longitud máxima del nombre de un método, sin incluir la firma. Es un máximo
// arbitrario pero obligatorio, que permite determinar el tamaño necesario para
// las cadenas de nombres de método en el analizador.
#define MAX_METHOD_NAME 64

// Longitud máxima de una firma de método. Las firmas tienen este aspecto:
//
//     foo        // Getter.
//     foo()      // Método sin argumentos.
//     foo(_)     // Método con un argumento.
//     foo(_,_)   // Método con dos argumentos.
//     init foo() // Inicializador del constructor.
//
// Para calcular la longitud máxima se tienen en cuenta el nombre de método más
// largo, el número máximo de parámetros con sus separadores, "init " y "()".
#define MAX_METHOD_SIGNATURE (MAX_METHOD_NAME + (MAX_PARAMETERS * 2) + 6)

// Longitud máxima de un identificador. La única razón real para esta limitación
// es permitir que los mensajes de error que mencionan variables se asignen en la
// pila.
#define MAX_VARIABLE_NAME 64

// Número máximo de campos de una clase, incluidos los heredados. Esta restricción
// está codificada explícitamente en el bytecode, ya que `CODE_CLASS` y
// `CODE_SUBCLASS` usan un solo byte para el número de campos. El límite es 255 y
// no 256 porque al crear una clase se usa el *número* de campos, no el índice más
// alto del campo.
#define MAX_FIELDS 255

// Número máximo de métodos que puede manejar la VM.
#define MAX_METHODS 65535

// Usa el asignador de la VM para asignar un objeto de tipo [type].
#define ALLOCATE(vm, type)                                                     \
    ((type*)wrenReallocate(vm, NULL, 0, sizeof(type)))

// Usa el asignador de la VM para asignar un objeto de tipo [mainType] que
// contiene una matriz flexible de [count] objetos de tipo [arrayType].
#define ALLOCATE_FLEX(vm, mainType, arrayType, count)                          \
    ((mainType*)wrenReallocate(vm, NULL, 0,                                    \
        sizeof(mainType) + sizeof(arrayType) * (count)))

// Usa el asignador de la VM para asignar una matriz de [count] elementos de tipo
// [type].
#define ALLOCATE_ARRAY(vm, type, count)                                        \
    ((type*)wrenReallocate(vm, NULL, 0, sizeof(type) * (count)))

// Usa el asignador de la VM para liberar la memoria previamente asignada en
// [pointer].
#define DEALLOCATE(vm, pointer) wrenReallocate(vm, pointer, 0, 0)

// El compilador de Microsoft no admite el modificador "inline" al compilar como
// C simple.
#if defined( _MSC_VER ) && !defined(__cplusplus)
  #define inline _inline
#endif

// Se usa para marcar claramente las matrices de tamaño flexible que aparecen al
// final de algunas estructuras asignadas dinámicamente, conocidas como
// "struct hack".
#if __STDC_VERSION__ >= 199901L
  // En C99, un miembro de matriz flexible se escribe simplemente "[]".
  #define FLEXIBLE_ARRAY
#else
  // En otros entornos se usa una matriz de tamaño cero. Técnicamente es un
  // comportamiento indefinido, pero funciona de forma fiable en la mayoría de
  // los compiladores conocidos.
  #define FLEXIBLE_ARRAY 0
#endif

// Las aserciones se usan para validar invariantes del programa: condiciones que
// se espera que sean verdaderas sobre su estado interno durante la ejecución. Si
// una aserción falla, hay un error en Wren.
//
// Las aserciones tienen un coste considerable, por lo que solo se habilitan en
// las compilaciones de depuración.
#ifdef DEBUG

  #include <stdio.h>

  #define ASSERT(condition, message)                                           \
      do                                                                       \
      {                                                                        \
        if (!(condition))                                                      \
        {                                                                      \
          fprintf(stderr, "[%s:%d] Falló la aserción en %s(): %s\n",           \
              __FILE__, __LINE__, __func__, message);                          \
          abort();                                                             \
        }                                                                      \
      } while (false)

  // Indica que sabemos que la ejecución nunca debería llegar a este punto del
  // programa. En modo de depuración, comprobamos esta condición porque llegar
  // aquí indicaría un error.
  //
  // En modo de lanzamiento, se usan funciones integradas específicas del
  // compilador para indicarle que no se puede llegar a este código. Esto evita
  // advertencias de retorno faltante en algunos casos y permite ciertas
  // optimizaciones basadas en que el código es inalcanzable.
  #define UNREACHABLE()                                                        \
      do                                                                       \
      {                                                                        \
        fprintf(stderr, "[%s:%d] No se debería llegar a este código en %s()\n", \
            __FILE__, __LINE__, __func__);                                     \
        abort();                                                               \
      } while (false)

#else

  #define ASSERT(condition, message) do { } while (false)

  // Indica al compilador que nunca se alcanzará esta parte del código.
  #if defined( _MSC_VER )
    #define UNREACHABLE() __assume(0)
  #elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
    #define UNREACHABLE() __builtin_unreachable()
  #else
    #define UNREACHABLE()
  #endif

#endif

#endif
