#ifndef wren_utils_h
#define wren_utils_h

#include "wren.h"
#include "wren_common.h"

// Estructuras de datos reutilizables y otras funciones auxiliares.

// Declara esto aquí para romper un ciclo entre wren_utils.h y wren_value.h.
typedef struct sObjString ObjString;

// Necesitamos buffers de varios tipos. Para evitar muchas conversiones entre
// void* y otros tipos, usaremos el preprocesador como un "genérico pobre" y lo
// haremos generar unos pocos buffers específicos por tipo.
#define DECLARE_BUFFER(name, type)                                             \
    typedef struct                                                             \
    {                                                                          \
      type* data;                                                              \
      int count;                                                               \
      int capacity;                                                            \
    } name##Buffer;                                                            \
    void wren##name##BufferInit(name##Buffer* buffer);                         \
    void wren##name##BufferClear(WrenVM* vm, name##Buffer* buffer);            \
    void wren##name##BufferFill(WrenVM* vm, name##Buffer* buffer, type data,   \
                                int count);                                    \
    void wren##name##BufferWrite(WrenVM* vm, name##Buffer* buffer, type data)

// Esto debe usarse una vez para cada instanciación de tipo, en algún archivo .c.
#define DEFINE_BUFFER(name, type)                                              \
    void wren##name##BufferInit(name##Buffer* buffer)                          \
    {                                                                          \
      buffer->data = NULL;                                                     \
      buffer->capacity = 0;                                                    \
      buffer->count = 0;                                                       \
    }                                                                          \
                                                                               \
    void wren##name##BufferClear(WrenVM* vm, name##Buffer* buffer)             \
    {                                                                          \
      wrenReallocate(vm, buffer->data, 0, 0);                                  \
      wren##name##BufferInit(buffer);                                          \
    }                                                                          \
                                                                               \
    void wren##name##BufferFill(WrenVM* vm, name##Buffer* buffer, type data,   \
                                int count)                                     \
    {                                                                          \
      if (buffer->capacity < buffer->count + count)                            \
      {                                                                        \
        int capacity = wrenPowerOf2Ceil(buffer->count + count);                \
        buffer->data = (type*)wrenReallocate(vm, buffer->data,                 \
            buffer->capacity * sizeof(type), capacity * sizeof(type));         \
        buffer->capacity = capacity;                                           \
      }                                                                        \
                                                                               \
      for (int i = 0; i < count; i++)                                          \
      {                                                                        \
        buffer->data[buffer->count++] = data;                                  \
      }                                                                        \
    }                                                                          \
                                                                               \
    void wren##name##BufferWrite(WrenVM* vm, name##Buffer* buffer, type data)  \
    {                                                                          \
      wren##name##BufferFill(vm, buffer, data, 1);                             \
    }

DECLARE_BUFFER(Byte, uint8_t);
DECLARE_BUFFER(Int, int);
DECLARE_BUFFER(String, ObjString*);

// TODO: Cambiar esto para usar un mapa.
typedef StringBuffer SymbolTable;

// Inicializa la tabla de símbolos.
void wrenSymbolTableInit(SymbolTable* symbols);

// Libera toda la memoria asignada dinámicamente usada por la tabla de símbolos,
// pero no la propia SymbolTable.
void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols);

// Añade name a la tabla de símbolos. Devuelve el índice en la tabla.
int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols,
                        const char* name, size_t length);

// Añade name a la tabla de símbolos. Devuelve el índice en la tabla. Usará un
// símbolo existente si ya está presente.
int wrenSymbolTableEnsure(WrenVM* vm, SymbolTable* symbols,
                           const char* name, size_t length);

// Busca name en la tabla de símbolos. Devuelve su índice si se encuentra o -1 si
// no.
int wrenSymbolTableFind(const SymbolTable* symbols,
                        const char* name, size_t length);

void wrenBlackenSymbolTable(WrenVM* vm, SymbolTable* symbolTable);

// Devuelve el número de bytes necesarios para codificar [value] en UTF-8.
//
// Devuelve 0 si [value] es demasiado grande para codificarse.
int wrenUtf8EncodeNumBytes(int value);

// Codifica value como una serie de bytes en [bytes], que se asume que es
// suficiente para albergar la codificación.
//
// Devuelve el número de bytes escritos.
int wrenUtf8Encode(int value, uint8_t* bytes);

// Decodifica la secuencia UTF-8 que comienza en [bytes] (que tiene longitud
// máxima [length]), devolviendo el punto de código.
//
// Devuelve -1 si los bytes no son una secuencia UTF-8 válida.
int wrenUtf8Decode(const uint8_t* bytes, uint32_t length);

// Devuelve el número de bytes en la secuencia UTF-8 que comienza por [byte].
//
// Si el carácter en ese índice no es el comienzo de una secuencia UTF-8,
// devuelve 0.
int wrenUtf8DecodeNumBytes(uint8_t byte);

// Devuelve la potencia de dos más pequeña que es igual o mayor que [n].
int wrenPowerOf2Ceil(int n);

// Valida que [value] esté dentro de `[0, count)`. También permite índices
// negativos que cuentan hacia atrás desde el final. Devuelve el valor positivo
// válido. Si es inválido, devuelve `UINT32_MAX`.
uint32_t wrenValidateIndex(uint32_t count, int64_t value);

#endif
