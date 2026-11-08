#include "wren_primitive.h"

#include <math.h>

// Valida que [value] sea un entero dentro de `[0, count)`. También permite
// índices negativos que cuentan hacia atrás desde el final. Devuelve el índice
// positivo válido. Si es inválido, informa de un error y devuelve `UINT32_MAX`.
static uint32_t validateIndexValue(WrenVM* vm, uint32_t count, double value,
                                   const char* argName)
{
  if (!validateIntValue(vm, value, argName)) return UINT32_MAX;
  
  // Los índices negativos cuentan desde el final.
  if (value < 0) value = count + value;
  
  // Comprueba los límites.
  if (value >= 0 && value < count) return (uint32_t)value;
  
  vm->fiber->error = wrenStringFormat(vm, "$ out of bounds.", argName);
  return UINT32_MAX;
}

bool validateFn(WrenVM* vm, Value arg, const char* argName)
{
  if (IS_CLOSURE(arg)) return true;
  RETURN_ERROR_FMT("$ must be a function.", argName);
}

bool validateNum(WrenVM* vm, Value arg, const char* argName)
{
  if (IS_NUM(arg)) return true;
  RETURN_ERROR_FMT("$ must be a number.", argName);
}

bool validateIntValue(WrenVM* vm, double value, const char* argName)
{
  if (trunc(value) == value) return true;
  RETURN_ERROR_FMT("$ must be an integer.", argName);
}

bool validateInt(WrenVM* vm, Value arg, const char* argName)
{
  // Primero, asegúrate de que sea un número.
  if (!validateNum(vm, arg, argName)) return false;
  return validateIntValue(vm, AS_NUM(arg), argName);
}

bool validateKey(WrenVM* vm, Value arg)
{
  if (wrenMapIsValidKey(arg)) return true;

  RETURN_ERROR("Key must be a value type.");
}

uint32_t validateIndex(WrenVM* vm, Value arg, uint32_t count,
                       const char* argName)
{
  if (!validateNum(vm, arg, argName)) return UINT32_MAX;
  return validateIndexValue(vm, count, AS_NUM(arg), argName);
}

bool validateString(WrenVM* vm, Value arg, const char* argName)
{
  if (IS_STRING(arg)) return true;
  RETURN_ERROR_FMT("$ must be a string.", argName);
}

uint32_t calculateRange(WrenVM* vm, ObjRange* range, uint32_t* length,
                        int* step)
{
  *step = 0;

  // Caso límite: se permite un rango vacío al final de una secuencia. De este
  // modo, list[0..-1] y list[0...list.count] se pueden usar para copiar una lista
  // incluso cuando está vacía.
  if (range->from == *length &&
      range->to == (range->isInclusive ? -1.0 : (double)*length))
  {
    *length = 0;
    return 0;
  }

  uint32_t from = validateIndexValue(vm, *length, range->from, "Range start");
  if (from == UINT32_MAX) return UINT32_MAX;

  // Comprueba manualmente los límites del final para manejar rangos exclusivos.
  double value = range->to;
  if (!validateIntValue(vm, value, "Range end")) return UINT32_MAX;

  // Los índices negativos cuentan desde el final.
  if (value < 0) value = *length + value;

  // Convierte el rango exclusivo en uno inclusivo.
  if (!range->isInclusive)
  {
    // Un rango exclusivo con los mismos puntos inicial y final está vacío.
    if (value == from)
    {
      *length = 0;
      return from;
    }

    // Desplaza el extremo para hacerlo inclusivo, manejando tanto rangos
    // crecientes como decrecientes.
    value += value >= from ? -1 : 1;
  }

  // Comprueba los límites.
  if (value < 0 || value >= *length)
  {
    vm->fiber->error = CONST_STRING(vm, "Range end out of bounds.");
    return UINT32_MAX;
  }

  uint32_t to = (uint32_t)value;
  *length = abs((int)(from - to)) + 1;
  *step = from < to ? 1 : -1;
  return from;
}
