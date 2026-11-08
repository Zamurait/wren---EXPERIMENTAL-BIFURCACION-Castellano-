#ifndef wren_opt_meta_h
#define wren_opt_meta_h

#include "wren_common.h"
#include "wren.h"

// Este módulo define la clase Meta y sus métodos asociados.
#if WREN_OPT_META

const char* wrenMetaSource();
WrenForeignMethodFn wrenMetaBindForeignMethod(WrenVM* vm,
                                               const char* className,
                                               bool isStatic,
                                               const char* signature);

#endif

#endif
