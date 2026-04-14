# PRP008 — Infraestructura de Unit Tests para el motor DAAD

- **ID**: PRP008
- **Tipo**: Testing infrastructure
- **Ficheros afectados**: `unitTests/`, `src/daad/daad_objects.c`
- **Severidad**: Mejora de calidad — sin cambios en producción salvo un bug fix en `getObjectId`
- **Fecha**: 2026-04-14 / 2026-04-15
- **Estado**: Completado ✅

---

## 1. Resumen

Esta PRP documenta la construcción completa de la infraestructura de unit tests para el proyecto MSX2DAAD, partiendo de un único fichero monolítico (`tests_condacts.c`, 5731 líneas) y llegando a **11 binarios .com independientes** con **323 tests** (292 OK, 0 FAIL, 31 TODO).

El trabajo se divide en cuatro áreas:

1. **Splitting y reorganización** del monolítico en 6 binarios por categoría + stubs compartidos
2. **Nueva suite dos.lib** con 23 tests de file I/O para MSX-DOS1
3. **Suite daad engine** con 4 binarios que prueban las funciones C del motor directamente
4. **Bug fix** en `getObjectId` (path LOC_CONTAINER nunca funcionaba)

---

## 2. Estado inicial vs. final

| Métrica | Inicial | Final |
|---------|---------|-------|
| Binarios .com de test | 1 (`condacts.com`) | **11** |
| Tests totales | 262 | **323** |
| Tests OK | 212 | **292** |
| Tests TODO | 50 | **31** |
| Tests FAIL | 0 | **0** |
| Ficheros fuente de test | 1 | **12** |
| Stubs compartidos | 0 | **2 pares (h/c)** |
| Tests dos.lib | 0 | **23** |
| Tests daad engine | 0 | **22** |

---

## 3. Arquitectura de la suite

### 3.1. Binarios generados

| Binario | Fuente | Contenido | Tests |
|---------|--------|-----------|-------|
| `dos.com` | `tests_dos.c` | dos.lib: dosver, fileexists, fcreate/fclose, fopen, fwrite/fread, fseek, fputs/fgets, filesize, remove, kbhit | 23 |
| `condact1.com` | `tests_condacts1.c` | AT, NOTAT, ATGT, ATLT, PRESENT, ABSENT, WORN, NOTWORN, CARRIED, NOTCARR, ISAT, ISNOTAT | 36 |
| `condact2.com` | `tests_condacts2.c` | ZERO…SMALLER, ADJECT1-2, ADVERB, PREP, NOUN2, CHANCE, ISDONE/ISNDONE, HASAT/HASNAT, INKEY, QUIT | 61 |
| `condact3.com` | `tests_condacts3.c` | GET, DROP, WEAR, REMOVE, CREATE, DESTROY, SWAP, PLACE, PUTO, PUTIN, TAKEOUT, DROPALL, AUTO* | 62 |
| `condact4.com` | `tests_condacts4.c` | COPYOO/OF/FO/FF/BF, WHATO, SETCO, WEIGH, SET-SUB, RANDOM, MOVE, GOTO, WEIGHT, ABILITY, RESET | 44 |
| `condact5.com` | `tests_condacts5.c` | MODE, INPUT, TIME, WINDOW, WINAT, WINSIZE, CENTRE, CLS, SAVEAT, BACKAT, PRINTAT, TAB | 22 |
| `condact6.com` | `tests_condacts6.c` | SAVE/LOAD, RAMSAVE/RAMLOAD, ANYKEY, PARSE, NEWTEXT, SYNONYM, PROCESS, REDO, SKIP, RESTART, DONE, NOTDONE, OK | 17 |
| `objwght.com` | `tests_daad_getObjectWeight.c` | `getObjectWeight()`: simple, NULLWORD, container, magic bag, cap 255 | 5 |
| `daadobjs.com` | `tests_daad_objects.c` | `getObjectId()` (6 tests), `referencedObject()` (2 tests) | 8 |
| `gettoken.com` | `tests_daad_getToken.c` | `getToken()`: tokens 0, 1, 2 | 3 |
| `initobj.com` | `tests_daad_init.c` | `initObjects()` (4 tests), `initFlags()` (2 tests) | 6 |

### 3.2. Stubs compartidos

**Para tests de condacts** (`condact1-6.com`):

- `condacts_stubs.h` / `condacts_stubs.c` — Globals (`flags[]`, `objects[]`, `hdr`, `ddb`, `windows[]`, `ramsave`, `tmpMsg`, `tmpTok`, `nullObject`, `doingPrompt`...), stubs de plataforma (gfx, sfx, keyboard, timer), stubs vacíos de funciones daad que no se prueban, `beforeAll()`, `beforeEach()`, `do_action()`.

  **Funciones reales enlazadas** (`.rel` añadidos a `CONDACTS_BASE_LIBS`):
  - `daad_condacts.rel` — motor de condacts completo
  - `daad_objects.rel` — `getObjectId`, `referencedObject` (antes eran stubs vacíos)
  - `daad_getObjectWeight.rel` — peso real con lógica de contenedores

**Para tests del motor daad** (`objwght/daadobjs/gettoken/initobj.com`):

- `daad_stubs.h` / `daad_stubs.c` — Mismo set de globals + stubs de plataforma + stubs PROC stack mínimos. No define las funciones que se prueban (vienen de sus `.rel`). Sin `daad_condacts.rel` (evita referencias cruzadas no resueltas).

### 3.3. Convención de test

Cada test sigue el patrón BDD:
```c
void test_XXX_scenario()
{
    const char *_func = __func__;
    beforeEach();           // reset global state

    // BDD given ...
    // BDD when ...
    // BDD then ...
    ASSERT_EQUAL(...);
    SUCCEED();
}
```

Las macros `ASSERT`, `ASSERT_EQUAL`, `FAIL`, `SUCCEED`, `TODO` se redefinen por fichero usando `__THIS_FILE__` estático para preservar el nombre de fichero correcto en los mensajes de error.

---

## 4. Bug fix: `getObjectId` — path LOC_CONTAINER

**Fichero**: `src/daad/daad_objects.c`

**Bug**: La condición para buscar objetos dentro de contenedores siempre fallaba:

```c
// BUGGY (antes):
location==LOC_CONTAINER && location<hdr->numObjDsc && objects[location].attribs.mask.isContainer
//  LOC_CONTAINER=256, numObjDsc≤255 → 256 < 255 es SIEMPRE false
```

**Fix**:

```c
// CORRECTO (ahora):
location==LOC_CONTAINER && objects[i].location<hdr->numObjDsc && objects[objects[i].location].attribs.mask.isContainer
// Comprueba si objects[i] está dentro de otro objeto que sea contenedor
```

**Impacto**:
- `AUTOT` ahora puede encontrar objetos que están dentro de un contenedor (path de búsqueda CONTAINER → CARRIED → WORN → HERE)
- `test_AUTOT_success` pasa de TODO a OK
- Ningún test regresionó (los tests existentes no usaban `LOC_CONTAINER`)

---

## 5. Validación contra la especificación

Todos los tests fueron validados contra tres fuentes documentales:

- `docs/DAAD_Manual_1991.md` — especificación oficial de condacts
- `docs/DAAD_Manual_2018.md` — revisión Tim Gilberts / Stefan Vogt
- `docs/Flags_for_Quill_PAWS_SWAN_and_DAAD.md` — referencia de flags del sistema
- `wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md` — referencia rápida MSX2

Resultado de la auditoría: **0 tests incorrectos**, casos corregidos durante la revisión:
- `test_WHATO_success`: añadidas verificaciones de `flags[56]` (fCOCon) y `flags[57]` (fCOWR) que el spec exige en "flags 54-57"
- `test_AUTOT_success`: renombrado el test original a `test_AUTOT_carried` (más preciso); `test_AUTOT_success` real implementado tras el bug fix de `getObjectId`
- `test_INPUT_indirection`: corregida notación `@75` (no estándar SDCC) → `75`

---

## 6. AUTOEXEC.BAT de la suite

```bat
ccc /r
del tests.txt
dos
condact1
condact2
condact3
condact4
condact5
condact6
objwght
daadobjs
gettoken
initobj
```

El resultado se escribe en `dsk/tests.txt` con formato `<fichero> :: <test> ... OK` o `### Assert failed ...` o `### TODO: ...`.

El target `make test` del Makefile lanza openMSX con esta imagen de disco y, al terminar, muestra el resumen:

```
========================================
 Test results  (dsk/tests.txt)
   OK:    292 / 323
   FAIL:    0 / 323
   TODO:   31 / 323
   TOTAL: 323
========================================
```

---

## 7. TODOs restantes (31) — justificación

| Razón | Condacts | N |
|-------|----------|---|
| UI output no verificable sin captura de pantalla | PAPER, INK, BORDER, SPACE, NEWLINE, MES, MESSAGE, SYSMES, DESC, PRINT, DPRINT, LISTOBJ×2, LISTAT×2 | 13 |
| No implementado en el motor | CALL, MOUSE | 2 |
| Requiere input interactivo o `die()` | PAUSE, END, EXIT | 3 |
| PROC stack complejo (tablas de proceso reales) | DOALL | 1 |
| Tablas DDB: Verbs/Connections | MOVE×2 | 2 |
| initObjects stub en contexto condacts | RESET | 1 |
| Input de teclado | QUIT | 1 |
| Audio/GFX sin estado verificable | SFX, GFX, PICTURE, DISPLAY, BEEP | 5 |
| EXTERN sin implementar | EXTERN | 1 |
| **Total** | | **29+2 = 31** |

---

## 8. Makefile — cambios relevantes

```makefile
# Versión del compilador
SDCC_VER := 4.5.0

# Docker monta el directorio padre para acceder a ../src/ y ../libs/
DOCKER_RUN = docker run -i --rm -u ... -v $(realpath $(CURDIR)/..):/src -w /src/unitTests $(DOCKER_IMG)

# Regla para fuentes en ../src/daad/*.c
$(OBJDIR)%.rel: $(DAAD_SRCDIR2)%.c

# Bibliotecas base por grupo
CONDACTS_BASE_LIBS := ... condacts_stubs.rel daad_condacts.rel daad_objects.rel daad_getObjectWeight.rel ...
DAAD_BASE_LIBS     := ... daad_stubs.rel ...   # SIN daad_condacts.rel

# Binarios
CONDACTS_PROGRAMS = condact1.com … condact6.com
DAAD_PROGRAMS     = objwght.com daadobjs.com gettoken.com initobj.com
PROGRAMS          = dos.com $(CONDACTS_PROGRAMS) $(DAAD_PROGRAMS)

# Resumen post-test
OK=$(grep -c "... OK$" dsk/tests.txt)
TODO=$(grep -c "### TODO:" dsk/tests.txt)   # Sin ^ (líneas con \r inicial)
FAIL=$(grep -c "### Assert failed" dsk/tests.txt)
```
