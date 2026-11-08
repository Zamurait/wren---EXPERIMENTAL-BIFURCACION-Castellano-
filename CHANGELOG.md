# Registro de cambios

## Versión experimental 0.1.0

Esta versión incluye únicamente:

- Organización del proyecto como preparación para su estudio.
- Traducción al español de gran parte de la documentación del proyecto.

## Versión base del proyecto: 0.4.0

Esta es la versión base inicial sobre la que se inició este proyecto experimental. Los cambios enumerados a continuación corresponden al proyecto base Wren 0.4.0; no son cambios realizados en esta bifurcación.

### Lenguaje

- Se añadió la palabra clave `continue`.
- Se añadió `as`: `import "..." for Name as OtherName`.
- Se añadió compatibilidad con el signo positivo en la notación científica.
- Se añadió `Fiber.try(value)` como complemento de `Fiber.call(value)`.
- Se permite colocar `.` en una línea distinta (para APIs fluidas o de creación encadenada).

### Módulos

- Random: se optimizó `Random.sample`.
- List:
  - Se añadió `list.sort()` y `list.sort {|a, b| ... }` (quicksort).
  - Se añadió `list.swap(index0, index1)` para intercambiar elementos de una lista.
  - Se añadió `list.indexOf(value)` para buscar valores en una lista.
- Num:
  - Se añadió `Num.tau`.
  - Se añadió `Num.nan`.
  - Se añadió `Num.infinity`.
  - Se añadieron `min(other)`, `max(other)` y `clamp(min, max)`.
  - Se añadieron `exp` y `log2`.

### Correcciones

- Se corrigió la corrupción de la pila relacionada con las llamadas a `Fn`.
- Se corrigió un error de desplazamiento de bytes en `CODE_IMPORT_VARIABLE`.
- Se corrigieron algunos casos de corrupción de la pila relacionados con varias llamadas a `wrenInterpret`.
- Se corrigió un fallo que ocurría cuando el recolector de basura recogía un módulo durante una importación.
- Se corrigió que `Bool`, `Num` y `Null` permitieran la creación de subclases, algo no válido.

### API

- **Cambio incompatible:** se añadió `userData` a `wrenReallocateFn`.
- **Cambio incompatible:** se añadió `WrenLoadModuleResult`, que incluye una función de retorno `onComplete` y permite liberar las cadenas de módulos.
- Se añadieron las consultas `wrenHasVariable` y `wrenHasModule` para usarlas con `wrenGetVariable`.
- Se añadió `wrenSetListElement` como complemento de `wrenGetListElement`; ambas funciones admiten índices negativos.
- Se añadieron funciones para Map a la API:
  - `wrenSetSlotNewMap`
  - `wrenGetMapCount`
  - `wrenGetMapContainsKey`
  - `wrenGetMapValue`
  - `wrenSetMapValue`
  - `wrenRemoveMapValue`

### Otros cambios

- Compilación: se añadió `util/generate_docs.py` para regenerar archivos del proyecto.
- VM: se permite el uso de *computed goto* con Clang en Windows.
- VM: el valor predeterminado de `WREN_MAX_TEMP_ROOTS` es 8 (en lugar de 5).
- VM: los tiempos de depuración del recolector de basura se muestran en milisegundos, no en segundos.
