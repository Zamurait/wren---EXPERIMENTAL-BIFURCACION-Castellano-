#include <string.h>

#include "wren_utils.h"
#include "wren_vm.h"

DEFINE_BUFFER(Byte, uint8_t);
DEFINE_BUFFER(Int, int);
DEFINE_BUFFER(String, ObjString*);

void wrenSymbolTableInit(SymbolTable* symbols)
{
  wrenStringBufferInit(symbols);
}

void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols)
{
  wrenStringBufferClear(vm, symbols);
}

int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols,
                       const char* name, size_t length)
{
  ObjString* symbol = AS_STRING(wrenNewStringLength(vm, name, length));
  
  wrenPushRoot(vm, &symbol->obj);
  wrenStringBufferWrite(vm, symbols, symbol);
  wrenPopRoot(vm);
  
  return symbols->count - 1;
}

int wrenSymbolTableEnsure(WrenVM* vm, SymbolTable* symbols,
                          const char* name, size_t length)
{
  // Comprueba si el símbolo ya está definido.
  int existing = wrenSymbolTableFind(symbols, name, length);
  if (existing != -1) return existing;

  // Es un símbolo nuevo, así que lo añade.
  return wrenSymbolTableAdd(vm, symbols, name, length);
}

int wrenSymbolTableFind(const SymbolTable* symbols,
                        const char* name, size_t length)
{
  // Comprueba si el símbolo ya está definido.
  // TODO: O(n). Buscar una solución mejor.
  for (int i = 0; i < symbols->count; i++)
  {
    if (wrenStringEqualsCString(symbols->data[i], name, length)) return i;
  }

  return -1;
}

void wrenBlackenSymbolTable(WrenVM* vm, SymbolTable* symbolTable)
{
  for (int i = 0; i < symbolTable->count; i++)
  {
    wrenGrayObj(vm, &symbolTable->data[i]->obj);
  }
  
  // Lleva la cuenta de cuánta memoria sigue en uso.
  vm->bytesAllocated += symbolTable->capacity * sizeof(*symbolTable->data);
}

int wrenUtf8EncodeNumBytes(int value)
{
  ASSERT(value >= 0, "Cannot encode a negative value.");
  
  if (value <= 0x7f) return 1;
  if (value <= 0x7ff) return 2;
  if (value <= 0xffff) return 3;
  if (value <= 0x10ffff) return 4;
  return 0;
}

int wrenUtf8Encode(int value, uint8_t* bytes)
{
  if (value <= 0x7f)
  {
    // Un byte (es decir, cabe en ASCII).
    *bytes = value & 0x7f;
    return 1;
  }
  else if (value <= 0x7ff)
  {
    // Secuencia de dos bytes: 110xxxxx 10xxxxxx.
    *bytes = 0xc0 | ((value & 0x7c0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
    return 2;
  }
  else if (value <= 0xffff)
  {
    // Secuencia de tres bytes: 1110xxxx 10xxxxxx 10xxxxxx.
    *bytes = 0xe0 | ((value & 0xf000) >> 12);
    bytes++;
    *bytes = 0x80 | ((value & 0xfc0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
    return 3;
  }
  else if (value <= 0x10ffff)
  {
    // Secuencia de cuatro bytes: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
    *bytes = 0xf0 | ((value & 0x1c0000) >> 18);
    bytes++;
    *bytes = 0x80 | ((value & 0x3f000) >> 12);
    bytes++;
    *bytes = 0x80 | ((value & 0xfc0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
    return 4;
  }

  // Valor Unicode no válido. Véase: http://tools.ietf.org/html/rfc3629
  UNREACHABLE();
  return 0;
}

int wrenUtf8Decode(const uint8_t* bytes, uint32_t length)
{
  // Un byte (es decir, cabe en ASCII).
  if (*bytes <= 0x7f) return *bytes;

  int value;
  uint32_t remainingBytes;
  if ((*bytes & 0xe0) == 0xc0)
  {
    // Secuencia de dos bytes: 110xxxxx 10xxxxxx.
    value = *bytes & 0x1f;
    remainingBytes = 1;
  }
  else if ((*bytes & 0xf0) == 0xe0)
  {
    // Secuencia de tres bytes: 1110xxxx 10xxxxxx 10xxxxxx.
    value = *bytes & 0x0f;
    remainingBytes = 2;
  }
  else if ((*bytes & 0xf8) == 0xf0)
  {
    // Secuencia de cuatro bytes: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
    value = *bytes & 0x07;
    remainingBytes = 3;
  }
  else
  {
    // Secuencia UTF-8 no válida.
    return -1;
  }

  // No leer más allá del final del búfer si la secuencia UTF-8 está truncada.
  if (remainingBytes > length - 1) return -1;

  while (remainingBytes > 0)
  {
    bytes++;
    remainingBytes--;

    // Los bytes restantes deben tener la forma 10xxxxxx.
    if ((*bytes & 0xc0) != 0x80) return -1;

    value = value << 6 | (*bytes & 0x3f);
  }

  return value;
}

int wrenUtf8DecodeNumBytes(uint8_t byte)
{
  // Si el byte comienza con 10xxxxx, está en medio de una secuencia UTF-8, así
  // que no lo cuentes.
  if ((byte & 0xc0) == 0x80) return 0;
  
  // Los bits altos del primer byte indican cuántos bytes tiene la secuencia
  // UTF-8.
  if ((byte & 0xf8) == 0xf0) return 4;
  if ((byte & 0xf0) == 0xe0) return 3;
  if ((byte & 0xe0) == 0xc0) return 2;
  return 1;
}

// De: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
int wrenPowerOf2Ceil(int n)
{
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  
  return n;
}

uint32_t wrenValidateIndex(uint32_t count, int64_t value)
{
  // Los índices negativos cuentan desde el final.
  if (value < 0) value = count + value;

  // Comprueba los límites.
  if (value >= 0 && value < count) return (uint32_t)value;

  return UINT32_MAX;
}
