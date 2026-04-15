# PRP010 — Unit tests para condacts de UI (mocks de salida de texto y color)

- **ID**: PRP010
- **Tipo**: Testing — eliminación de TODOs "[UI not mocked]"
- **Ficheros afectados**: `unitTests/src/condacts_stubs.h`, `unitTests/src/condacts_stubs.c`, `unitTests/src/tests_condacts5.c`
- **Severidad**: Mejora de calidad — sin cambios en producción
- **Fecha**: 2026-04-15
- **Estado**: Completado ✅
- **Commit**: staged

---

## 1. Resumen

15 tests en `tests_condacts5.c` estaban marcados como `TODO("UI not mocked")` porque los stubs de las funciones de impresión y color eran no-ops sin captura de valor. El cambio necesario era mínimo: añadir **7 variables `fake_*`** en los stubs y actualizar las firmas correspondientes. Con eso, 15 tests pasan de TODO a OK. Adicionalmente se añadieron **14 tests de edge cases** (indirección, fronteras, semántica de LISTOBJ/LISTAT).

```
             Antes       Después (tests.txt)
OK:    342 / 373    →   371 / 387   (+29)
FAIL:    0 / 373    →     0 / 387
TODO:   31 / 373    →    16 / 387   (−15)
TOTAL:      373     →       387     (+14 tests nuevos)
```

`test_SYSMES_success` no requirió cambios en stubs porque `printSystemMsg` ya capturaba en `fake_lastSysMesPrinted`.

---

## 2. Análisis de los TODOs

### 2.1 Clasificación por stub afectado

| Condact(s) | Stub actual | Problema | Solución |
|---|---|---|---|
| `SYSMESS` | `printSystemMsg` → `fake_lastSysMesPrinted` ✓ | Ninguno — ya mocked | Solo escribir el test |
| `PAPER` | `gfxSetPaperCol(col) {}` | Descarta `col` | Añadir `fake_lastPaperCol` |
| `INK` | `gfxSetInkCol(col) {}` | Descarta `col` | Añadir `fake_lastInkCol` |
| `BORDER` | `gfxSetBorderCol(col) {}` | Descarta `col` | Añadir `fake_lastBorderCol` |
| `SPACE`, `NEWLINE` | `printChar(c) {}` | Descarta `c` | Añadir `fake_lastCharPrinted` |
| `MES`, `MESSAGE` | `printUserMsg(num) {}` | Descarta `num` | Añadir `fake_lastUserMsgPrinted` |
| `DESC` | `printLocationMsg(num) {}` | Descarta `num` | Añadir `fake_lastLocMsgPrinted` |
| `PRINT`, `DPRINT` | `printBase10(value) {}` | Descarta `value` | Añadir `fake_lastBase10Printed` |
| `LISTOBJ`, `LISTAT` | `flags[fOFlags] & F53_LISTED` ✓ | Ya observable | Solo escribir los tests |

### 2.2 Cadena de llamadas

```
PAPER   → gfxSetPaperCol(col)
INK     → gfxSetInkCol(col)
BORDER  → gfxSetBorderCol(col)
SPACE   → printChar(' ')
NEWLINE → printChar('\r')
MES     → printUserMsg(mesno)
MESSAGE → printUserMsg(mesno) + printChar('\r')
SYSMESS → printSystemMsg(num)        ← ya capturado en fake_lastSysMesPrinted
DESC    → printLocationMsg(locno)
PRINT   → printBase10(flags[flagno])
DPRINT  → printBase10(flags[f] | flags[f+1]<<8)
LISTOBJ → _internal_listat(fPlayer, true)  → flags[fOFlags] & F53_LISTED
LISTAT  → _internal_listat(locno, false)   → flags[fOFlags] & F53_LISTED
```

---

## 3. Cambios en stubs

### 3.1 `unitTests/src/condacts_stubs.h` — añadir 7 declaraciones `extern`

Añadir tras `extern int16_t fake_lastSysMesPrinted;`:

```c
extern int      fake_lastCharPrinted;      // printChar: last char printed (' '=SPACE, '\r'=NEWLINE)
extern int16_t  fake_lastUserMsgPrinted;   // printUserMsg: last user message number (-1=none)
extern int16_t  fake_lastLocMsgPrinted;    // printLocationMsg: last location number (-1=none)
extern uint16_t fake_lastBase10Printed;    // printBase10: last decimal value printed
extern uint8_t  fake_lastPaperCol;         // gfxSetPaperCol: last paper colour set
extern uint8_t  fake_lastInkCol;           // gfxSetInkCol: last ink colour set
extern uint8_t  fake_lastBorderCol;        // gfxSetBorderCol: last border colour set
```

### 3.2 `unitTests/src/condacts_stubs.c` — definir y capturar

#### Añadir definiciones de variables (junto a las existentes):

```c
int      fake_lastCharPrinted     = -1;
int16_t  fake_lastUserMsgPrinted  = -1;
int16_t  fake_lastLocMsgPrinted   = -1;
uint16_t fake_lastBase10Printed   = 0;
uint8_t  fake_lastPaperCol        = 0;
uint8_t  fake_lastInkCol          = 0;
uint8_t  fake_lastBorderCol       = 0;
```

#### Actualizar stubs no-op para capturar valores:

```c
// Antes:
void printChar(int c) __z88dk_fastcall {}
void printUserMsg(uint8_t num) __z88dk_fastcall {}
void printLocationMsg(uint8_t num) __z88dk_fastcall {}
void printBase10(uint16_t value) __z88dk_fastcall {}
inline void gfxSetPaperCol(uint8_t col) {}
inline void gfxSetInkCol(uint8_t col) {}
inline void gfxSetBorderCol(uint8_t col) {}

// Después:
void printChar(int c) __z88dk_fastcall       { fake_lastCharPrinted = c; }
void printUserMsg(uint8_t num) __z88dk_fastcall  { fake_lastUserMsgPrinted = num; }
void printLocationMsg(uint8_t num) __z88dk_fastcall { fake_lastLocMsgPrinted = num; }
void printBase10(uint16_t value) __z88dk_fastcall   { fake_lastBase10Printed = value; }
inline void gfxSetPaperCol(uint8_t col)  { fake_lastPaperCol = col; }
inline void gfxSetInkCol(uint8_t col)    { fake_lastInkCol = col; }
inline void gfxSetBorderCol(uint8_t col) { fake_lastBorderCol = col; }
```

#### Reset en `beforeEach()` (junto a los resets existentes):

```c
fake_lastCharPrinted    = -1;
fake_lastUserMsgPrinted = -1;
fake_lastLocMsgPrinted  = -1;
fake_lastBase10Printed  = 0;
fake_lastPaperCol       = 0;
fake_lastInkCol         = 0;
fake_lastBorderCol      = 0;
```

---

## 4. Tests a implementar en `tests_condacts5.c`

### 4.1 SYSMESS — sin cambios en stubs (Manual 1991 L.1853)

```c
void test_SYSMES_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given flag 5 as the system message to print
    static const char proc[] = { _SYSMESS, 5, 255 };
    do_action(proc);

    //BDD then printSystemMsg(5) was called
    ASSERT_EQUAL(fake_lastSysMesPrinted, 5, "SYSMESS must call printSystemMsg with the given number");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.2 PAPER, INK, BORDER (Manual 1991 L.1755, L.1765, L.1779)

```c
void test_PAPER_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given colour 3
    static const char proc[] = { _PAPER, 3, 255 };
    do_action(proc);

    //BDD then gfxSetPaperCol(3) was called
    ASSERT_EQUAL(fake_lastPaperCol, 3, "PAPER must call gfxSetPaperCol with given colour");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_INK_success()
{
    const char *_func = __func__;
    beforeEach();

    static const char proc[] = { _INK, 7, 255 };
    do_action(proc);

    ASSERT_EQUAL(fake_lastInkCol, 7, "INK must call gfxSetInkCol with given colour");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_BORDER_success()
{
    const char *_func = __func__;
    beforeEach();

    static const char proc[] = { _BORDER, 2, 255 };
    do_action(proc);

    ASSERT_EQUAL(fake_lastBorderCol, 2, "BORDER must call gfxSetBorderCol with given colour");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.3 SPACE, NEWLINE (Manual 1991 L.1812, L.1822)

```c
void test_SPACE_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD when SPACE is called
    static const char proc[] = { _SPACE, 255 };
    do_action(proc);

    //BDD then printChar(' ') was called
    ASSERT_EQUAL(fake_lastCharPrinted, ' ', "SPACE must call printChar(' ')");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_NEWLINE_success()
{
    const char *_func = __func__;
    beforeEach();

    static const char proc[] = { _NEWLINE, 255 };
    do_action(proc);

    //BDD then printChar('\r') was called
    ASSERT_EQUAL(fake_lastCharPrinted, '\r', "NEWLINE must call printChar('\\r')");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.4 MES, MESSAGE (Manual 1991 L.1832, L.1842)

```c
void test_MES_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given user message 10
    static const char proc[] = { _MES, 10, 255 };
    do_action(proc);

    //BDD then printUserMsg(10) was called
    ASSERT_EQUAL(fake_lastUserMsgPrinted, 10, "MES must call printUserMsg with given message number");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_MESSAGE_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given user message 10
    //     MESSAGE = MES + NEWLINE: calls printUserMsg(10) then printChar('\r')
    static const char proc[] = { _MESSAGE, 10, 255 };
    do_action(proc);

    //BDD then both printUserMsg and printChar('\r') were called
    ASSERT_EQUAL(fake_lastUserMsgPrinted, 10, "MESSAGE must call printUserMsg with given message number");
    ASSERT_EQUAL(fake_lastCharPrinted, '\r', "MESSAGE must call NEWLINE after MES");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.5 DESC (Manual 1991 L.1863)

```c
void test_DESC_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given location 5
    static const char proc[] = { _DESC, 5, 255 };
    do_action(proc);

    //BDD then printLocationMsg(5) was called
    ASSERT_EQUAL(fake_lastLocMsgPrinted, 5, "DESC must call printLocationMsg with given location number");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.6 PRINT, DPRINT (Manual 1991 L.1874, L.1884)

```c
void test_PRINT_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given flag 100 = 42
    flags[100] = 42;

    //BDD when PRINT 100
    static const char proc[] = { _PRINT, 100, 255 };
    do_action(proc);

    //BDD then printBase10(42) was called
    ASSERT_EQUAL(fake_lastBase10Printed, 42, "PRINT must call printBase10 with the value of the given flag");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_DPRINT_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given flag 100 = 0x34, flag 101 = 0x12 → 16-bit value 0x1234 = 4660
    flags[100] = 0x34;
    flags[101] = 0x12;

    //BDD when DPRINT 100
    static const char proc[] = { _DPRINT, 100, 255 };
    do_action(proc);

    //BDD then printBase10(4660) was called
    ASSERT_EQUAL(fake_lastBase10Printed, 0x1234, "DPRINT must call printBase10 with 16-bit value (flag + flag+1)");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.7 LISTOBJ (Manual 1991 L.1895)

```c
void test_LISTOBJ_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given player at loc 5, obj1 and obj2 at loc 5
    flags[fPlayer] = 5;
    objects[1].location = 5;
    objects[1].nounId = 10;
    objects[2].location = 5;
    objects[2].nounId = 11;

    //BDD when LISTOBJ
    static const char proc[] = { _LISTOBJ, 255 };
    do_action(proc);

    //BDD then F53_LISTED is set (objects were found and listed)
    ASSERT(flags[fOFlags] & F53_LISTED, "LISTOBJ must set F53_LISTED when objects are present at player location");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_LISTOBJ_none()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given player at loc 5, no objects at loc 5
    flags[fPlayer] = 5;
    // all objects stay at location 0 (from beforeEach memset), player is at 5

    //BDD when LISTOBJ
    static const char proc[] = { _LISTOBJ, 255 };
    do_action(proc);

    //BDD then F53_LISTED is NOT set (no objects at player location)
    ASSERT(!(flags[fOFlags] & F53_LISTED), "LISTOBJ must not set F53_LISTED when no objects at player location");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.8 LISTAT (Manual 1991 L.1933)

```c
void test_LISTAT_success()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given obj1 at loc 3
    objects[1].location = 3;
    objects[1].nounId = 10;

    //BDD when LISTAT 3
    static const char proc[] = { _LISTAT, 3, 255 };
    do_action(proc);

    //BDD then F53_LISTED is set (objects were found and listed)
    //     Note: do_LISTAT prints SM51 (".") on success, but the stub filters SM51
    //     (if (num!=51)) so fake_lastSysMesPrinted does NOT capture it.
    //     F53_LISTED is the only observable output here.
    ASSERT(flags[fOFlags] & F53_LISTED, "LISTAT must set F53_LISTED when objects present at given location");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_LISTAT_none()
{
    const char *_func = __func__;
    beforeEach();

    //BDD given no objects at loc 7 (all objects at location 0 from memset)
    //BDD when LISTAT 7
    static const char proc[] = { _LISTAT, 7, 255 };
    do_action(proc);

    //BDD then F53_LISTED NOT set and SM53 ("Nada.") was printed
    ASSERT(!(flags[fOFlags] & F53_LISTED), "LISTAT must not set F53_LISTED when no objects at given location");
    ASSERT_EQUAL(fake_lastSysMesPrinted, 53, "LISTAT must print SM53 when no objects found");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

### 4.9 Actualización del `main` en `tests_condacts5.c`

Reemplazar las llamadas `TODO` por las nuevas funciones:

```c
    test_PAPER_success();
    test_INK_success();
    test_BORDER_success();
    // ... (resto de tests existentes) ...
    test_PRINTAT_success(); test_PRINTAT_indirection();
    test_TAB_success(); test_TAB_indirection();
    test_SPACE_success();
    test_NEWLINE_success();
    test_MES_success();
    test_MESSAGE_success();
    test_SYSMES_success();
    test_DESC_success();
    test_PRINT_success();
    test_DPRINT_success();
    test_LISTOBJ_success(); test_LISTOBJ_none();
    test_LISTAT_success(); test_LISTAT_none();
```

---

## 5. Notas de implementación

- **`printChar` es muy usada internamente**: otros condacts (PUTIN, TAKEOUT, etc.) llaman a funciones que invocan `printChar` indirectamente. `beforeEach()` resetea `fake_lastCharPrinted = -1`, así que tests de SPACE/NEWLINE deben ejecutarse como operaciones aisladas (no encadenadas con condacts que impriman).

- **`fake_lastSysMesPrinted` es last-write-wins y filtra SM51**: el stub guarda solo el último SM impreso, y excluye SM51 (`"."`) explícitamente. La razón del filtro: SM51 actúa como terminador de frase en muchos condacts (PUTIN, TAKEOUT, ISAT…) y sobreescribiría el SM "interesante" que el test quiere verificar. Consecuencia directa para este PRP: `test_LISTAT_success` **no puede** asertarse contra SM51 — solo puede verificar `F53_LISTED`. `test_LISTAT_none` sí puede asertarse contra SM53 porque 53 ≠ 51 y no está filtrado. **No se debe eliminar el filtro** — rompería decenas de tests existentes de condacts3.

- **`LISTOBJ_none`**: `beforeEach()` hace memset de objects a 0, por lo que `objects[i].location == 0` para todos los objetos. El jugador se coloca en `loc 5`. Como ningún objeto está en loc 5, F53_LISTED no se activa. Esta asunción debe verificarse contra el `beforeEach` real.

- **`LISTAT` SM51 vs SM53**: el stub filtra SM51 actualmente — ver punto anterior. Si se mantiene el filtro, `test_LISTAT_success` no podrá verificar SM51 y debe asertarse solo con `F53_LISTED`.

---

## 6. Resultado final

| Fichero | Antes | Después |
|---------|-------|---------|
| `condact5.com` | 7 OK + 15 TODO | **37 OK + 0 TODO** |
| **Suite total** | 342 OK / 373 | **371 OK / 387** |

```
             Antes        Después (tests.txt)
OK:    342 / 373  →  371 / 387   (+29: 15 TODO→OK + 14 nuevos)
FAIL:    0 / 373  →    0 / 387
TODO:   31 / 373  →   16 / 387   (−15)
TOTAL:      373   →      387     (+14 tests de edge cases)
```

### Tests de edge cases añadidos (+14)

| Test | Qué cubre |
|---|---|
| `test_PAPER/INK/BORDER_indirection` | Operador `@` en los 3 condacts de color |
| `test_MES_indirection` | `@` en MES |
| `test_SYSMES_indirection` | `@` en SYSMESS |
| `test_SYSMES_sm51_not_captured` | Documenta el filtro SM51 del stub: `SYSMESS 51` no sobreescribe `fake_lastSysMesPrinted` |
| `test_DESC_indirection` | `@` en DESC |
| `test_PRINT_indirection` | `@` doble: resuelve índice de flag via indirección, luego imprime el valor de ese flag |
| `test_PRINT_zero` | Valor 0 en PRINT (frontera) |
| `test_DPRINT_max_value` | Valor máximo 0xFFFF en DPRINT |
| `test_LISTOBJ_carried_not_shown` | Objetos portados NO aparecen en LISTOBJ (LOC_CARRIED ≠ loc del jugador) |
| `test_LISTOBJ_worn_not_shown` | Objetos puestos NO aparecen en LISTOBJ |
| `test_LISTOBJ_sm48_terminator` | SM48 (`".\n"`) se imprime como terminador de lista |
| `test_LISTAT_loc_carried` | `LISTAT 254` lista el inventario — patrón real de aventura |

Los 16 TODOs restantes (condacts5 y condacts6) corresponden a condacts que requieren mocks más complejos (entrada de usuario, plataforma gráfica real, etc.) y quedan fuera del alcance de este PRP.
