# PRP005 — BUG-06: `initFlags` usa `sizeof(windows)` (puntero) en lugar de `sizeof(Window)` (struct)

- **ID**: PRP005
- **Bug ref**: [REPORT_DAAD_BUGS.md § BUG-06](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad.c:154-155](../src/daad.c#L154-L155)
- **Severidad**: BUG (inicialización incompleta de estructuras `Window` → `cursorX`/`cursorY` con basura del heap)
- **Fecha**: 2026-04-12
- **Estado**: Finalizado ✅
- **Resultado**: binario final **19758 → 19739 bytes** (−19 bytes), más del doble de la estimación (−4 a −9). El ahorro extra refleja que SDCC emitía más de lo esperado para `cw->winX = cw->winY = 0;` sobre un puntero cargado en iteración (probable uso de registros IX/IY con múltiples `LD (IX+n),#0` y los setups asociados).

---

## 1. Resumen

En `initFlags()`, tras reservar el array de ventanas se invoca un `memset` con la longitud `sizeof(windows)*WINDOWS_NUM`. `windows` es un **puntero** (`Window*`), no el tipo de la estructura, por lo que `sizeof(windows)` devuelve el tamaño del puntero (2 bytes en Z80) en lugar del tamaño de `Window` (11 bytes). El memset deja sin limpiar la mayor parte del array, y los campos `cursorX`/`cursorY` —los únicos que dependían de ese zero-init— quedan con basura del heap.

## 2. Código actual

[src/daad.c:144-179](../src/daad.c#L144-L179):

```c
void initFlags()
{
	uint8_t i;

	//Clear flag of player location
	flags[fPlayer] = 0;

	gfxSetScreenModeFlags();

	//Initialize DAAD windows
	windows = malloc(sizeof(Window)*WINDOWS_NUM);
	memset(windows, 0, sizeof(windows)*WINDOWS_NUM);
	flags[fCurWin] = 0;
	for (i=0; i<WINDOWS_NUM; i++) {
		cw = &windows[i];
		cw->winX = cw->winY = 0;
		cw->winW = MAX_COLUMNS;
		cw->winH = MAX_LINES;
		cw->mode = 0;
		cw->lastPicLocation = 256;
		gfxSetPaperCol(0);
		gfxSetInkCol(15);
	}
	cw = &windows[0];
	...
}
```

## 3. Tipos y constantes involucrados

- `Window` ([include/daad.h:145-156](../include/daad.h#L145-L156)):

```c
typedef struct {
	uint8_t  winX;
	uint8_t  winY;
	uint8_t  winW;
	uint8_t  winH;
	uint8_t  cursorX;
	uint8_t  cursorY;
	uint8_t  mode;
	uint8_t  paper;
	uint8_t  ink;
	uint16_t lastPicLocation;
} Window;
```

Sin padding en Z80/SDCC: `sizeof(Window) = 9*1 + 1*2 = 11 bytes`.

- `WINDOWS_NUM` = 8 si `DISABLE_WINDOW` no está definido, 1 en caso contrario ([include/daad.h:24-28](../include/daad.h#L24-L28)).
- `windows` es de tipo `Window*` (declarado como tal); `sizeof(Window*) = 2` en Z80.

## 4. Análisis detallado del comportamiento actual

### 4.1. Bytes realmente puestos a cero

| modo            | `sizeof(windows)` | `WINDOWS_NUM` | bytes limpiados (buggy) | bytes que debería limpiar |
|-----------------|------------------:|--------------:|------------------------:|--------------------------:|
| MSX2 (normal)   | 2                 | 8             | **16**                  | 88                        |
| `DISABLE_WINDOW`| 2                 | 1             | **2**                   | 11                        |

En el modo normal, el `memset` buggy limpia los primeros **16 bytes** del array: cubre completamente `windows[0]` (11 B) y escribe 5 bytes adicionales en `windows[1]` (los primeros campos: `winX, winY, winW, winH, cursorX`). Todo lo demás del array queda con el contenido aleatorio del heap.

### 4.2. ¿Qué campos quedan sin inicializar en la práctica?

El bucle `for` siguiente al memset recorre los 8 windows y, para cada uno, asigna:

- `winX`, `winY` → 0
- `winW` → `MAX_COLUMNS`
- `winH` → `MAX_LINES`
- `mode` → 0
- `lastPicLocation` → 256
- `paper`, `ink` → mediante `gfxSetPaperCol(0)` / `gfxSetInkCol(15)`, que en modos gráficos estándar expanden a `cw->paper = …` / `cw->ink = …` (ver [src/daad_platform_msx2.c:387-388](../src/daad_platform_msx2.c#L387-L388)).

Los **únicos campos que no toca el bucle** son `cursorX` y `cursorY`. Éstos dependen íntegramente del `memset` para quedar a 0.

### 4.3. Consecuencias observables

Con el `memset` buggy:

- `windows[0].cursorX/cursorY`: zerados por accidente (los primeros 11 bytes caen dentro de los 16 limpiados).
- `windows[1].cursorX/cursorY`: valor **indeterminado** — los 16 bytes iniciales cubren `winX..cursorX` de `windows[1]`, pero `cursorY` y adelante están en basura. De hecho `cursorX` de `windows[1]` sí queda a 0, pero `cursorY` no.
- `windows[2..7].cursorX/cursorY`: **todos con basura del heap**.

En tiempo de ejecución:

- La primera impresión sobre `windows[0]` funciona (cursor a 0).
- Un `WINDOW n` (con `n ≥ 2`) sin un `CLS` previo imprime desde una posición de cursor aleatoria — texto fuera de la caja de la ventana, scrolls espurios, o (si `cursorY ≥ winH`) ciclos de *scroll* inmediatos.
- Los condacts `CLS`, `WINAT`, `PRINTAT`, `WINDOW` que asignan `cw->cursorX = cw->cursorY = 0` ([src/daad_condacts.c:1677, 1694, 1719](../src/daad_condacts.c#L1677)) enmascaran el bug al ser invocados, pero entre la inicialización y el primer reset manual, cualquier ventana distinta de la 0 tiene cursor indefinido.

### 4.4. Causa raíz

El idioma C correcto es `sizeof(*windows)` o —equivalente— `sizeof(Window)`. Escribir `sizeof(windows)` es un error común que SDCC no detecta ni avisa: es una expresión válida (el tamaño del puntero) con valor constante pequeño, que pasa desapercibido porque "algo limpia".

## 5. Análisis de optimización (Z80/SDCC)

### 5.1. `sizeof(Window) * WINDOWS_NUM` es constante en compilación

Tanto `sizeof(Window) = 11` como `WINDOWS_NUM = 8` son constantes conocidas en compilación. El producto `88` se pliega por el *front-end* de SDCC a un inmediato de 16 bits (`LD HL,#88`) en el *callsite* de `memset`. Coste del argumento: 3 bytes para el inmediato + un push — lo mismo que la versión buggy actual (`2*8 = 16`, también constante).

Por tanto, el fix mínimo (`sizeof(windows)` → `sizeof(Window)`) no introduce *ningún* coste de tamaño adicional sobre la versión actual: la única diferencia es el valor del inmediato (88 en lugar de 16), misma cantidad de bytes de código.

### 5.2. Qué campos del bucle son zero-inits "redundantes" tras arreglar el memset

Con el `memset` corregido, todos los `Window` quedan a 0 antes de entrar al bucle. El bucle asigna:

| asignación en el bucle               | ¿la cubre el memset correcto? | ¿puede quitarse? |
|--------------------------------------|:-----------------------------:|:----------------:|
| `cw->winX = cw->winY = 0`            | sí (valor 0)                  | **sí**           |
| `cw->winW = MAX_COLUMNS`              | no (valor ≠ 0)                | no               |
| `cw->winH = MAX_LINES`                | no (valor ≠ 0)                | no               |
| `cw->mode = 0`                       | sí (valor 0)                  | **sí**           |
| `cw->lastPicLocation = 256`          | no (valor ≠ 0)                | no               |
| `gfxSetPaperCol(0)` (→ `cw->paper`)  | **no**                        | no               |
| `gfxSetInkCol(15)` (→ `cw->ink`)     | **no**                        | no               |

Nota sobre `gfxSetPaperCol(0)` / `gfxSetInkCol(15)`: aunque los argumentos sean 0 y 15, la asignación real a `cw->paper` / `cw->ink` pasa por `getColor()` ([src/daad_platform_msx2.c:445-460](../src/daad_platform_msx2.c#L445-L460)), que en SCREEN 8/10 devuelve valores **no nulos** incluso para el argumento 0 (por ejemplo, SCREEN 10: `getColor(0) = 8`). Además tienen efectos secundarios fuera del struct. Deben quedarse.

Se pueden eliminar del bucle **dos asignaciones** (`winX/winY` y `mode`) sin perder corrección, al estar ya garantizadas por el `memset`.

### 5.3. Comparación de tamaños (Z80/SDCC)

| opción | `memset` | bucle | Δ tamaño estimado vs. buggy | notas                        |
|--------|----------|-------|-----------------------------|-------------------------------|
| A: sólo arreglar `sizeof(windows)` → `sizeof(Window)` | mantiene | sin cambios | ±0 (mismo inmediato, 16→88) | mantiene dependencia implícita |
| B: sin `memset`, init explícito `cursorX/cursorY`    | elimina  | +1 asignación | −10 a −14 B           | explícito, más agresivo       |
| **C: `memset` correcto + quitar zero-assignments del bucle** | mantiene corregido | −2 asignaciones | **−4 a −6 B** | idiomático y resiliente       |

### 5.4. Elección preferida

**Opción C**. Razones:

- El `memset` queda como **fuente única** de zero-init de todos los campos de `Window`. Si en el futuro se añade un campo nuevo a `Window` cuyo valor por defecto sea 0, **no hay que acordarse de inicializarlo en el bucle**: el memset lo cubre automáticamente. Esta propiedad es valiosa para mantenimiento.
- Se quitan del bucle exactamente las asignaciones redundantes (`winX/winY`, `mode`), lo que reduce tamaño.
- El patrón `malloc + memset + init-no-cero-en-bucle` es el idioma estándar de inicialización de structs en C.

La opción B queda documentada como alternativa (más agresiva en tamaño, más explícita campo a campo, pero menos resiliente a cambios futuros de la struct).

## 6. Solución propuesta

Corregir el `memset` y eliminar del bucle las asignaciones a 0 que ya cubre el memset. Los campos que requieren valores no nulos (o que tienen efectos secundarios) se mantienen.

```diff
 	//Initialize DAAD windows
 	windows = malloc(sizeof(Window)*WINDOWS_NUM);
-	memset(windows, 0, sizeof(windows)*WINDOWS_NUM);
+	memset(windows, 0, sizeof(Window)*WINDOWS_NUM);
 	flags[fCurWin] = 0;
 	for (i=0; i<WINDOWS_NUM; i++) {
 		cw = &windows[i];
-		cw->winX = cw->winY = 0;
 		cw->winW = MAX_COLUMNS;
 		cw->winH = MAX_LINES;
-		cw->mode = 0;
 		cw->lastPicLocation = 256;
 		gfxSetPaperCol(0);
 		gfxSetInkCol(15);
 	}
```

### 6.1. Verificación de corrección

Tras el `memset` + bucle, cada `Window` queda con:

| campo            | valor                       | fuente                                |
|------------------|-----------------------------|----------------------------------------|
| `winX`, `winY`   | 0                           | `memset` ✓                             |
| `winW`           | `MAX_COLUMNS`               | asignación explícita en bucle ✓        |
| `winH`           | `MAX_LINES`                 | asignación explícita en bucle ✓        |
| `cursorX`, `cursorY` | **0**                   | **`memset` ✓** (antes: basura)         |
| `mode`           | 0                           | `memset` ✓                             |
| `paper`          | `getColor(0)` (modo gráfico normal) | `gfxSetPaperCol(0)` en bucle ✓ |
| `ink`            | `getColor(15)` ("            ") | `gfxSetInkCol(15)` en bucle ✓      |
| `lastPicLocation`| 256                         | asignación explícita en bucle ✓        |

Todos los campos quedan definidos. Los campos con valor 0 se cubren por el `memset` (ya correcto); los de valor no nulo o con efectos secundarios se asignan en el bucle. ✓

### 6.2. Impacto real de tamaño

| versión                              | bytes totales | Δ vs. estado previo |
|--------------------------------------|---------------|---------------------|
| Tras PRP004 (memset buggy en initFlags) | 19758      | — (baseline)        |
| Tras PRP005 (solución adoptada)      | 19739         | **−19 bytes**       |

El ahorro (−19 B) supera la estimación inicial (−4 a −9 B). Desglose plausible:

- El `memset` corregido no crece (mismo opcode con inmediato 88 en lugar de 16).
- Eliminar `cw->winX = cw->winY = 0;` ahorra más de lo que sugiere "dos stores a 0": SDCC probablemente tenía que recargar el puntero `cw` o la dirección desplazada varias veces (dependiendo de cómo estuviera materializado `windows[i]` en registros), añadiendo *setups* por asignación.
- Eliminar `cw->mode = 0;` añade un puñado de bytes más.

El resultado es consistente con el patrón "eliminar stores redundantes en el cuerpo de un bucle ahorra más que la instrucción individual".

### 6.3. Comentarios

Sin cambios en comentarios. El comentario de la línea 153 (`//Initialize DAAD windows`) sigue siendo preciso.

## 7. Alternativas consideradas

### 7.1. Fix mínimo: sólo corregir `sizeof(windows)` → `sizeof(Window)`

```c
windows = malloc(sizeof(Window)*WINDOWS_NUM);
memset(windows, 0, sizeof(Window)*WINDOWS_NUM);
// bucle igual que la versión actual, con todas las asignaciones
```

- **Correcto funcionalmente**: limpia 88 bytes (todos los `Window`).
- **Mismo tamaño de código** que la versión buggy (el producto es constante en ambos casos: pasa de `LD HL,#16` a `LD HL,#88`, misma huella).
- **Descartada frente a §6** porque mantiene en el bucle asignaciones a 0 que ya realiza el `memset` (código redundante, desperdicia bytes).

### 7.2. Sin `memset`, inicialización explícita de `cursorX`/`cursorY` en el bucle

```diff
-memset(windows, 0, sizeof(windows)*WINDOWS_NUM);
 for (i=0; i<WINDOWS_NUM; i++) {
     cw = &windows[i];
     cw->winX = cw->winY = 0;
     cw->winW = MAX_COLUMNS;
     cw->winH = MAX_LINES;
+    cw->cursorX = cw->cursorY = 0;
     cw->mode = 0;
     cw->lastPicLocation = 256;
     ...
 }
```

- **Correcto funcionalmente**: todos los campos se inicializan en el bucle (o por `gfxSetPaperCol/InkCol`).
- **Más pequeño que §6**: elimina el *callsite* completo de `memset` (~15-20 B), pagando sólo ~6 B por la nueva asignación de `cursorX/cursorY` en el cuerpo. Ahorro estimado: −10 a −14 B frente a la versión buggy.
- **Descartada frente a §6** por **mantenibilidad**: si en el futuro se añade un campo a `Window` cuyo valor por defecto sea 0, hay que acordarse de inicializarlo en el bucle. Con §6, el `memset` lo cubre automáticamente. La ganancia extra de tamaño (~5-8 B vs §6) no compensa la fragilidad.

### 7.3. Constante literal `memset(windows, 0, 88)`

- Equivalente en código generado a §6 (ambos pliegan a `LD HL,#88`).
- **Descartada**: `sizeof(Window)*WINDOWS_NUM` es auto-documentado y resistente a cambios de la struct; `88` es un número mágico.

### 7.4. Sustituir el bucle entero por `memset` al valor "plantilla"

- Requeriría una plantilla `Window` estática en ROM y un `memcpy` × N. Más complejo, sin beneficio de tamaño.
- Descartada.

## 8. Riesgos y compatibilidad

- **Compatibilidad de juegos**: positiva. Antes, cualquier ventana distinta de la 0 usada antes de un `CLS`/`WINAT` podía imprimir desde un cursor indeterminado. Tras el fix, toda ventana parte de `(0,0)` como debe ser.
- **Orden de inicialización**: el `flags[fCurWin] = 0` se desplaza levemente (ahora queda justo después del `malloc`, antes del bucle). Es idéntico al flujo anterior; `fCurWin` no se usa en el bucle.
- **Modos gráficos donde `COLOR_PAPER`/`COLOR_INK` no expanden a `cw->paper`/`cw->ink`**: los campos `paper`/`ink` quedarían en teoría sin inicializar en esos modos. **Revisar**: en [src/daad_platform_msx2.c:385-389](../src/daad_platform_msx2.c#L385-L389), el bloque `#else` define `COLOR_PAPER = 0x00` como constante, lo que implica que en ese modo `cw->paper`/`cw->ink` **nunca se usan** (y por tanto su valor es irrelevante). Antes del fix, el `memset` buggy ni siquiera los limpiaba para la mayoría de windows; el fix no empeora esta situación.
- **ABI, *targets***: sin cambios.

## 9. Plan de verificación

1. **Inspección estática** ✅: la línea del `memset` queda como `memset(windows, 0, sizeof(Window)*WINDOWS_NUM);` ([src/daad.c:155](../src/daad.c#L155)) y del bucle se eliminaron las asignaciones `cw->winX = cw->winY = 0;` y `cw->mode = 0;` ([src/daad.c:156-164](../src/daad.c#L156-L164)). El resto queda intacto.
2. **Compilación** ✅: rebuild completo sin warnings nuevos.
3. **Medida de tamaño** ✅: binario final 19758 → **19739 bytes** (−19 bytes). Ver tabla en §6.2.
4. **Tests unitarios** ✅: batería completa ejecutada en openmsx → **212 OK / 50 TODO / 0 fallos** (ver [unitTests/dsk/tests.txt](../unitTests/dsk/tests.txt)).

   El conteo de OK incluye, además de los tests preexistentes:
   - **3 tests de regresión** para los PRPs previos: `test_CHANCE_100_always_succeeds` (PRP001), `test_DROPALL_no_overflow` (PRP002), `test_RAMSAVE_no_overflow` (PRP003).
   - **5 tests** migrados de TODO a implementación real: `test_ISDONE_success`, `test_ISDONE_fails` (completados), y `test_DONE_success`, `test_NOTDONE_success`, `test_OK_success` (implementados desde cero con la infraestructura existente — `isDone`, `fake_lastSysMesPrinted`).

   `initFlags` está mockeado con `void initFlags() {}` en [unitTests/src/tests_condacts.c:97](../unitTests/src/tests_condacts.c#L97), así que el fix de este PRP005 no se verifica directamente en esta batería; sí se confirma la ausencia de regresión en el resto de tests.
5. **Prueba funcional con DDB real** (pendiente):
   - Juego que use más de una ventana (muchos lo hacen, p.ej. para barra de estado).
   - Ejecutar un `WINDOW n` (con `n ≠ 0`) **sin** `CLS`/`WINAT` previo y emitir texto.
   - Confirmar que el texto aparece en la esquina `(0,0)` de esa ventana. Antes del fix, podría aparecer en posición aleatoria o disparar scrolls.

## 10. Referencias

- Informe: [REPORT_DAAD_BUGS.md § BUG-06](../REPORT_DAAD_BUGS.md)
- Código afectado: [src/daad.c:144-179](../src/daad.c#L144-L179) (`initFlags`)
- Estructura `Window`: [include/daad.h:145-156](../include/daad.h#L145-L156)
- Definición `WINDOWS_NUM`: [include/daad.h:24-28](../include/daad.h#L24-L28)
- Uso de `cursorX`/`cursorY`: [src/daad.c:792-843](../src/daad.c#L792-L843), [src/daad_condacts.c:1677-1803](../src/daad_condacts.c#L1677-L1803)
- Macros `COLOR_PAPER`/`COLOR_INK`: [src/daad_platform_msx2.c:385-389](../src/daad_platform_msx2.c#L385-L389)
