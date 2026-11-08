#include "wren_opt_random.h"

#if WREN_OPT_RANDOM

#include <string.h>
#include <time.h>

#include "wren.h"
#include "wren_vm.h"

#include "wren_opt_random.wren.inc"

// Implementa el generador de números pseudoaleatorios lineal de período largo
// y buena equidistribución (WELL512a).
//
// https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear
typedef struct
{
  uint32_t state[16];
  uint32_t index;
} Well512;

// Código de: http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
static uint32_t advanceState(Well512* well)
{
  uint32_t a, b, c, d;
  a = well->state[well->index];
  c = well->state[(well->index + 13) & 15];
  b =  a ^ c ^ (a << 16) ^ (c << 15);
  c = well->state[(well->index + 9) & 15];
  c ^= (c >> 11);
  a = well->state[well->index] = b ^ c;
  d = a ^ ((a << 5) & 0xda442d24U);

  well->index = (well->index + 15) & 15;
  a = well->state[well->index];
  well->state[well->index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
  return well->state[well->index];
}

static void randomAllocate(WrenVM* vm)
{
  Well512* well = (Well512*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Well512));
  well->index = 0;
}

static void randomSeed0(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

  srand((uint32_t)time(NULL));
  for (int i = 0; i < 16; i++)
  {
    well->state[i] = rand();
  }
}

static void randomSeed1(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

  srand((uint32_t)wrenGetSlotDouble(vm, 1));
  for (int i = 0; i < 16; i++)
  {
    well->state[i] = rand();
  }
}

static void randomSeed16(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

  for (int i = 0; i < 16; i++)
  {
    well->state[i] = (uint32_t)wrenGetSlotDouble(vm, i + 1);
  }
}

static void randomFloat(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

  // Un double tiene 53 bits de precisión en su mantisa y queremos aprovecharlos
  // por completo, así que necesitamos 53 bits de datos de origen aleatorio.

  // Primero, empieza con 32 bits aleatorios desplazados 21 bits a la izquierda.
  double result = (double)advanceState(well) * (1 << 21);

  // Luego añade otros 21 bits aleatorios.
  result += (double)(advanceState(well) & ((1 << 21) - 1));

  // Ahora tenemos un número de 0 a (2^53). Divide por el rango para obtener un
  // double de 0 a 1.0 (incluye el extremo inferior).
  result /= 9007199254740992.0;

  wrenSetSlotDouble(vm, 0, result);
}

static void randomInt0(WrenVM* vm)
{
  Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

  wrenSetSlotDouble(vm, 0, (double)advanceState(well));
}

const char* wrenRandomSource()
{
  return randomModuleSource;
}

WrenForeignClassMethods wrenRandomBindForeignClass(WrenVM* vm,
                                                   const char* module,
                                                   const char* className)
{
  ASSERT(strcmp(className, "Random") == 0, "Should be in Random class.");
  WrenForeignClassMethods methods;
  methods.allocate = randomAllocate;
  methods.finalize = NULL;
  return methods;
}

WrenForeignMethodFn wrenRandomBindForeignMethod(WrenVM* vm,
                                                const char* className,
                                                bool isStatic,
                                                const char* signature)
{
  ASSERT(strcmp(className, "Random") == 0, "Should be in Random class.");
  
  if (strcmp(signature, "<allocate>") == 0) return randomAllocate;
  if (strcmp(signature, "seed_()") == 0) return randomSeed0;
  if (strcmp(signature, "seed_(_)") == 0) return randomSeed1;
  
  if (strcmp(signature, "seed_(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)") == 0)
  {
    return randomSeed16;
  }
  
  if (strcmp(signature, "float()") == 0) return randomFloat;
  if (strcmp(signature, "int()") == 0) return randomInt0;
  
  ASSERT(false, "Unknown method.");
  return NULL;
}

#endif
