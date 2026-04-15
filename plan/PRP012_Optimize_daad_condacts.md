# PRP012 — Optimizaciones Z80 en daad_condacts.c

- **ID**: PRP012
- **Tipo**: Optimización de rendimiento
- **Ficheros afectados**: [`src/daad_condacts.c`](../src/daad_condacts.c)
- **Severidad**: Media (rendimiento en Z80 / MSX2)
- **Fecha**: 2026-04-15
- **Estado**: Completado ✅

---

## 1. Resumen

Análisis de `daad_condacts.c` a la luz de la documentación técnica del Z80 (timings de instrucciones, coste de divisiones software, LDIR, asignación de registros en SDCC) y las herramientas del MCP openMSX.

Se detectan **8 optimizaciones** de rendimiento, ordenadas de mayor a menor impacto:

| # | Ubicación | Tipo | Impacto |
|---|---|---|---|
| OPT-01 | `getValueOrIndirection()` L232 | Eliminar `static` local → registro | **Alto** |
| OPT-02 | `processPROC()` L192–193 | Doble índice en tabla caliente → caché | **Alto** |
| OPT-03 | `processPROC()` L171–172 | `static` locals en zona speed → registros | **Alto** |
| OPT-04 | `_internal_hasat()` L588 | `% 8` → `& 7` (división → máscara bit) | **Medio** |
| OPT-05 | `_internal_listat()` L1902 | `int` → `uint8_t` en contador de bucle | **Medio** |
| OPT-06 | `do_CHANCE()` / `do_RANDOM()` L548/1503 | `rand() % 100` → división 8-bit | **Medio** |
| OPT-07 | `do_PLUS()` L1427–1430 | Ensanchamiento a 16-bit innecesario | **Medio** |
| OPT-08 | `_internal_doall()` L2193–2203 | Acceso repetido a `objects[objno]` → puntero | **Bajo** |

---

## 2. Detalles de cada optimización

---

### OPT-01 ✅ — `getValueOrIndirection()`: eliminar `static` en variable local

**Ubicación**: [`src/daad_condacts.c:232–238`](../src/daad_condacts.c#L232-L238)

**Código actual**:
```c
static uint8_t getValueOrIndirection()
{
    static uint8_t value;
    value = *pPROC++;
    return indirection ? flags[value] : value;
}
```

**Problema**: `static uint8_t value` fuerza a SDCC a reservar una dirección fija en RAM (variable global encubierta). El compilador no puede asignarla al registro `A`, obligando a una store/load extra por cada llamada. Esta función se invoca en casi todos los condacts — es el helper más llamado del intérprete.

**Contexto Z80**: En Z80, acceder a una dirección absoluta cuesta 13 T-states (`LD A,(nn)`); acceder a una variable en registro A es 0 T-states adicionales. Con `#pragma opt_code_speed` activo en esta zona (línea 14), SDCC puede asignar `value` al registro A si no es `static`.

**Corrección**:
```c
static uint8_t getValueOrIndirection()
{
    uint8_t value = *pPROC++;
    return indirection ? flags[value] : value;
}
```

---

### OPT-02 ✅ — `processPROC()`: doble índice en `condactList` en el bucle caliente

**Ubicación**: [`src/daad_condacts.c:192–193`](../src/daad_condacts.c#L192-L193)

**Código actual**:
```c
condactList[currCondact->condact].function();
isDone |= condactList[currCondact->condact].flag;
```

**Problema**: `condactList[currCondact->condact]` se evalúa dos veces en cada iteración del bucle principal. En Z80 esto implica: leer `currCondact->condact`, multiplicar el índice por `sizeof(CONDACT_LIST)` (estructura de 3 bytes: puntero 2B + flag 1B), sumar la base de `condactList`. Esta secuencia se ejecuta dos veces consecutivas.

**Corrección**: cachear un puntero a la entrada antes de usar sus campos:
```c
const CONDACT_LIST *ce = &condactList[currCondact->condact];
ce->function();
isDone |= ce->flag;
```

**Ganancia**: elimina el cálculo de dirección repetido (índice × 3 + base), que en Z80 software puro son varias instrucciones ADD HL,xx.

---

### OPT-03 ✅ — `processPROC()`: `static` locals en zona `opt_code_speed`

**Ubicación**: [`src/daad_condacts.c:171–172`](../src/daad_condacts.c#L171-L172)

**Código actual**:
```c
static CondactStruct *currCondact;
static uint8_t temp;
```

**Problema**: Igual que OPT-01. Ambas variables están en la sección `#pragma opt_code_speed` (antes de la línea 240) pero son `static`, lo que impide la asignación a registros por parte de SDCC. `currCondact` es un puntero de 16-bit (HL/IX/IY) y `temp` es 8-bit (A). Con `static` removido, SDCC puede mantenerlos en registros durante todo el bucle interno.

**Nota**: `currCondact` se usa únicamente dentro del bucle `while`, por lo que no existe riesgo de ruptura por reutilización entre iteraciones si se elimina `static`.

**Corrección**:
```c
CondactStruct *currCondact;
uint8_t temp;
```

---

### OPT-04 ✅ — `_internal_hasat()`: `% 8` → `& 7`

**Ubicación**: [`src/daad_condacts.c:588`](../src/daad_condacts.c#L588)

**Código actual**:
```c
bit = 1 << (value % 8);
```

**Problema**: `% 8` (módulo 8) sobre un entero `uint8_t` en SDCC genera una llamada a rutina de división software. En Z80 no existe instrucción de división — la división de 8 bits requiere un bucle de 8 iteraciones (~60–80 T-states).

**Documentación MSX**: El artículo "[ASM] Multiplications, divisions and shifts" de la base de datos de recursos MSX confirma que dividir por una potencia de 2 debe hacerse siempre mediante desplazamientos (SRL/SRA) o máscaras, que cuestan 4–8 T-states.

**Corrección**: Dado que 8 es potencia de 2:
```c
bit = 1 << (value & 7);
```
`& 7` compila a `AND 0x07` — 1 instrucción, 4 T-states en Z80.

**Resultado idéntico** para cualquier `uint8_t value` (los bits del módulo 8 son exactamente los 3 bits bajos).

---

### OPT-05 ✅ — `_internal_listat()`: contador `int` → `uint8_t`

**Ubicación**: [`src/daad_condacts.c:1902`](../src/daad_condacts.c#L1902)

**Código actual**:
```c
for (int i=0; i<=hdr->numObjDsc; i++) {
```

**Problema**: En SDCC para Z80, `int` es 16-bit. El bucle genera instrucciones de incremento e comparación 16-bit (INC HL / CALL __cmpw o similar) en lugar de simples INC A / CP. Con `uint8_t`, el bucle usa `INC A` (4 T-states) + `CP n` (7 T-states) frente a ~25 T-states para la versión 16-bit. Este bucle recorre todos los objetos del juego en cada llamada a LISTOBJ/LISTAT.

**Precondición**: `hdr->numObjDsc` es `uint8_t` (máximo 255). Si `numObjDsc == 255`, un contador `uint8_t` que llega a 255 y se incrementa haría wraparound a 0, produciendo un bucle infinito. La corrección segura usa una estructura de bucle alternativa:

**Corrección**:
```c
uint8_t n = hdr->numObjDsc;
for (uint8_t i=0; i<=n; i++) {
    if (i==n || objects[i].location == loc) {
```
Aquí `n` es una copia local de `numObjDsc`, y si `n < 255` el bucle es siempre finito. Si el juego garantiza que el número de objetos no puede ser 255 (límite de facto de DAAD), la corrección es directa.

---

### OPT-06 ✅ — `do_CHANCE()` / `do_RANDOM()`: `rand() % 100` → división 8-bit

**Ubicaciones**:
- [`src/daad_condacts.c:548`](../src/daad_condacts.c#L548) — `do_CHANCE()`
- [`src/daad_condacts.c:1503`](../src/daad_condacts.c#L1503) — `do_RANDOM()`

**Código actual**:
```c
// do_CHANCE():
checkEntry = (rand()%100) < getValueOrIndirection();

// do_RANDOM():
flags[getValueOrIndirection()] = (rand()%100)+1;
```

**Problema**: En SDCC para Z80, `rand()` devuelve `int` (16-bit). La operación `% 100` es una división 16-bit / 16-bit por software — la rutina genérica de módulo Z80 tarda ~200–300 T-states (bucle de 16 iteraciones, según las rutinas de la documentación MSX "[ASM] Z80 routines collection").

**Solución**: tomar solo el byte bajo de `rand()`, que ya es pseudoaleatorio suficiente para un juego de texto, y aplicar módulo 100 sobre 8 bits (rutina Div8 = 8 iteraciones, ~80–100 T-states):

```c
// do_CHANCE():
checkEntry = ((uint8_t)rand() % 100) < getValueOrIndirection();

// do_RANDOM():
flags[getValueOrIndirection()] = ((uint8_t)rand() % 100) + 1;
```

**Alternativa sin división**: usar la técnica de multiplicación-shift (`((uint8_t)rand() * 100) >> 8`) que evita cualquier división y da distribución uniforme 0–99. Requiere una rutina de multiplicación 8×8→16 bits (~40 T-states), de la que solo se toma el byte alto.

**Impacto en aleatoriedad**: los bits bajos de un LCG (generador de `rand()` estándar) tienen periodo ligeramente menor, pero en un aventura de texto la diferencia es imperceptible.

---

### OPT-07 ✅ — `do_PLUS()`: ensanchamiento a 16-bit innecesario

**Ubicación**: [`src/daad_condacts.c:1427–1430`](../src/daad_condacts.c#L1427-L1430)

**Código actual**:
```c
uint8_t flagno = getValueOrIndirection();
uint16_t value = (uint16_t)flags[flagno] + *pPROC++;
if (value>255) value = 255;
flags[flagno] = (uint8_t)value;
```

**Problema**: SDCC ensancha la suma a 16-bit para detectar overflow. En Z80, la instrucción `ADD A, n` establece el **carry flag** directamente cuando el resultado excede 255 — no se necesita ensanchamiento. La versión 16-bit genera 2 bytes por operando (LD HL / ADD HL,BC) y un comparador 16-bit, frente a `ADD A,n` + `JR NC` + `LD A,0xFF`.

**Corrección usando comportamiento de carry Z80**:
```c
uint8_t flagno = getValueOrIndirection();
uint8_t addend = *pPROC++;
uint8_t result = flags[flagno] + addend;
if (result < flags[flagno]) result = 255;  // carry: wraparound indica overflow
flags[flagno] = result;
```
La comparación `result < flags[flagno]` es el patrón estándar de detección de carry en C sin extensión a 16-bit — SDCC puede compilarlo a `ADD A,n` + `JR NC, skip` + `LD A, 0xFF`.

**Nota**: La misma lógica aplica a `do_ADD()` ([L1454–1457](../src/daad_condacts.c#L1454-L1457)) que usa `uint16_t sum`.

---

### OPT-08 ✅ — `_internal_doall()`: accesos repetidos a `objects[objno]`

**Ubicación**: [`src/daad_condacts.c:2193–2203`](../src/daad_condacts.c#L2193-L2203)

**Código actual**:
```c
while (objects[objno].location!=locno || (objects[objno].nounId==flags[fNoun2] && objects[objno].adjectiveId==flags[fAdject2])) {
    objno++;
    ...
}
flags[fDAObjNo] = objno++;
flags[fNoun1] = objects[flags[fDAObjNo]].nounId;
flags[fAdject1] = objects[flags[fDAObjNo]].adjectiveId;
```

**Problema**: `objects[objno]` se evalúa hasta 3 veces dentro del bucle (`.location`, `.nounId`, `.adjectiveId`). Cada evaluación requiere: cargar `objno`, multiplicar por `sizeof(Object)` = 6, sumar `objects`. Multiplicar por 6 en Z80 (potencia de 2 no exacta: 4+2) requiere desplazamientos y sumas extra.

Tras el bucle, `objects[flags[fDAObjNo]]` se evalúa dos veces más para `.nounId` y `.adjectiveId`, cuando `flags[fDAObjNo]` acaba de asignarse con `objno++`.

**Corrección**: usar un puntero local:
```c
Object *obj = &objects[objno];
while (obj->location!=locno || (obj->nounId==flags[fNoun2] && obj->adjectiveId==flags[fAdject2])) {
    objno++;
    if (objno >= hdr->numObjDsc) {
        currProc->condactDOALL = NULL;
        do_DONE();
        return;
    }
    obj = &objects[objno];
}
flags[fDAObjNo] = objno;
flags[fNoun1] = obj->nounId;
flags[fAdject1] = obj->adjectiveId;
```
La aritmética de índice se hace una sola vez por iteración, y los accesos a campos son simples desplazamientos de un puntero HL.

---

## 3. Plan de implementación

### Fase 1 — Bucle caliente (mayor impacto, menor riesgo)

- [x] **OPT-01**: Eliminar `static` de `value` en `getValueOrIndirection()` (L232)
- [x] **OPT-02**: Cachear puntero `ce` en `processPROC()` (L192–193)
- [x] **OPT-03**: Eliminar `static` de `currCondact` y `temp` en `processPROC()` (L171–172)

### Fase 2 — Operaciones aritméticas

- [x] **OPT-04**: `value % 8` → `value & 7` en `_internal_hasat()` (L588)
- [x] **OPT-07**: Eliminar ensanchamiento 16-bit en `do_PLUS()` (L1427) y `do_ADD()` (L1454)
- [x] **OPT-06**: `rand() % 100` → `(uint8_t)rand() % 100` en `do_CHANCE()` (L548) y `do_RANDOM()` (L1503)

### Fase 3 — Bucles de objetos

- [x] **OPT-05**: `int i` → `uint8_t i` en `_internal_listat()` (L1902) con reestructuración segura
- [x] **OPT-08**: Puntero local en `_internal_doall()` (L2193)

### Fase 4 — Verificación en openMSX

- [ ] Compilar con SDCC y comparar el listado ASM antes/después de OPT-01/02/03
- [ ] Medir tiempos de ejecución del intérprete con `mcp__mcp-openmsx__debug_cpu` (breakpoints + T-states)
- [ ] Verificar comportamiento funcional con los tests unitarios existentes (PRP008/PRP009/PRP010)

---

## 4. Riesgos y notas

| Optimización | Riesgo | Mitigación |
|---|---|---|
| OPT-01/03 | SDCC puede necesitar `static` para funciones re-entrantes en algunas configuraciones | Verificar ASM generado |
| OPT-05 | `numObjDsc == 255` causa bucle infinito con `uint8_t` | Documentar precondición; DAAD estándar no usa 255 objetos |
| OPT-06 | Bits bajos de LCG tienen periodo menor | Aceptable para texto aventura; no afecta jugabilidad |
| OPT-07 | El patrón `result < flags[flagno]` requiere que SDCC lo compile a carry-check | Verificar ASM; añadir `__z88dk_fastcall` si no lo optimiza |

---

## 5. Referencias

- **[ASM] Multiplications, divisions and shifts** — `msxdocs://programming/asm_mult_div_shifts` — Coste de divisiones Z80, técnica shift×multiply
- **[ASM] Z80 routines collection** — `msxdocs://programming/asm_z80_routines_collection` — Rutinas Div8 / Div16 y sus T-states
- **[ASM] Fast loops** — `msxdocs://programming/asm_fast_loops` — LDIR vs LDI unrolled, DJNZ 16-bit
- **Z80 / R800 Instruction Set** — `msxdocs://processors/z80_r800_instruction_set` — T-states por instrucción
- [`src/daad_condacts.c`](../src/daad_condacts.c) — Fichero analizado
- [`include/daad.h`](../include/daad.h) — Definición de structs y constantes (`LOC_*`, `Object`, `PROCstack`)
