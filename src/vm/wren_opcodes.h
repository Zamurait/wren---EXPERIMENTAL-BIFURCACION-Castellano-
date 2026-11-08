// Define las instrucciones de bytecode que usa la VM. Para ello, invoca una
// macro OPCODE() que debe estar definida en el punto donde se incluye este
// archivo. (Véase: http://en.wikipedia.org/wiki/X_Macro para más información.)
//
// El primer argumento es el nombre del opcode. El segundo es su «efecto sobre
// la pila»: cuánto cambia el tamaño de la pila la instrucción. Un efecto de 1
// significa que inserta un valor y la pila crece en uno. -2 significa que quita
// dos valores, etc.
//
// Ten en cuenta que el orden de las instrucciones afecta el orden de la tabla de
// despacho del bucle intérprete de la VM. Esto, a su vez, afecta la caché y, por
// tanto, al rendimiento general. Si cambias el orden, asegúrate de ejecutar
// pruebas de rendimiento.

// Carga la constante en el índice [arg].
OPCODE(CONSTANT, 1)

// Inserta null en la pila.
OPCODE(NULL, 1)

// Inserta false en la pila.
OPCODE(FALSE, 1)

// Inserta true en la pila.
OPCODE(TRUE, 1)

// Inserta el valor de la ranura local indicada.
OPCODE(LOAD_LOCAL_0, 1)
OPCODE(LOAD_LOCAL_1, 1)
OPCODE(LOAD_LOCAL_2, 1)
OPCODE(LOAD_LOCAL_3, 1)
OPCODE(LOAD_LOCAL_4, 1)
OPCODE(LOAD_LOCAL_5, 1)
OPCODE(LOAD_LOCAL_6, 1)
OPCODE(LOAD_LOCAL_7, 1)
OPCODE(LOAD_LOCAL_8, 1)

// Nota: el compilador supone que las siguientes instrucciones _STORE siempre
// aparecen inmediatamente después de sus correspondientes instrucciones _LOAD.

// Inserta en la pila el valor de la ranura local [arg].
OPCODE(LOAD_LOCAL, 1)

// Guarda la parte superior de la pila en la ranura local [arg]. No la quita.
OPCODE(STORE_LOCAL, 0)

// Inserta en la pila el valor del upvalue [arg].
OPCODE(LOAD_UPVALUE, 1)

// Guarda la parte superior de la pila en el upvalue [arg]. No la quita.
OPCODE(STORE_UPVALUE, 0)

// Inserta en la pila el valor de la variable de nivel superior en la ranura [arg].
OPCODE(LOAD_MODULE_VAR, 1)

// Guarda la parte superior de la pila en la ranura [arg] de la variable de nivel
// superior. No la quita.
OPCODE(STORE_MODULE_VAR, 0)

// Inserta el valor del campo en la ranura [arg] del receptor de la función
// actual. Se usa para accesos normales a campos de «this» directamente en
// métodos. Esta instrucción es más rápida que la instrucción más general
// CODE_LOAD_FIELD.
OPCODE(LOAD_FIELD_THIS, 1)

// Guarda la parte superior de la pila en el campo de la ranura [arg] del
// receptor actual. No quita el valor. Esta instrucción es más rápida que la
// instrucción más general CODE_LOAD_FIELD.
OPCODE(STORE_FIELD_THIS, 0)

// Quita una instancia de la pila e inserta su valor de campo en la ranura [arg].
OPCODE(LOAD_FIELD, 0)

// Quita una instancia de la pila y guarda en ella el siguiente valor de la parte
// superior de la pila, en el campo de la ranura [arg]. No quita el valor.
OPCODE(STORE_FIELD, -1)

// Quita y descarta el valor de la parte superior de la pila.
OPCODE(POP, -1)

// Invoca el método con el símbolo [arg]. El número indica cuántos argumentos
// tiene (sin incluir el receptor).
OPCODE(CALL_0, 0)
OPCODE(CALL_1, -1)
OPCODE(CALL_2, -2)
OPCODE(CALL_3, -3)
OPCODE(CALL_4, -4)
OPCODE(CALL_5, -5)
OPCODE(CALL_6, -6)
OPCODE(CALL_7, -7)
OPCODE(CALL_8, -8)
OPCODE(CALL_9, -9)
OPCODE(CALL_10, -10)
OPCODE(CALL_11, -11)
OPCODE(CALL_12, -12)
OPCODE(CALL_13, -13)
OPCODE(CALL_14, -14)
OPCODE(CALL_15, -15)
OPCODE(CALL_16, -16)

// Invoca un método de superclase con el símbolo [arg]. El número indica cuántos
// argumentos tiene (sin incluir el receptor).
OPCODE(SUPER_0, 0)
OPCODE(SUPER_1, -1)
OPCODE(SUPER_2, -2)
OPCODE(SUPER_3, -3)
OPCODE(SUPER_4, -4)
OPCODE(SUPER_5, -5)
OPCODE(SUPER_6, -6)
OPCODE(SUPER_7, -7)
OPCODE(SUPER_8, -8)
OPCODE(SUPER_9, -9)
OPCODE(SUPER_10, -10)
OPCODE(SUPER_11, -11)
OPCODE(SUPER_12, -12)
OPCODE(SUPER_13, -13)
OPCODE(SUPER_14, -14)
OPCODE(SUPER_15, -15)
OPCODE(SUPER_16, -16)

// Avanza [arg] posiciones el puntero de instrucción.
OPCODE(JUMP, 0)

// Retrocede [arg] posiciones el puntero de instrucción.
OPCODE(LOOP, 0)

// Quita un valor y, si no es verdadero, avanza [arg] posiciones el puntero de
// instrucción.
OPCODE(JUMP_IF, -1)

// Si el valor de la parte superior de la pila es false, avanza [arg] posiciones.
// De lo contrario, lo quita y continúa.
OPCODE(AND, -1)

// Si el valor de la parte superior de la pila no es false, avanza [arg]
// posiciones. De lo contrario, lo quita y continúa.
OPCODE(OR, -1)

// Cierra el upvalue de la variable local en la parte superior de la pila y luego
// lo quita.
OPCODE(CLOSE_UPVALUE, -1)

// Sale de la función actual y devuelve el valor de la parte superior de la pila.
OPCODE(RETURN, 0)

// Crea una clausura para la función almacenada en [arg] de la tabla de
// constantes.
//
// Después del argumento de función hay una serie de argumentos, dos por cada
// upvalue. El primero es true si la variable capturada es local (en lugar de un
// upvalue), y el segundo es el índice de la variable local o del upvalue que se
// captura.
//
// Inserta la clausura creada en la pila.
OPCODE(CLOSURE, 1)

// Crea una nueva instancia de una clase.
//
// Supone que el objeto de clase está en la ranura cero y lo reemplaza por la
// nueva instancia no inicializada de esa clase. Esta instrucción solo la emiten
// los métodos de metaclase del constructor generados por el compilador.
OPCODE(CONSTRUCT, 0)

// Crea una nueva instancia de una clase externa.
//
// Supone que el objeto de clase está en la ranura cero y lo reemplaza por la
// nueva instancia no inicializada de esa clase. Esta instrucción solo la emiten
// los métodos de metaclase del constructor generados por el compilador.
OPCODE(FOREIGN_CONSTRUCT, 0)

// Crea una clase. La parte superior de la pila contiene la superclase. Debajo hay
// una cadena con el nombre de la clase. El byte [arg] indica el número de campos
// de la clase.
OPCODE(CLASS, -1)

// Termina una clase.
// En este momento la pila contiene la clase y ClassAttributes (o null).
OPCODE(END_CLASS, -2)

// Crea una clase externa. La parte superior de la pila contiene la superclase.
// Debajo hay una cadena con el nombre de la clase.
OPCODE(FOREIGN_CLASS, -1)

// Define el método para el símbolo [arg]. Se quita de la pila la clase que
// recibirá el método y luego se quita la función que define su cuerpo.
//
// Si se está definiendo un método externo, la «función» será una cadena que lo
// identifica. De lo contrario, será una función o clausura.
OPCODE(METHOD_INSTANCE, -2)

// Define el método para el símbolo [arg]. Se quita de la pila la clase cuya
// metaclase recibirá el método y luego se quita la función que define su cuerpo.
//
// Si se está definiendo un método externo, la «función» será una cadena que lo
// identifica. De lo contrario, será una función o clausura.
OPCODE(METHOD_STATIC, -2)

// Se ejecuta al final del cuerpo del módulo. Inserta NULL en la pila como «valor
// de retorno» de la sentencia de importación y almacena el módulo como el
// importado más recientemente.
OPCODE(END_MODULE, 1)

// Importa el módulo cuyo nombre es la cadena almacenada en [arg] de la tabla de
// constantes.
//
// Inserta null en la pila para que la fibra del módulo importado lo reemplace por
// un valor ficticio al regresar. (Las fibras siempre devuelven un valor al
// reanudar a quien las llamó).
OPCODE(IMPORT_MODULE, 1)

// Importa una variable del módulo importado más recientemente. El nombre de la
// variable está en [arg] de la tabla de constantes. Inserta en la pila el valor
// cargado de la variable.
OPCODE(IMPORT_VARIABLE, 1)

// Esta seudoinstrucción indica el final del bytecode. Siempre debe estar
// precedida por `CODE_RETURN`, por lo que nunca se ejecuta realmente.
OPCODE(END, 0)
