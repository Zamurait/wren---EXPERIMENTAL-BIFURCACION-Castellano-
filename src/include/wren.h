#ifndef wren_h
#define wren_h

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

// El número de versión semántica de Wren.
#define WREN_VERSION_MAJOR 0
#define WREN_VERSION_MINOR 4
#define WREN_VERSION_PATCH 0

// Una representación legible para los humanos de la versión.
#define WREN_VERSION_STRING "0.4.0"

// Una representación numérica y monotónicamente creciente del número de
// versión. Úsala si quieres hacer comprobaciones de rango entre versiones.
#define WREN_VERSION_NUMBER (WREN_VERSION_MAJOR * 1000000 +                    \
                              WREN_VERSION_MINOR * 1000 +                       \
                              WREN_VERSION_PATCH)

#ifndef WREN_API
  #if defined(_MSC_VER) && defined(WREN_API_DLLEXPORT)
    #define WREN_API __declspec( dllexport )
  #else
    #define WREN_API
  #endif
#endif //WREN_API

// Una única máquina virtual para ejecutar código Wren.
//
// Wren no tiene estado global, por lo que todo el estado almacenado por un
// intérprete en ejecución vive aquí.
typedef struct WrenVM WrenVM;

// Un identificador para un objeto Wren.
//
// Esto permite que código fuera de la VM mantenga una referencia persistente a
// un objeto. Después de obtener un identificador y hasta que se libere, esto
// garantiza que el recolector de basura no reclamará el objeto al que hace
// referencia.
typedef struct WrenHandle WrenHandle;

// Una función genérica de asignación que maneja toda la gestión explícita de
// memoria utilizada por Wren. Se usa así:
//
// - Para asignar memoria nueva, [memory] es NULL y [newSize] es el tamaño
//   deseado. Debe devolver la memoria asignada o NULL si falla.
//
// - Para intentar aumentar una asignación existente, [memory] es la memoria y
//   [newSize] es el tamaño deseado. Debe devolver [memory] si pudo ampliar la
//   memoria in situ, o un puntero nuevo si tuvo que moverla.
//
// - Para reducir memoria, [memory] y [newSize] son los mismos que arriba, pero
//   siempre devolverá [memory].
//
// - Para liberar memoria, [memory] será la memoria a liberar y [newSize] será
//   cero. Debe devolver NULL.
typedef void* (*WrenReallocateFn)(void* memory, size_t newSize, void* userData);

// Una función invocable desde código Wren, pero implementada en C.
typedef void (*WrenForeignMethodFn)(WrenVM* vm);

// Una función finalizadora para liberar recursos propiedad de una instancia de
// una clase externa. A diferencia de la mayoría de los métodos externos,
// finalizadores no tienen acceso a la VM y no deben interactuar con ella, ya
// que se encuentran en medio de una recolección de basura.
typedef void (*WrenFinalizerFn)(void* data);

// Da al anfitrión la oportunidad de canonizar el nombre del módulo importado,
// posiblemente teniendo en cuenta el nombre (ya resuelto) del módulo que lo
// contiene. Normalmente, esto se usa para implementar importaciones relativas.
typedef const char* (*WrenResolveModuleFn)(WrenVM* vm,
    const char* importer, const char* name);

// Declaración adelantada.
struct WrenLoadModuleResult;

// Se invoca después de que se llame a loadModuleFn para el módulo [name]. El
// resultado original devuelto se te entrega en esta devolución de llamada, para
// que puedas liberar memoria si es apropiado.
typedef void (*WrenLoadModuleCompleteFn)(WrenVM* vm, const char* name, struct WrenLoadModuleResult result);

// El resultado de una llamada a loadModuleFn.
// [source] es el código fuente del módulo, o NULL si no se encontró.
// [onComplete] es una devolución de llamada opcional que se ejecutará cuando
// Wren termine con el resultado.
typedef struct WrenLoadModuleResult
{
  const char* source;
  WrenLoadModuleCompleteFn onComplete;
  void* userData;
} WrenLoadModuleResult;

// Carga y devuelve el código fuente del módulo [name].
typedef WrenLoadModuleResult (*WrenLoadModuleFn)(WrenVM* vm, const char* name);

// Devuelve un puntero a un método externo en [className] del módulo [module]
// con la firma [signature].
typedef WrenForeignMethodFn (*WrenBindForeignMethodFn)(WrenVM* vm,
    const char* module, const char* className, bool isStatic,
    const char* signature);

// Muestra una cadena de texto al usuario.
typedef void (*WrenWriteFn)(WrenVM* vm, const char* text);

typedef enum
{
  // Un error de sintaxis o de resolución detectado en tiempo de compilación.
  WREN_ERROR_COMPILE,

  // El mensaje de error para un error de ejecución.
  WREN_ERROR_RUNTIME,

  // Una entrada del seguimiento de pila de un error en tiempo de ejecución.
  WREN_ERROR_STACK_TRACE
} WrenErrorType;

// Informa de un error al usuario.
//
// Un error detectado durante la compilación se informa llamando a esto una vez
// con [type] `WREN_ERROR_COMPILE`, el nombre resuelto del [module] y la [line]
// donde ocurre el error, y el [message] del compilador.
//
// Un error de ejecución se informa llamando a esto una vez con [type]
// `WREN_ERROR_RUNTIME`, sin [module] ni [line], y el [message] del error de
// ejecución. Después, se realizan varias llamadas de tipo
// `WREN_ERROR_STACK_TRACE` para cada línea del seguimiento de pila. Cada una de
// esas tiene el [module] y la [line] resueltos donde se define el método o la
// función y [message] es el nombre del método o la función.
typedef void (*WrenErrorFn)(
    WrenVM* vm, WrenErrorType type, const char* module, int line,
    const char* message);

typedef struct
{
  // La devolución de llamada invocada cuando se crea el objeto externo.
  //
  // Debe proporcionarse. Dentro del cuerpo de esta, debe llamar a
  // [wrenSetSlotNewForeign()] exactamente una vez.
  WrenForeignMethodFn allocate;

  // La devolución de llamada invocada cuando el recolector de basura está a
  // punto de recopilar la memoria del objeto externo.
  //
  // Puede ser `NULL` si la clase externa no necesita finalización.
  WrenFinalizerFn finalize;
} WrenForeignClassMethods;

// Devuelve un par de punteros a los métodos externos utilizados para asignar y
// finalizar los datos de instancias de [className] en el [module] resuelto.
typedef WrenForeignClassMethods (*WrenBindForeignClassFn)(
    WrenVM* vm, const char* module, const char* className);

typedef struct
{
  // La devolución de llamada que Wren usará para asignar, reasignar y liberar
  // memoria.
  //
  // Si es `NULL`, usa una función incorporada que usa `realloc` y `free`.
  WrenReallocateFn reallocateFn;

  // La devolución de llamada que Wren usa para resolver un nombre de módulo.
  //
  // Algunas aplicaciones anfitrionas pueden querer admitir importaciones
  // "relativas", donde el significado de una cadena de importación depende del
  // módulo que la contiene. Para admitir eso sin introducir ninguna política en
  // Wren, la VM da al anfitrión la oportunidad de resolver una importación.
  //
  // Antes de cargar una importación, llama a esto, pasando el nombre del módulo
  // que contiene la importación y la cadena de importación. La aplicación
  // anfitriona puede mirar ambos y producir una cadena "canónica" nueva que
  // identifique de forma única el módulo. Esta cadena se usa como nombre del
  // módulo a partir de ese momento. Es la que se pasa a [loadModuleFn], cómo se
  // detectan importaciones duplicadas del mismo módulo y cómo se informa del
  // módulo en las trazas de la pila.
  //
  // Si dejas esta función en NULL, entonces la cadena de importación original se
  // trata como la cadena resuelta.
  //
  // Si una importación no puede resolverse por el embebedor, debe devolver NULL
  // y Wren reportará eso como un error de ejecución.
  //
  // Wren se hará cargo de la cadena que devuelvas y la liberará por ti, así
  // que debe asignarse usando la misma función de asignación que proporciones
  // arriba.
  WrenResolveModuleFn resolveModuleFn;

  // La devolución de llamada que Wren usa para cargar un módulo.
  //
  // Como Wren no habla directamente con el sistema de archivos, depende del
  // embebedor para localizar y leer físicamente el código fuente de un módulo.
  // La primera vez que aparece una importación, Wren la llamará y le pasará el
  // nombre del módulo que se importa. El método devolverá un resultado, que
  // contiene el código fuente de ese módulo. La memoria del código fuente es
  // propiedad de la aplicación anfitriona y puede liberarse utilizando la
  // devolución de llamada onComplete.
  //
  // Esto solo se llamará una vez para cualquier nombre de módulo. Wren almacena
  // en caché el resultado internamente, por lo que las importaciones posteriores
  // del mismo módulo reutilizarán el código fuente anterior y no llamarán a
  // esto.
  //
  // Si un módulo con el nombre dado no se encontró por el embebedor, debe
  // devolver NULL y Wren informará de un error de ejecución.
  WrenLoadModuleFn loadModuleFn;

  // La devolución de llamada que Wren usa para encontrar un método externo y
  // enlazarlo a una clase.
  //
  // Cuando se declara un método externo en una clase, esto se llamará con el
  // módulo, la clase y la firma del método externo cuando se ejecute el cuerpo
  // de la clase. Debe devolver un puntero a la función externa que se enlazará
  // a ese método.
  //
  // Si la función externa no se encontró, esto debe devolver NULL y Wren la
  // reportará como error en tiempo de ejecución.
  WrenBindForeignMethodFn bindForeignMethodFn;

  // La devolución de llamada que Wren usa para encontrar una clase externa y
  // obtener sus métodos externos.
  //
  // Cuando una clase externa se declara, esto se llamará con el módulo y el
  // nombre de la clase cuando se ejecute el cuerpo de la clase. Debe devolver
  // las funciones externas que se usan para asignar y (opcionalmente) finalizar
  // los bytes almacenados en el objeto externo cuando se crea una instancia.
  WrenBindForeignClassFn bindForeignClassFn;

  // La devolución de llamada que Wren usa para mostrar texto cuando
  // `System.print()` u otras funciones relacionadas se invocan.
  //
  // Si esto es `NULL`, Wren descarta cualquier texto impreso.
  WrenWriteFn writeFn;

  // La devolución de llamada que Wren usa para informar errores.
  //
  // Cuando ocurre un error, esto se llamará con el nombre del módulo, el número
  // de línea y un mensaje de error. Si esto es `NULL`, Wren no informa de
  // errores.
  WrenErrorFn errorFn;

  // El número de bytes que Wren asignará antes de desencadenar la primera
  // recolección de basura.
  //
  // Si es cero, usa 10MB por defecto.
  size_t initialHeapSize;

  // Después de que ocurra una recolección, el umbral para la siguiente se
  // determina en función del número de bytes restantes en uso. Esto permite que
  // Wren reduzca automáticamente su uso de memoria después de reclamar una gran
  // cantidad de memoria.
  //
  // Esto puede usarse para asegurar que el heap no se vuelva demasiado pequeño,
  // lo que a su vez puede provocar un gran número de recolecciones después de
  // que el heap vuelva a crecer hasta un tamaño útil.
  //
  // Si es cero, usa 1MB por defecto.
  size_t minHeapSize;

  // Wren redimensionará el heap automáticamente según cambie el número de bytes
  // que quedan en uso después de una recolección. Este número determina la
  // cantidad de memoria adicional que Wren usará después de una recolección, en
  // porcentaje del tamaño actual del heap.
  //
  // Por ejemplo, supongamos que esto es 50. Después de una recolección, cuando
  // queden 400 bytes de memoria en uso, la siguiente recolección se disparará
  // después de asignar un total de 600 bytes (incluidos los 400 ya en uso).
  //
  // Establecerlo a un valor más pequeño desperdicia menos memoria, pero activa
  // recolecciones más frecuentes.
  //
  // Si es cero, usa 50 por defecto.
  int heapGrowthPercent;

  // Datos definidos por el usuario asociados a la VM.
  void* userData;

} WrenConfiguration;

typedef enum
{
  WREN_RESULT_SUCCESS,
  WREN_RESULT_COMPILE_ERROR,
  WREN_RESULT_RUNTIME_ERROR
} WrenInterpretResult;

// El tipo de un objeto almacenado en una ranura.
//
// Esto no es necesariamente la *clase* del objeto, sino su tipo de
// representación de bajo nivel.
typedef enum
{
  WREN_TYPE_BOOL,
  WREN_TYPE_NUM,
  WREN_TYPE_FOREIGN,
  WREN_TYPE_LIST,
  WREN_TYPE_MAP,
  WREN_TYPE_NULL,
  WREN_TYPE_STRING,

  // El objeto es de un tipo que no es accesible por la API de C.
  WREN_TYPE_UNKNOWN
} WrenType;

// Obtiene el número de versión actual de Wren.
//
// Se puede usar para comprobar rangos de versiones.
WREN_API int wrenGetVersionNumber();

// Inicializa [configuration] con todos sus valores predeterminados.
//
// Llamar a esto antes de establecer los campos particulares que te interesen.
WREN_API void wrenInitConfiguration(WrenConfiguration* configuration);

// Crea una nueva máquina virtual de Wren usando la [configuration] dada. Wren
// copiará los datos de configuración, así que el argumento pasado a esta
// función puede liberarse después de llamarla. Si [configuration] es `NULL`,
// usa una configuración predeterminada.
WREN_API WrenVM* wrenNewVM(WrenConfiguration* configuration);

// Libera todos los recursos en uso por [vm], que fueron creados previamente por
// una llamada a [wrenNewVM].
WREN_API void wrenFreeVM(WrenVM* vm);

// Ejecuta de inmediato el recolector de basura para liberar memoria no usada.
WREN_API void wrenCollectGarbage(WrenVM* vm);

// Ejecuta [source], una cadena de código fuente de Wren en una nueva fibra en
// [vm] en el contexto del [module] resuelto.
WREN_API WrenInterpretResult wrenInterpret(WrenVM* vm, const char* module,
                                   const char* source);

// Crea un identificador que puede usarse para invocar un método con [signature]
// sobre un receptor y argumentos que se preparan en la pila.
//
// Este identificador puede usarse repetidamente para invocar directamente ese
// método desde código C usando [wrenCall].
//
// Cuando termines con este identificador, debe liberarse usando
// [wrenReleaseHandle].
WREN_API WrenHandle* wrenMakeCallHandle(WrenVM* vm, const char* signature);

// Llama a [method], usando el receptor y los argumentos previamente configurados
// en la pila.
//
// [method] debe haber sido creado por una llamada a [wrenMakeCallHandle]. Los
// argumentos del método deben estar ya en la pila. El receptor debe estar en la
// ranura 0 y los argumentos restantes después de este, en orden. Es un error si
// el número de argumentos proporcionados no coincide con la firma del método.
//
// Después de que esto devuelve, puedes acceder al valor de retorno desde la
// ranura 0 en la pila.
WREN_API WrenInterpretResult wrenCall(WrenVM* vm, WrenHandle* method);

// Libera la referencia almacenada en [handle]. Después de llamar a esto,
// [handle] ya no puede usarse.
WREN_API void wrenReleaseHandle(WrenVM* vm, WrenHandle* handle);

// Las siguientes funciones están pensadas para ser llamadas desde métodos o
// finalizadores externos. La interfaz que Wren proporciona a un método externo
// se parece a una máquina de registros: se te da una matriz numerada de ranuras
// en las que se pueden leer y escribir valores. Los valores siempre viven en una
// ranura (a menos que se capturen explícitamente usando
// wrenGetSlotHandle(), que asegura que el recolector de basura pueda
// encontrarlos).
//
// Cuando se llama a tu función externa, se te da una ranura para el receptor y
// cada argumento del método. El receptor está en la ranura 0 y los argumentos
// están en ranuras numeradas cada vez mayores después de esta. Eres libre de
// leer y escribir en esas ranuras como quieras. Si necesitas más ranuras para
// usar como espacio de trabajo, puedes llamar a wrenEnsureSlots() para añadir
// más.
//
// Cuando tu función devuelve, cada ranura excepto la cero se descarta y el
// valor de la ranura cero se usa como valor de retorno del método. Si no
// almacenas un valor de retorno en esa ranura tú mismo, conservará su valor
// anterior, el receptor.
//
// Aunque Wren es dinámicamente tipado, C no lo es. Esto significa que la
// interfaz C tiene que soportar los distintos tipos de valores primitivos que
// puede contener una variable de Wren: booleano, doble, cadena, etc. Si
// soportáramos esto para cada operación en la API de C, habría una explosión
// combinatoria de funciones, como "obtener un elemento dobleado de una lista",
// "insertar una clave de cadena y un valor doble en un mapa", etc.
//
// Para evitar eso, la única forma de convertir a y desde un valor C bruto es
// entrar y salir de una ranura. Todas las demás funciones trabajan con valores
// ya en una ranura. Por lo tanto, para añadir un elemento a una lista, pones la
// lista en una ranura y el elemento en otra. Luego hay una sola función de API,
// wrenInsertInList(), que toma el elemento de esa ranura y lo coloca en la
// lista.
//
// El objetivo de esta API es ser fácil de usar sin comprometer el rendimiento.
// Esto último significa que no hace comprobaciones de tipos ni de límites en
// tiempo de ejecución, excepto mediante aserciones que generalmente se eliminan
// en compilaciones de lanzamiento. C es un lenguaje inseguro, por lo que te
// corresponde tener cuidado al usarlo correctamente. A cambio, obtienes una FFI
// muy rápida.

// Devuelve el número de ranuras disponibles para el método externo actual.
WREN_API int wrenGetSlotCount(WrenVM* vm);

// Asegura que la pila del método externo tenga al menos [numSlots] disponibles
// para su uso, creciendo la pila si es necesario.
//
// No encoge la pila si tiene más de suficientes ranuras.
//
// Es un error llamar a esto desde un finalizador.
WREN_API void wrenEnsureSlots(WrenVM* vm, int numSlots);

// Obtiene el tipo del objeto en [slot].
WREN_API WrenType wrenGetSlotType(WrenVM* vm, int slot);

// Lee un valor booleano de [slot].
//
// Es un error llamar a esto si la ranura no contiene un valor booleano.
WREN_API bool wrenGetSlotBool(WrenVM* vm, int slot);

// Lee una matriz de bytes de [slot].
//
// La memoria de la cadena devuelta es propiedad de Wren. Puedes inspeccionarla
// mientras estés en tu método externo, pero no puedes conservar un puntero a
// ella después de que la función devuelva, porque el recolector de basura puede
// reclamarla.
//
// Devuelve un puntero al primer byte de la matriz y llena [length] con el
// número de bytes de la matriz.
//
// Es un error llamar a esto si la ranura no contiene una cadena.
WREN_API const char* wrenGetSlotBytes(WrenVM* vm, int slot, int* length);

// Lee un número de [slot].
//
// Es un error llamar a esto si la ranura no contiene un número.
WREN_API double wrenGetSlotDouble(WrenVM* vm, int slot);

// Lee un objeto externo de [slot] y devuelve un puntero a los datos
// externos almacenados con él.
//
// Es un error llamar a esto si la ranura no contiene una instancia de una
// clase externa.
WREN_API void* wrenGetSlotForeign(WrenVM* vm, int slot);

// Lee una cadena de [slot].
//
// La memoria de la cadena devuelta es propiedad de Wren. Puedes inspeccionarla
// mientras estés en tu método externo, pero no puedes conservar un puntero a
// ella después de que la función devuelva, porque el recolector de basura puede
// reclamarla.
//
// Es un error llamar a esto si la ranura no contiene una cadena.
WREN_API const char* wrenGetSlotString(WrenVM* vm, int slot);

// Crea un identificador para el valor almacenado en [slot].
//
// Esto evitará que el objeto al que se hace referencia sea recolectado por la
// basura hasta que se libere el identificador llamando a [wrenReleaseHandle()].
WREN_API WrenHandle* wrenGetSlotHandle(WrenVM* vm, int slot);

// Almacena el booleano [value] en [slot].
WREN_API void wrenSetSlotBool(WrenVM* vm, int slot, bool value);

// Almacena la matriz [length] de [bytes] en [slot].
//
// Los bytes se copian a una nueva cadena dentro del heap de Wren, así que
// puedes liberar la memoria usada por ellos después de llamar a esto.
WREN_API void wrenSetSlotBytes(WrenVM* vm, int slot, const char* bytes, size_t length);

// Almacena el valor numérico [value] en [slot].
WREN_API void wrenSetSlotDouble(WrenVM* vm, int slot, double value);

// Crea una nueva instancia de la clase externa almacenada en [classSlot] con
// [size] bytes de almacenamiento sin procesar y coloca el objeto resultante en
// [slot].
//
// Esto no invoca el constructor de la clase externa en la nueva instancia. Si
// necesitas que ocurra, llama al constructor desde Wren, que entonces llamará
// al método externo del asignador. En él, llama a esto para crear el objeto y
// luego el constructor se invocará cuando el asignador devuelva.
//
// Devuelve un puntero a los datos del objeto externo.
WREN_API void* wrenSetSlotNewForeign(WrenVM* vm, int slot, int classSlot, size_t size);

// Almacena una nueva lista vacía en [slot].
WREN_API void wrenSetSlotNewList(WrenVM* vm, int slot);

// Almacena un nuevo mapa vacío en [slot].
WREN_API void wrenSetSlotNewMap(WrenVM* vm, int slot);

// Almacena null en [slot].
WREN_API void wrenSetSlotNull(WrenVM* vm, int slot);

// Almacena la cadena [text] en [slot].
//
// El [text] se copia a una nueva cadena dentro del heap de Wren, así que puedes
// liberar la memoria usada por él después de llamar a esto. La longitud se
// calcula usando [strlen()]. Si la cadena puede contener bytes nulos en el
// medio, entonces debes usar [wrenSetSlotBytes()] en su lugar.
WREN_API void wrenSetSlotString(WrenVM* vm, int slot, const char* text);

// Almacena el valor capturado en [handle] en [slot].
//
// Esto no libera el identificador del valor.
WREN_API void wrenSetSlotHandle(WrenVM* vm, int slot, WrenHandle* handle);

// Devuelve el número de elementos en la lista almacenada en [slot].
WREN_API int wrenGetListCount(WrenVM* vm, int slot);

// Lee el elemento [index] de la lista en [listSlot] y lo almacena en
// [elementSlot].
WREN_API void wrenGetListElement(WrenVM* vm, int listSlot, int index, int elementSlot);

// Establece el valor almacenado en [index] en la lista en [listSlot],
// al valor de [elementSlot].
WREN_API void wrenSetListElement(WrenVM* vm, int listSlot, int index, int elementSlot);

// Toma el valor almacenado en [elementSlot] e insértalo en la lista almacenada
// en [listSlot] en [index].
//
// Como en Wren, se pueden usar índices negativos para insertar desde el final.
// Para añadir un elemento, usa `-1` como índice.
WREN_API void wrenInsertInList(WrenVM* vm, int listSlot, int index, int elementSlot);

// Devuelve el número de entradas en el mapa almacenado en [slot].
WREN_API int wrenGetMapCount(WrenVM* vm, int slot);

// Devuelve true si la clave en [keySlot] se encuentra en el mapa colocado en
// [mapSlot].
WREN_API bool wrenGetMapContainsKey(WrenVM* vm, int mapSlot, int keySlot);

// Recupera un valor con la clave en [keySlot] del mapa en [mapSlot] y lo
// almacena en [valueSlot].
WREN_API void wrenGetMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);

// Toma el valor almacenado en [valueSlot] y lo inserta en el mapa almacenado en
// [mapSlot] con la clave [keySlot].
WREN_API void wrenSetMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot);

// Elimina un valor del mapa en [mapSlot], con la clave de [keySlot],
// y lo coloca en [removedValueSlot]. Si no se encuentra, [removedValueSlot] se
// establece en null, el mismo comportamiento que la API de Map de Wren.
WREN_API void wrenRemoveMapValue(WrenVM* vm, int mapSlot, int keySlot,
                         int removedValueSlot);

// Busca la variable de nivel superior con [name] en el [module] resuelto y la
// almacena en [slot].
WREN_API void wrenGetVariable(WrenVM* vm, const char* module, const char* name,
                      int slot);

// Busca la variable de nivel superior con [name] en el [module] resuelto,
// devuelve false si no se encuentra. El módulo debe estar importado en ese
// momento; usa wrenHasModule para asegurarte antes de llamar.
WREN_API bool wrenHasVariable(WrenVM* vm, const char* module, const char* name);

// Devuelve true si [module] se ha importado/resuelto antes, false si no.
WREN_API bool wrenHasModule(WrenVM* vm, const char* module);

// Establece la fibra actual para que se aborta y usa el valor en [slot] como
// el objeto de error de ejecución.
WREN_API void wrenAbortFiber(WrenVM* vm, int slot);

// Devuelve los datos de usuario asociados a la WrenVM.
WREN_API void* wrenGetUserData(WrenVM* vm);

// Establece los datos de usuario asociados a la WrenVM.
WREN_API void wrenSetUserData(WrenVM* vm, void* userData);

#endif
