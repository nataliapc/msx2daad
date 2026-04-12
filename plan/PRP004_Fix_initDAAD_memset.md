# PRP004 — BUG-05: `initDAAD` memset de `ramsave` desborda la reserva

- **ID**: PRP004
- **Bug ref**: [REPORT_DAAD_BUGS.md § BUG-05](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad.c:114-115](../src/daad.c#L114-L115)
- **Severidad**: BUG (escritura fuera del buffer `ramsave` durante inicialización → corrupción del heap)
- **Fecha**: 2026-04-12
- **Estado**: Finalizado ✅
- **Resultado**: binario final **19806 → 19758 bytes** (−48 bytes), bastante más de lo estimado (−10/−20). El ahorro real refleja que la multiplicación runtime de `sizeof(Object) * numObjDsc` (×6 sobre un `uint8_t` promovido a 16 bits) generaba una llamada a la rutina de multiplicación de SDCC más el preámbulo de la suma, todo lo cual desaparece al sustituir por la constante `512`.

---

## 1. Resumen

Durante `initDAAD()`, tras reservar 512 bytes para `ramsave`, se invoca un `memset` con una longitud calculada como `1 + 256 + sizeof(Object) * numObjDsc`. Esa expresión es **incorrecta en dos dimensiones**:

1. Usa `sizeof(Object)` (= 6 bytes, estructura completa) cuando en `ramsave` sólo se almacena el campo `.location` de cada objeto (1 byte).
2. Incluye un `+1` inicial sin justificación aparente en el layout del buffer.

El resultado: para cualquier juego con `numObjDsc ≥ 43` el `memset` escribe ceros fuera del bloque de 512 bytes reservado por `malloc`, corrompiendo los siguientes *chunks* del heap (que inmediatamente después se usan para `objects`, `tmpTok`, `tmpMsg` y resto de asignaciones — ver [src/daad.c:117-121](../src/daad.c#L117-L121)).

## 2. Código actual

[src/daad.c:113-121](../src/daad.c#L113-L121):

```c
//Get memory for RAMSAVE
ramsave = (char*)malloc(512);	// 256 bytes for Flags + 256 bytes for Objects location
memset(ramsave, 0, 1+256+sizeof(Object)*hdr->numObjDsc);
//Get memory for objects
objects = (Object*)malloc(sizeof(Object)*hdr->numObjDsc);
nullObject = (Object *) nullObjFake;
//Get memory for tmpTok & tmpMsg
tmpTok = (char*)malloc(32);
tmpMsg = (char*)malloc(TEXT_BUFFER_LEN);
```

## 3. Especificación y layout del buffer

El layout de `ramsave` está fijado por el uso que de él hacen `do_RAMSAVE` y `do_RAMLOAD` tras PRP003 ([src/daad_condacts.c:2002-2041](../src/daad_condacts.c#L2002-L2041)):

```
 offset 0    256         256+numObjDsc     512
 ├── flags ──┼── locs ──┼─── sin usar ──┤
 │ (256 B)   │ (N B)    │  (512-256-N)  │
```

- `ramsave[0..255]` — copia de `flags[]`.
- `ramsave[256..256+numObjDsc-1]` — un byte por objeto: su `.location`.
- `ramsave[256+numObjDsc..511]` — reservado pero no utilizado.

El total **nunca excede 512 bytes**, porque `numObjDsc` es `uint8_t` (≤ 255) → `256 + 255 = 511 ≤ 512`. La reserva de `malloc(512)` es correcta; lo que falla es el `memset` de la línea siguiente.

## 4. Análisis detallado del comportamiento actual

### 4.1. Evaluación de la fórmula

`sizeof(Object) = 6` ([include/daad.h:159-173](../include/daad.h#L159-L173)). Luego:

```
longitud_memset = 1 + 256 + 6 * numObjDsc = 257 + 6 * numObjDsc
```

Tabla de desbordamientos:

| `numObjDsc` | bytes del `memset` | ¿excede 512? | bytes escritos fuera |
|-------------|-------------------:|:------------:|---------------------:|
| 10          | 317                | no           | 0                    |
| 42          | 509                | no           | 0                    |
| **43**      | **515**            | **sí**       | **3**                |
| 50          | 557                | sí           | 45                   |
| 100         | 857                | sí           | 345                  |
| 200         | 1457               | sí           | 945                  |
| 255 (máx.)  | 1787               | sí           | 1275                 |

En juegos reales con decenas de objetos el desbordamiento es sistemático.

### 4.2. Qué se corrompe

Inmediatamente después del `memset`, el mismo `initDAAD` hace:

1. `objects = malloc(sizeof(Object)*numObjDsc)` ([src/daad.c:117](../src/daad.c#L117))
2. `tmpTok = malloc(32)` ([src/daad.c:120](../src/daad.c#L120))
3. `tmpMsg = malloc(TEXT_BUFFER_LEN)` ([src/daad.c:121](../src/daad.c#L121))

El `malloc` local ([src/heap.c:6-10](../src/heap.c#L6-L10)) es un *bump allocator*: los bloques se colocan **consecutivamente** tras el anterior. El `memset` que desborda `ramsave` escribe ceros sobre lo que será la siguiente área del heap.

Pero hay un detalle temporal crítico: el desbordamiento ocurre **antes** de que los `malloc` posteriores se llamen. En el instante del `memset`, la zona más allá de `ramsave` está *libre* (no asignada aún). Escribir ceros allí es **inocuo en ese momento**. Sin embargo:

- Las siguientes llamadas a `malloc` devuelven punteros al mismo rango, que acaba de ser "puesto a cero" por el desbordamiento del `memset` anterior.
- `objects` queda inicializado implícitamente a 0 (lo cual sería deseable, pero es accidental).
- `tmpTok` y `tmpMsg` quedan a 0 (también por accidente).

**Consecuencia práctica**: la corrupción "no se nota" en la ruta normal porque lo que se sobrescribe aún no es datos del programa. Pero:

- Si alguna operación anterior al `malloc` de `objects` hubiera dejado datos significativos en esa zona (por ejemplo, cambios futuros en `initDAAD`), se perderían.
- En configuraciones donde el heap esté en la frontera con otras estructuras del sistema (ROM slots, área de DOS, VRAM mapeada, etc.), el desbordamiento podría tocar memoria sensible.
- El comportamiento es **indeterminado por estándar C**: escribir fuera de una asignación es UB, y cualquier reorganización futura del heap podría convertir el bug latente en uno observable.

### 4.3. Análisis de la fórmula errónea

Dos errores independientes:

1. **`sizeof(Object)` vs 1 byte**. El autor parece haber pensado "reservo el `Object` completo", pero `ramsave` sólo guarda el byte `.location` de cada objeto (así lo lee/escribe `do_RAMSAVE`/`do_RAMLOAD`). La fórmula correcta en número de bytes útiles es `256 + 1 * numObjDsc`.
2. **El `+1` inicial**. No corresponde a nada en el layout: no hay un byte de cabecera, no hay flag cero reservado separadamente, etc. Es muy probablemente un residuo de una versión anterior de código (quizá un *tamaño + 1 para null-terminator* trasladado sin sentido desde una cadena). Debe eliminarse.

## 5. Análisis de optimización (Z80/SDCC)

### 5.1. `memset` con longitud constante vs variable

`memset(ramsave, 0, 512)` — longitud constante de 16 bits — compila a un `LD HL,#0x0200` (3 bytes) + push y call a la implementación de `memset`. Ligeramente más barato que cargar `numObjDsc`, extender a 16 bits, sumar 256 y pushear.

### 5.2. `memset` de 512 vs 256+numObjDsc

Ambas son funcionalmente válidas (ver §3). En Z80/SDCC la implementación estándar de `memset` itera bytes usando `LDIR` o equivalente, por lo que la diferencia de rendimiento entre `512` y `256+numObjDsc` es proporcional a los bytes extra puestos a cero (≤ 255 bytes). En el código `initDAAD` que se llama una sola vez al inicio, ese coste en ciclos es despreciable.

El código generado es más pequeño con la constante 512 (el cálculo `256 + uint8_t` obliga a una promoción y suma adicional). Además, limpiar toda la reserva es el comportamiento más natural y defensivo.

### 5.3. ¿Merece la pena un `calloc`?

No existe `calloc` en [src/heap.c](../src/heap.c) (sólo `malloc`/`free` bump allocator). Añadirlo sólo para este caso aportaría una función nueva para un único *callsite*. Descartado por §5.4.

### 5.4. Elección preferida

**`memset(ramsave, 0, 512)`**:

- Longitud constante → código más pequeño.
- Limpia toda la reserva → defensivo y documentable ("clear the entire buffer").
- No requiere cambios en el resto del archivo.

## 6. Solución propuesta

```diff
 //Get memory for RAMSAVE
 ramsave = (char*)malloc(512);	// 256 bytes for Flags + 256 bytes for Objects location
-memset(ramsave, 0, 1+256+sizeof(Object)*hdr->numObjDsc);
+memset(ramsave, 0, 512);
```

### 6.1. Verificación de corrección

- **Rango del `memset`**: `[ramsave, ramsave+512)` — coincide exactamente con la reserva `malloc(512)`. No hay escritura fuera del bloque. ✓
- **Contenido resultante**: los 512 bytes quedan a cero, incluyendo el área usada (`[0..256+numObjDsc-1]`) y el resto no usado. Esto cubre el propósito original: si un juego ejecuta `RAMLOAD` antes de cualquier `RAMSAVE`, lee ceros en lugar de basura del heap. ✓
- **Compatibilidad con PRP003**: `do_RAMSAVE` y `do_RAMLOAD` sólo leen/escriben `[0..256+numObjDsc-1]`, que queda contenido en los 512 bytes inicializados. Ninguna interacción negativa. ✓
- **Compatibilidad con `malloc` subsiguiente**: al no desbordarse, los bloques `objects`, `tmpTok`, `tmpMsg` que se asignan después **ya no se pisan**. Eso implica que:
  - `objects` ya **no** queda accidentalmente a cero: su inicialización real pasa a ser responsabilidad de `initObjects()` ([src/daad.c:188-206](../src/daad.c#L188-L206)), que rellena todos los campos de cada objeto (ver §8).
  - `tmpTok` y `tmpMsg` quedan con contenido indeterminado, pero se usan siempre como buffers de escritura antes de ser leídos, por lo que no importa.

### 6.2. Impacto real de tamaño

| versión                              | bytes totales | Δ vs. estado previo |
|--------------------------------------|---------------|---------------------|
| Tras PRP003 (memset buggy)           | 19806         | — (baseline)        |
| Tras PRP004 (solución adoptada)      | 19758         | **−48 bytes**       |

El ahorro supera con creces la estimación inicial (−10 a −20). Desglose:

- Desaparece la **lectura indirecta** de `hdr->numObjDsc` (`LD HL,(hdr) / LD A,(HL+3)` o similar).
- Desaparece la **promoción a 16 bits** (`LD H,0`).
- Desaparece la **multiplicación por 6** — que en SDCC/Z80 sobre enteros de 16 bits no se inlinea a shifts porque 6 no es potencia de 2: genera una llamada a la rutina de multiplicación (`__mulint` o equivalente), varios bytes extra en cada *callsite*.
- Desaparecen dos sumas de constantes (`+256`, `+1`) y sus pushes.

Sustituir todo el cálculo por una constante `512` reduce la carga del argumento de `memset` a un único `LD HL,#0x0200`.

## 7. Alternativas consideradas

### 7.1. `memset(ramsave, 0, 256 + hdr->numObjDsc)`

```c
memset(ramsave, 0, 256 + hdr->numObjDsc);
```

- Correcto: limpia sólo el área semánticamente utilizada.
- Ligeramente más grande que §6 (requiere cargar `numObjDsc` y sumar 256 en runtime).
- No aporta ningún beneficio frente a §6 — los bytes extra zerados por §6 están dentro de la misma reserva y no cuestan nada semánticamente.
- **Descartada**: §6 es equivalente o mejor en todos los aspectos.

### 7.2. Mantener el `memset` solo si se arregla la fórmula al correcto `256 + 1 * numObjDsc`

- Es §7.1 por otra ruta.
- Descartada por el mismo motivo.

### 7.3. Eliminar el `memset` completamente

```c
ramsave = (char*)malloc(512);
```

- **Descartada**: aunque la ruta normal sea `RAMSAVE` → `RAMLOAD`, un juego puede ejecutar `RAMLOAD` sin haber hecho `RAMSAVE` previamente. El buffer contendría entonces basura del heap (típicamente ceros del *bump allocator* sobre memoria recién inicializada, pero no garantizado). Mantener el zero-init es la postura defensiva.

### 7.4. Sustituir `malloc+memset` por un hipotético `calloc`

- Requiere añadir `calloc` a [src/heap.c](../src/heap.c) para un único *callsite*. No merece la pena.
- Descartada.

## 8. Riesgos y compatibilidad

### 8.1. Cambio en la inicialización de `objects`, `tmpTok`, `tmpMsg`

Este es el punto más delicado. Antes del fix:

1. `memset` desbordante → pone a cero la región que pronto será `objects`, `tmpTok`, `tmpMsg`.
2. `malloc(objects)` → devuelve puntero a zona ya zerada por el desbordamiento.
3. `initObjects()` sobrescribe todos los campos de cada `Object`.

Tras el fix:

1. `memset` sólo limpia los 512 bytes de `ramsave`.
2. `malloc(objects)` → devuelve puntero a memoria **no zerada** (contenido del heap en ese momento — típicamente datos previos o residuos).
3. `initObjects()` sobrescribe todos los campos: `location`, `attribs.byte`, `extAttr1`, `extAttr2`, `nounId`, `adjectiveId`. Los 6 bytes de cada `Object` se asignan explícitamente en [src/daad.c:197-205](../src/daad.c#L197-L205). Ningún campo queda sin inicializar.

Por tanto el cambio es **seguro**: `initObjects()` no dependía del zero-init accidental. Análogo para `tmpTok`/`tmpMsg` (son buffers de escritura siempre).

### 8.2. Interacción con otros PRPs

- **PRP003** (RAMSAVE/RAMLOAD): ortogonal. Ahora ambas funciones respetan el rango correcto (`numObjDsc` en lugar de 256), y el buffer está bien inicializado.
- **BUG-06** (initFlags con `sizeof(windows)` pointer bug): independiente, sigue pendiente.

### 8.3. ABI y plataformas

- Sin cambios de ABI.
- Aplica por igual a MSX2, DOS, CPM y cualquier otro *target*. `memset` es estándar.

## 9. Plan de verificación

1. **Inspección estática** ✅: la línea 115 queda como `memset(ramsave, 0, 512);` ([src/daad.c:115](../src/daad.c#L115)). El comentario de la línea 114 no se altera.
2. **Compilación** ✅: rebuild completo sin warnings nuevos.
3. **Medida de tamaño** ✅: binario final 19806 → **19758 bytes** (−48 bytes). Ver tabla en §6.2.
4. **Tests unitarios** ✅: batería completa ejecutada en openmsx → **204 OK / 55 TODO / 0 fallos** (ver [unitTests/dsk/tests.txt](../unitTests/dsk/tests.txt)). `initDAAD` está mockeado con `bool initDAAD() {}` en [unitTests/src/tests_condacts.c:96](../unitTests/src/tests_condacts.c#L96), por lo que esta batería no ejercita directamente el fix de este PRP. Lo relevante: los tests que cubren los 3 PRPs anteriores (`test_CHANCE_*`, `test_DROPALL_success`, `test_RAMSAVE_success`, `test_RAMLOAD_success`, `test_SAVE_LOAD_success`) siguen pasando — no hay regresión.
5. **Prueba funcional con DDB real** (pendiente):
   - Cargar un juego con `numObjDsc ≥ 43` (p.ej. cualquier aventura comercial).
   - Arrancar el juego; comprobar que no se produce *crash* ni comportamiento extraño en los primeros turnos (antes del fix la corrupción es latente pero existente; tras el fix debe desaparecer).
   - Probar `RAMLOAD` sin `RAMSAVE` previo: el juego debe leer ceros (flags = 0, `objects[*].location = 0`), no basura.
6. **Inspección de heap** (opcional, build debug): colocar canarios inmediatamente después de `ramsave` (en la zona del siguiente `malloc`); verificar que tras `initDAAD` no están alterados (cosa que antes del fix sí ocurría).

## 10. Referencias

- Informe: [REPORT_DAAD_BUGS.md § BUG-05](../REPORT_DAAD_BUGS.md)
- Código afectado: [src/daad.c:114-115](../src/daad.c#L114-L115)
- Layout de `ramsave` consolidado: [PRP003_Fix_RAMSAVE_RAMLOAD.md](PRP003_Fix_RAMSAVE_RAMLOAD.md) §3
- Uso correcto del buffer: [src/daad_condacts.c:2002-2041](../src/daad_condacts.c#L2002-L2041) (tras PRP003)
- Heap allocator: [src/heap.c](../src/heap.c)
- Inicialización real de `objects`: [src/daad.c:188-206](../src/daad.c#L188-L206) (`initObjects`)
- Estructura `Object` (sizeof = 6): [include/daad.h:159-173](../include/daad.h#L159-L173)
