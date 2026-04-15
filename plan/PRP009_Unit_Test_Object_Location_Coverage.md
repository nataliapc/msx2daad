# PRP009 — Cobertura de tests: localizaciones especiales de objetos y WEIGH

- **ID**: PRP009
- **Tipo**: Testing — cobertura de casos límite
- **Ficheros afectados**: `unitTests/src/tests_condacts1.c`, `unitTests/src/tests_condacts3.c`, `unitTests/src/tests_condacts4.c`, `unitTests/src/tests_daad_getObjectWeight.c`
- **Severidad**: Mejora de calidad — sin cambios en producción
- **Fecha**: 2026-04-15
- **Estado**: Pendiente ⏳

---

## 1. Resumen

Análisis de las lagunas en la suite de unit tests respecto a los escenarios de localización de objetos definidos en la especificación DAAD. Los valores especiales de localización (`NOT_CREATED=252`, `LOC_WORN=253`, `LOC_CARRIED=254`) y el comportamiento de objetos dentro de contenedores están insuficientemente cubiertos en los condacts de condición (`PRESENT`, `ABSENT`, `WORN`, `NOTWORN`, `CARRIED`, `NOTCARR`, `ISAT`, `ISNOTAT`) y en los condacts `AUTO*`.

---

## 2. Marco de referencia: valores de localización (spec DAAD)

> Fuente primaria: `docs/DAAD_Manual_1991.md` (en adelante "Manual 1991")

| Valor | Constante | Significado | Ref. manual 1991 |
|-------|-----------|-------------|-----------------|
| 0–251 | *(localizaciones)* | Localización real del juego | L.853 |
| 252 | `NOT_CREATED` | El objeto no existe en el mundo del juego | L.855: _"252 or '_' (meaning not-created)"_ |
| 253 | `LOC_WORN` | Lo lleva puesto el jugador | L.856: _"253 or 'WORN'"_ |
| 254 | `LOC_CARRIED` | Lo porta el jugador | L.856: _"254 or 'CARRIED'"_ |
| 255 | `LOC_HERE` | Pseudo-valor solo para ISAT/ISNOTAT (loc actual del jugador) | L.856: _"255 or 'HERE' (which is converted into the current location of the player)"_ |

Un objeto dentro de un contenedor tiene como `.location` el **índice del objeto contenedor** (0–251). No es `LOC_CARRIED` aunque el contenedor esté portado. (L.1246: PUTIN establece `obj.location = locno`, donde `locno` es el índice del contenedor.)

### Semántica de los condacts de condición (spec)

| Condact | Condición de éxito | Ref. manual 1991 |
|---------|--------------------|-----------------|
| PRESENT | `obj.location == LOC_CARRIED` **OR** `LOC_WORN` **OR** `flags[fPlayer]` | L.914: _"Succeeds if Object objno. is carried, worn or at the current location"_ |
| ABSENT  | negación de PRESENT | L.919: _"Succeeds if Object objno. is not carried, not worn and not at the current location"_ |
| WORN    | `obj.location == LOC_WORN` | L.924: _"Succeeds if object objno. is worn"_ |
| NOTWORN | `obj.location != LOC_WORN` | L.928: _"Succeeds if Object objno. is not worn"_ |
| CARRIED | `obj.location == LOC_CARRIED` | L.932: _"Succeeds if Object objno. is carried"_ |
| NOTCARR | `obj.location != LOC_CARRIED` | L.936: _"Succeeds if Object objno. is not carried"_ |
| ISAT    | `obj.location == locno` (255 → `flags[fPlayer]`) | L.940: _"Succeeds if Object objno. is at Location locno"_ |
| ISNOTAT | `obj.location != locno` | L.944: _"Succeeds if Object objno. is not at Location locno"_ |

### Prioridades de búsqueda AUTO* (spec)

| Condact | Orden de búsqueda | Ref. manual 1991 |
|---------|------------------|-----------------|
| AUTOG   | here > carried > worn | L.1306: _"in order of location priority; here, carried, worn"_ |
| AUTOD   | carried > worn > here | L.1320: _"in order of location priority; carried, worn, here"_ |
| AUTOW   | carried > worn > here | L.1333: _"in order of location priority; carried, worn, here"_ |
| AUTOR   | worn > carried > here | L.1346: _"in order of location priority; worn, carried, here"_ |
| AUTOP   | carried > worn > here | L.1359: _"in order of location priority; carried, worn, here"_ |
| AUTOT   | container > carried > worn > here | L.1373: _"in order of location priority; in container, carried, worn, here"_ |

---

## 3. Lagunas identificadas

### 3.1. PRESENT — valores especiales no probados

**Fichero**: `unitTests/src/tests_condacts1.c`

Los tests actuales solo cubren:
- Objeto en la misma localización que el jugador → éxito
- Objeto en otra localización → falla
- Indirección con `LOC_HERE`

**Faltante:**

| Test a añadir | Escenario | Resultado esperado | Ref. manual 1991 |
|---|---|---|---|
| `test_PRESENT_carried` | `obj.location = LOC_CARRIED` | **éxito** | L.914: _"Succeeds if Object objno. is **carried**, worn or at the current location"_ |
| `test_PRESENT_worn` | `obj.location = LOC_WORN` | **éxito** | L.914: _"… is carried, **worn** or at the current location"_ |
| `test_PRESENT_not_created` | `obj.location = NOT_CREATED` | **falla** | L.914: NOT_CREATED no es carried, worn ni la loc del jugador; L.855: _"252 = not-created"_ |
| `test_PRESENT_in_container_at_loc` | obj dentro de contenedor en loc del jugador (`obj.location = índice_contenedor`, contenedor en `flags[fPlayer]`) | **falla** — `obj.location` es el índice del contenedor (0–251), no la loc del jugador | L.914 + L.1246: PUTIN establece `obj.location = locno` (índice del contenedor), que es distinto de la loc del jugador salvo coincidencia |
| `test_PRESENT_in_carried_container` | obj dentro de contenedor portado (`obj.location = índice_contenedor`, contenedor en `LOC_CARRIED`) | **falla** — `obj.location ≠ LOC_CARRIED` | L.914 + L.1246: misma razón; el objeto está en el contenedor, no directamente en el inventario |

> El caso del contenedor es el más crítico: es contraintuitivo que un objeto en una bolsa portada no esté PRESENT. La spec lo confirma (L.914): PRESENT solo comprueba `LOC_CARRIED`, `LOC_WORN` y `flags[fPlayer]` directamente. Los objetos dentro de un contenedor requieren TAKEOUT antes de ser accesibles.

### 3.2. ABSENT — valores especiales no probados

**Fichero**: `unitTests/src/tests_condacts1.c`

Los tests actuales solo cubren:
- Objeto en otra localización → éxito
- Objeto en la localización del jugador → falla

**Faltante:**

| Test a añadir | Escenario | Resultado esperado | Ref. manual 1991 |
|---|---|---|---|
| `test_ABSENT_carried` | `obj.location = LOC_CARRIED` | **falla** (objeto presente) | L.919: _"Succeeds if Object objno. is **not** carried, not worn and not at the current location"_ — carried lo niega |
| `test_ABSENT_worn` | `obj.location = LOC_WORN` | **falla** (objeto presente) | L.919: worn lo niega |
| `test_ABSENT_not_created` | `obj.location = NOT_CREATED` | **éxito** | L.919: NOT_CREATED no es ninguna de las tres condiciones; L.855: _"252 = not-created"_ |
| `test_ABSENT_in_carried_container` | obj dentro de contenedor portado | **éxito** (no está directamente presente) | L.919 + L.1246: `obj.location = índice_contenedor`, que no es carried, worn ni la loc del jugador |

### 3.3. WORN / NOTWORN — casos adicionales

**Fichero**: `unitTests/src/tests_condacts1.c`

`test_WORN_fails` solo comprueba objeto en localización normal; no distingue `LOC_CARRIED` de otros valores.

| Test a añadir | Escenario | Resultado esperado | Ref. manual 1991 |
|---|---|---|---|
| `test_WORN_carried` | `obj.location = LOC_CARRIED` | **falla** (portado ≠ puesto) | L.924: _"Succeeds if object objno. is **worn**"_ — carried (254) ≠ worn (253) |
| `test_WORN_not_created` | `obj.location = NOT_CREATED` | **falla** | L.924: NOT_CREATED (252) ≠ worn (253) |
| `test_NOTWORN_carried` | `obj.location = LOC_CARRIED` | **éxito** | L.928: _"Succeeds if Object objno. is **not** worn"_ — carried no es worn |
| `test_NOTWORN_not_created` | `obj.location = NOT_CREATED` | **éxito** | L.928: NOT_CREATED no es worn |

### 3.4. CARRIED / NOTCARR — casos adicionales

**Fichero**: `unitTests/src/tests_condacts1.c`

`test_CARRIED_fails` solo comprueba objeto en localización normal.

| Test a añadir | Escenario | Resultado esperado | Ref. manual 1991 |
|---|---|---|---|
| `test_CARRIED_worn` | `obj.location = LOC_WORN` | **falla** (puesto ≠ portado) | L.932: _"Succeeds if Object objno. is **carried**"_ — worn (253) ≠ carried (254) |
| `test_CARRIED_not_created` | `obj.location = NOT_CREATED` | **falla** | L.932: NOT_CREATED (252) ≠ carried (254) |
| `test_NOTCARR_worn` | `obj.location = LOC_WORN` | **éxito** | L.936: _"Succeeds if Object objno. is **not** carried"_ — worn no es carried |
| `test_NOTCARR_not_created` | `obj.location = NOT_CREATED` | **éxito** | L.936: NOT_CREATED no es carried |

### 3.5. ISAT / ISNOTAT — valores especiales como argumento `locno`

**Fichero**: `unitTests/src/tests_condacts1.c`

La spec permite usar 252, 253, 254 como `locno` (son valores de localización válidos). Solo se prueba localización normal y `LOC_HERE` (255).

| Test a añadir | Escenario | Resultado esperado | Ref. manual 1991 |
|---|---|---|---|
| `test_ISAT_loc_carried` | `ISAT obj LOC_CARRIED` con `obj.location=LOC_CARRIED` | **éxito** | L.940: _"Succeeds if Object objno. is at Location locno"_ + L.856: _"254 or 'CARRIED'"_ como locno+ válido |
| `test_ISAT_loc_worn` | `ISAT obj LOC_WORN` con `obj.location=LOC_WORN` | **éxito** | L.940 + L.856: _"253 or 'WORN'"_ como locno+ válido |
| `test_ISAT_loc_not_created` | `ISAT obj NOT_CREATED` con `obj.location=NOT_CREATED` | **éxito** | L.940 + L.855: _"252 or '_' (meaning not-created)"_ como locno+ válido |
| `test_ISAT_in_container` | `ISAT obj 0` con `obj.location=0` (dentro del contenedor obj0) | **éxito** | L.940 + L.853: _"locno. is a valid location number"_ — 0–251 son localizaciones válidas, incluida la de un contenedor |
| `test_ISAT_loc_carried_fails` | `ISAT obj LOC_CARRIED` con `obj.location=5` | **falla** | L.940: `obj.location(5) ≠ locno(254)` |
| `test_ISNOTAT_loc_carried` | `ISNOTAT obj LOC_CARRIED` con `obj.location=LOC_CARRIED` | **falla** | L.944: _"Succeeds if Object objno. is **not** at Location locno"_ — está exactamente en ese loc |
| `test_ISNOTAT_loc_worn` | `ISNOTAT obj LOC_WORN` con `obj.location=5` | **éxito** | L.944: `obj.location(5) ≠ locno(253)` |
| `test_ISNOTAT_in_container` | `ISNOTAT obj 0` con `obj.location=1` | **éxito** | L.944: `obj.location(1) ≠ locno(0)` |

### 3.6. Condacts AUTO* — solo el happy path de mayor prioridad

**Fichero**: `unitTests/src/tests_condacts3.c`

Cada condact AUTO* solo tiene test del primer caso en su orden de prioridad. Faltan los fallbacks y el test de prioridad con múltiples candidatos.

#### AUTOG (`here > carried > worn`)

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_AUTOG_priority_here_over_carried` | Dos objetos con mismo nounId: uno HERE, otro CARRIED → debe elegir el HERE | L.1306: _"in order of location priority; **here, carried, worn**"_ |
| `test_AUTOG_not_found` | Objeto existe en otra loc (no here/carried/worn) → SM26 | L.1311: _"SM26 ('There isn't one of those here.')"_ |

#### AUTOD (`carried > worn > here`)

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_AUTOD_worn` | Objeto worn (no carried): AUTOD lo encuentra via worn, llama a DROP → SM24 | L.1320: _"priority; **carried, worn, here**"_ + L.1147: DROP worn → SM24 _"I can't. I'm wearing the _."_ |
| `test_AUTOD_not_found` | Objeto en otra loc (no carried/worn/here) → SM28 | L.1324: _"SM28 ('I don't have one of those.')"_ |

#### AUTOW (`carried > worn > here`)

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_AUTOW_worn` | Objeto ya worn: AUTOW lo encuentra via worn, llama a WEAR → SM29 | L.1333: _"priority; **carried, worn, here**"_ + L.1168: WEAR worn → SM29 _"I'm already wearing the _."_ |
| `test_AUTOW_here` | Objeto here (no carried, no worn): AUTOW lo encuentra via here, llama a WEAR → SM49 | L.1333 + L.1164: WEAR en loc actual (no portado) → SM49 _"I don't have the _."_ (**no SM28**: ese sería si no está ni aquí) |

#### AUTOR (`worn > carried > here`)

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_AUTOR_carried` | Objeto carried (no worn): AUTOR lo encuentra via carried, llama a REMOVE → SM50 | L.1346: _"priority; **worn, carried, here**"_ + L.1183: REMOVE en carried → SM50 _"I'm not wearing the _."_ |
| `test_AUTOR_not_found` | Objeto en otra loc (no worn/carried/here) → SM23 | L.1350: _"SM23 ('I'm not wearing one of those.')"_ |

#### AUTOP (`carried > worn > here`)

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_AUTOP_worn` | Objeto worn: AUTOP lo encuentra via worn, llama a PUTIN → SM24 | L.1359: _"priority; **carried, worn, here**"_ + L.1235: PUTIN worn → SM24 _"I can't. I'm wearing the _."_ |
| `test_AUTOP_here` | Objeto here (no carried): AUTOP lo encuentra via here, llama a PUTIN → SM49 | L.1359 + L.1238: PUTIN en loc actual (no portado) → SM49 _"I don't have the _."_ |

#### AUTOT (`container > carried > worn > here`)

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_AUTOT_worn` | Objeto worn (no en contenedor): AUTOT cae a worn, llama a TAKEOUT → SM25 | L.1373: _"priority; **in container, carried, worn, here**"_ + L.1252: TAKEOUT worn → SM25 _"I already have the _."_ |
| `test_AUTOT_here` | Objeto here (no en contenedor, ni carried, ni worn): llama a TAKEOUT → SM45 | L.1373 + L.1255: TAKEOUT en loc actual → SM45 _"The _ isn't in the"_ |

---

## 4. Resumen de tests nuevos

| Fichero | Tests nuevos | Descripción |
|---------|-------------|-------------|
| `tests_condacts1.c` | 5 | PRESENT: LOC_CARRIED, LOC_WORN, NOT_CREATED, en contenedor en loc, en contenedor portado |
| `tests_condacts1.c` | 4 | ABSENT: LOC_CARRIED, LOC_WORN, NOT_CREATED, en contenedor portado |
| `tests_condacts1.c` | 4 | WORN/NOTWORN: LOC_CARRIED y NOT_CREATED |
| `tests_condacts1.c` | 4 | CARRIED/NOTCARR: LOC_WORN y NOT_CREATED |
| `tests_condacts1.c` | 8 | ISAT/ISNOTAT: LOC_CARRIED, LOC_WORN, NOT_CREATED, en contenedor |
| `tests_condacts3.c` | 2 | AUTOG: prioridad here>carried, objeto no encontrado |
| `tests_condacts3.c` | 2 | AUTOD: worn, no encontrado |
| `tests_condacts3.c` | 2 | AUTOW: worn (ya puesto), here |
| `tests_condacts3.c` | 2 | AUTOR: carried (no worn), no encontrado |
| `tests_condacts3.c` | 2 | AUTOP: worn, here |
| `tests_condacts3.c` | 2 | AUTOT: worn, here |
| **Total** | **37** | |

---

## 5. Plan de implementación

### Fase 1 — `tests_condacts1.c` (25 tests)

Añadir tras los bloques de tests existentes de cada condact, siguiendo el patrón BDD establecido.

#### 5.1 Bloque PRESENT (añadir tras `test_PRESENT_indirection`)

```c
void test_PRESENT_carried()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given player at loc 5 and object 1 is carried
    flags[fPlayer] = 5;
    objects[1].location = LOC_CARRIED;

    // BDD when checking PRESENT 1
    static const char proc[] = { _PRESENT, 1, 255 };
    do_action(proc);

    // BDD then success (carried counts as present)
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_PRESENT_worn()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given player at loc 5 and object 1 is worn
    flags[fPlayer] = 5;
    objects[1].location = LOC_WORN;

    // BDD when checking PRESENT 1
    static const char proc[] = { _PRESENT, 1, 255 };
    do_action(proc);

    // BDD then success (worn counts as present)
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_PRESENT_not_created()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given player at loc 5 and object 1 does not exist
    flags[fPlayer] = 5;
    objects[1].location = LOC_NOT_CREATED;

    // BDD when checking PRESENT 1
    static const char proc[] = { _PRESENT, 1, 255 };
    do_action(proc);

    // BDD then fails (not created = not present)
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_PRESENT_in_container_at_loc()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given player at loc 5, container obj0 also at loc 5,
    //           obj1 inside container (obj1.location = 0)
    flags[fPlayer] = 5;
    objects[0].location = 5;
    objects[0].attribs.mask.isContainer = 1;
    objects[1].location = 0;   // inside container obj0

    // BDD when checking PRESENT 1
    static const char proc[] = { _PRESENT, 1, 255 };
    do_action(proc);

    // BDD then fails: obj1.location=0 is not LOC_CARRIED, LOC_WORN, or fPlayer(5)
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_PRESENT_in_carried_container()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given player at loc 5, container obj0 carried,
    //           obj1 inside container (obj1.location = 0)
    flags[fPlayer] = 5;
    objects[0].location = LOC_CARRIED;
    objects[0].attribs.mask.isContainer = 1;
    objects[1].location = 0;   // inside carried container

    // BDD when checking PRESENT 1
    static const char proc[] = { _PRESENT, 1, 255 };
    do_action(proc);

    // BDD then fails: obj1 is not directly carried/worn/here
    //           (needs TAKEOUT first)
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### 5.2 Bloque ABSENT (añadir tras `test_ABSENT_indirection`)

```c
void test_ABSENT_carried()
{
    const char *_func = __func__;
    beforeEach();

    flags[fPlayer] = 5;
    objects[1].location = LOC_CARRIED;

    static const char proc[] = { _ABSENT, 1, 255 };
    do_action(proc);

    // carried = present, so ABSENT fails
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_ABSENT_worn()
{
    const char *_func = __func__;
    beforeEach();

    flags[fPlayer] = 5;
    objects[1].location = LOC_WORN;

    static const char proc[] = { _ABSENT, 1, 255 };
    do_action(proc);

    // worn = present, so ABSENT fails
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_ABSENT_not_created()
{
    const char *_func = __func__;
    beforeEach();

    flags[fPlayer] = 5;
    objects[1].location = LOC_NOT_CREATED;

    static const char proc[] = { _ABSENT, 1, 255 };
    do_action(proc);

    // not created = not present, so ABSENT succeeds
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_ABSENT_in_carried_container()
{
    const char *_func = __func__;
    beforeEach();

    flags[fPlayer] = 5;
    objects[0].location = LOC_CARRIED;
    objects[0].attribs.mask.isContainer = 1;
    objects[1].location = 0;   // inside carried container

    static const char proc[] = { _ABSENT, 1, 255 };
    do_action(proc);

    // obj inside container is not directly present, so ABSENT succeeds
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

#### 5.3 Bloque WORN / NOTWORN (añadir tras los tests existentes)

```c
void test_WORN_carried()
{
    const char *_func = __func__;
    beforeEach();

    flags[fPlayer] = 5;
    objects[1].location = LOC_CARRIED;

    static const char proc[] = { _WORN, 1, 255 };
    do_action(proc);

    // carried != worn
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_WORN_not_created()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_NOT_CREATED;

    static const char proc[] = { _WORN, 1, 255 };
    do_action(proc);

    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_NOTWORN_carried()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_CARRIED;

    static const char proc[] = { _NOTWORN, 1, 255 };
    do_action(proc);

    // carried != worn, so NOTWORN succeeds
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_NOTWORN_not_created()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_NOT_CREATED;

    static const char proc[] = { _NOTWORN, 1, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

#### 5.4 Bloque CARRIED / NOTCARR (añadir tras los tests existentes)

```c
void test_CARRIED_worn()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_WORN;

    static const char proc[] = { _CARRIED, 1, 255 };
    do_action(proc);

    // worn != carried
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_CARRIED_not_created()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_NOT_CREATED;

    static const char proc[] = { _CARRIED, 1, 255 };
    do_action(proc);

    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_NOTCARR_worn()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_WORN;

    static const char proc[] = { _NOTCARR, 1, 255 };
    do_action(proc);

    // worn != carried, so NOTCARR succeeds
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_NOTCARR_not_created()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_NOT_CREATED;

    static const char proc[] = { _NOTCARR, 1, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

#### 5.5 Bloque ISAT (añadir tras `test_ISAT_indirection`)

```c
void test_ISAT_loc_carried()
{
    const char *_func = __func__;
    beforeEach();

    // ISAT 1 LOC_CARRIED: object 1 is carried
    objects[1].location = LOC_CARRIED;

    static const char proc[] = { _ISAT, 1, LOC_CARRIED, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_ISAT_loc_worn()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_WORN;

    static const char proc[] = { _ISAT, 1, LOC_WORN, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_ISAT_loc_not_created()
{
    const char *_func = __func__;
    beforeEach();

    objects[1].location = LOC_NOT_CREATED;

    static const char proc[] = { _ISAT, 1, LOC_NOT_CREATED, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_ISAT_in_container()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 inside container obj0: obj1.location = 0
    objects[0].attribs.mask.isContainer = 1;
    objects[1].location = 0;

    // ISAT 1 0: is obj1 at location 0 (inside obj0)?
    static const char proc[] = { _ISAT, 1, 0, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_ISAT_loc_carried_fails()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 at a real location, not carried
    objects[1].location = 5;

    static const char proc[] = { _ISAT, 1, LOC_CARRIED, 255 };
    do_action(proc);

    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### 5.6 Bloque ISNOTAT (añadir tras `test_ISNOTAT_indirection`)

```c
void test_ISNOTAT_loc_carried()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 IS carried, so ISNOTAT LOC_CARRIED must fail
    objects[1].location = LOC_CARRIED;

    static const char proc[] = { _ISNOTAT, 1, LOC_CARRIED, 255 };
    do_action(proc);

    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_ISNOTAT_loc_worn()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 at normal location, not worn: ISNOTAT LOC_WORN succeeds
    objects[1].location = 5;

    static const char proc[] = { _ISNOTAT, 1, LOC_WORN, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_ISNOTAT_in_container()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 at location 1, not inside container obj0 (location 0)
    objects[1].location = 1;

    static const char proc[] = { _ISNOTAT, 1, 0, 255 };
    do_action(proc);

    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

#### 5.7 Actualización del `main` en `tests_condacts1.c`

```c
    // PRESENT
    test_PRESENT_success(); test_PRESENT_fails(); test_PRESENT_indirection();
    test_PRESENT_carried(); test_PRESENT_worn(); test_PRESENT_not_created();
    test_PRESENT_in_container_at_loc(); test_PRESENT_in_carried_container();
    // ABSENT
    test_ABSENT_success(); test_ABSENT_fails(); test_ABSENT_indirection();
    test_ABSENT_carried(); test_ABSENT_worn(); test_ABSENT_not_created();
    test_ABSENT_in_carried_container();
    // WORN / NOTWORN
    test_WORN_success(); test_WORN_fails(); test_WORN_indirection();
    test_WORN_carried(); test_WORN_not_created();
    test_NOTWORN_success(); test_NOTWORN_fails(); test_NOTWORN_indirection();
    test_NOTWORN_carried(); test_NOTWORN_not_created();
    // CARRIED / NOTCARR
    test_CARRIED_success(); test_CARRIED_fails(); test_CARRIED_indirection();
    test_CARRIED_worn(); test_CARRIED_not_created();
    test_NOTCARR_success(); test_NOTCARR_fails(); test_NOTCARR_indirection();
    test_NOTCARR_worn(); test_NOTCARR_not_created();
    // ISAT
    test_ISAT_success(); test_ISAT_fails(); test_ISAT_indirection();
    test_ISAT_loc_carried(); test_ISAT_loc_worn(); test_ISAT_loc_not_created();
    test_ISAT_in_container(); test_ISAT_loc_carried_fails();
    // ISNOTAT
    test_ISNOTAT_success(); test_ISNOTAT_fails(); test_ISNOTAT_indirection();
    test_ISNOTAT_loc_carried(); test_ISNOTAT_loc_worn(); test_ISNOTAT_in_container();
```

---

### Fase 2 — `tests_condacts3.c` (12 tests, AUTO*)

Añadir al final de cada bloque AUTO*, antes del `main`.

#### AUTOG

```c
void test_AUTOG_priority_here_over_carried()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 = carried with nounId=10, obj2 = HERE with nounId=10
    // AUTOG must prefer HERE (obj2) over CARRIED (obj1)
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    flags[fNOCarr] = 1;
    flags[fMaxCarr] = 4;
    flags[fStrength] = 20;
    objects[1].location = LOC_CARRIED;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;
    objects[1].attribs.mask.weight = 1;
    objects[2].location = 5;           // HERE
    objects[2].nounId = 10;
    objects[2].adjectiveId = NULLWORD;
    objects[2].attribs.mask.weight = 1;

    static const char proc[] = { _AUTOG, 255 };
    do_action(proc);

    // obj2 (here) must be picked up, not obj1 (already carried)
    ASSERT_EQUAL(objects[2].location, LOC_CARRIED, "AUTOG must pick up obj HERE first");
    ASSERT_EQUAL(objects[1].location, LOC_CARRIED, "obj1 already carried must remain carried");
    ASSERT_EQUAL(flags[fNOCarr], 2, ERROR_CARROBJNUM);
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

void test_AUTOG_not_found()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 exists but is at a different location (not here/carried/worn)
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[1].location = 3;   // at loc 3, player is at 5
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;

    static const char proc[] = { _AUTOG, 255 };
    do_action(proc);

    // object exists but not reachable: SM26 "There isn't one of those here."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 26, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### AUTOD

```c
void test_AUTOD_worn()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 worn (not carried): AUTOD(carried>worn>here) finds it via worn,
    // calls _internal_drop which fails: SM50 "I'm not wearing one of those."
    // Note: DROP on worn object prints SM24 "I can't. I'm wearing the _."
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[1].location = LOC_WORN;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;
    objects[1].attribs.mask.isWareable = 1;

    static const char proc[] = { _AUTOD, 255 };
    do_action(proc);

    // DROP on worn object prints SM24
    ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_AUTOD_not_found()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 exists at a different location, not reachable
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[1].location = 3;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;

    static const char proc[] = { _AUTOD, 255 };
    do_action(proc);

    // SM28 "I don't have one of those."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### AUTOW

```c
void test_AUTOW_worn()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 already worn: AUTOW finds it via worn, calls WEAR -> SM29
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[1].location = LOC_WORN;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;
    objects[1].attribs.mask.isWareable = 1;

    static const char proc[] = { _AUTOW, 255 };
    do_action(proc);

    // SM29 "I'm already wearing the _."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 29, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_AUTOW_here()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 here (at player's location, not carried, not worn):
    // AUTOW(carried>worn>here) finds it via here, calls WEAR.
    // WEAR spec (manual): "If Object objno. is at the current location
    //   (but not carried or worn) SM49 ('I don't have the _.') is printed"
    // Note: SM28 is for objects that are NOT at current location AND not carried.
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[1].location = 5;   // at player's location, not carried
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;
    objects[1].attribs.mask.isWareable = 1;

    static const char proc[] = { _AUTOW, 255 };
    do_action(proc);

    // SM49 "I don't have the _."  (object here but not carried)
    // Verified by: test_WEAR_isHere already expects SM49 for this case.
    ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### AUTOR

```c
void test_AUTOR_carried()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 carried (not worn): AUTOR(worn>carried>here) finds it via carried,
    // calls REMOVE -> SM50 "I'm not wearing one of those."
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    flags[fNOCarr] = 1;
    objects[1].location = LOC_CARRIED;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;
    objects[1].attribs.mask.isWareable = 1;

    static const char proc[] = { _AUTOR, 255 };
    do_action(proc);

    // SM50 "I'm not wearing the _."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 50, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_AUTOR_not_found()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 exists at a different location
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[1].location = 3;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;

    static const char proc[] = { _AUTOR, 255 };
    do_action(proc);

    // SM23 "I'm not wearing one of those."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 23, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### AUTOP

```c
void test_AUTOP_worn()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 worn: AUTOP(carried>worn>here) finds it via worn,
    // calls PUTIN -> SM24 "I can't. I'm wearing the _."
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[0].attribs.mask.isContainer = 1;
    objects[1].location = LOC_WORN;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;

    static const char proc[] = { _AUTOP, 0, 255 };
    do_action(proc);

    // SM24 "I can't. I'm wearing the _."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_AUTOP_here()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 here (not carried): AUTOP finds it via here,
    // calls PUTIN -> SM49 "I don't have the _."
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[0].attribs.mask.isContainer = 1;
    objects[1].location = 5;   // here
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;

    static const char proc[] = { _AUTOP, 0, 255 };
    do_action(proc);

    // SM49 "I don't have the _."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### AUTOT

```c
void test_AUTOT_worn()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 worn (not in container): AUTOT(container>carried>worn>here)
    // falls through container+carried, finds via worn,
    // calls TAKEOUT -> SM25 "I already have the _."
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[0].attribs.mask.isContainer = 1;
    objects[1].location = LOC_WORN;
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;

    static const char proc[] = { _AUTOT, 0, 255 };
    do_action(proc);

    // SM25 "I already have the _."
    ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}

void test_AUTOT_here()
{
    const char *_func = __func__;
    beforeEach();

    // obj1 here (not in container, not carried, not worn):
    // AUTOT falls through all, finds via here,
    // calls TAKEOUT -> SM45 "The _ isn't in the"
    flags[fPlayer] = 5;
    flags[fNoun1] = 10;
    flags[fAdject1] = NULLWORD;
    objects[0].attribs.mask.isContainer = 1;
    objects[0].location = 5;   // container also here
    objects[1].location = 5;   // obj1 here, not inside container
    objects[1].nounId = 10;
    objects[1].adjectiveId = NULLWORD;

    static const char proc[] = { _AUTOT, 0, 255 };
    do_action(proc);

    // SM45 "The _ isn't in the"
    ASSERT_EQUAL(fake_lastSysMesPrinted, 45, ERROR_SYSMES);
    ASSERT(!checkEntry, ERROR);
    SUCCEED();
}
```

#### 5.8 Actualización del `main` en `tests_condacts3.c`

```c
    test_AUTOG_carried(); test_AUTOG_worn(); test_AUTOG_success();
    test_AUTOG_priority_here_over_carried(); test_AUTOG_not_found();
    test_AUTOD_success(); test_AUTOD_worn(); test_AUTOD_not_found();
    test_AUTOW_success(); test_AUTOW_worn(); test_AUTOW_here();
    test_AUTOR_success(); test_AUTOR_carried(); test_AUTOR_not_found();
    test_AUTOP_success(); test_AUTOP_worn(); test_AUTOP_here();
    test_AUTOT_carried(); test_AUTOT_success();
    test_AUTOT_worn(); test_AUTOT_here();
```

---

---

## 6. Tests de WEIGH — edge cases no cubiertos

### 6.1 Marco de referencia (spec DAAD 1991)

> Fuente: `docs/DAAD_Manual_1991.md` **L.1445–1453**
>
> _"The true weight of Object objno. is calculated (i.e. if it is a container, any objects inside have their weight added — **don't forget that nested containers stop adding their contents after ten levels**) and the value is placed in Flag flagno. This will have a maximum value of **255 which will not be exceeded**. If Object objno. is a **container of zero weight**, Flag flagno. will be cleared as objects in zero weight containers, **also weigh zero!**"_

La implementación (`do_WEIGH`) delega en `getObjectWeight(objno, false)` y almacena el resultado en `flags[flagno]`. El campo `.weight` de un objeto es independiente de su `.location`.

### 6.2 Estado actual de los tests de WEIGH

**`tests_condacts4.c` — condact WEIGH:**

| Test | Escenario cubierto |
|------|--------------------|
| `test_WEIGH_success` | Objeto simple, peso 7 |
| `test_WEIGH_indirection` | Indirección en `objno` |
| `test_WEIGH_zero_weight_container` | Contenedor peso=0 con items → flag=0 |

**`tests_daad_getObjectWeight.c` — función `getObjectWeight`:**

| Test | Escenario cubierto |
|------|--------------------|
| `test_getObjectWeight_single` | Objeto simple |
| `test_getObjectWeight_nullword_carried_worn` | NULLWORD: suma carried+worn |
| `test_getObjectWeight_container_adds_contents` | Contenedor no-cero + items |
| `test_getObjectWeight_zero_weight_magic_bag` | Contenedor peso=0 → 0 |
| `test_getObjectWeight_cap_255` | Saturación en 255 |

### 6.3 Gaps identificados

#### En `tests_condacts4.c` — condact WEIGH

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_WEIGH_container_with_contents` | Contenedor no-cero con items dentro → suma contenedor+items en `flagno` | L.1447: _"if it is a container, any objects inside have their weight added"_ |
| `test_WEIGH_cap_at_255` | Contenedor pesado → resultado capado en `flagno=255` | L.1450: _"maximum value of 255 which will not be exceeded"_ |
| `test_WEIGH_empty_container` | Contenedor sin items dentro → solo el peso del contenedor | L.1447: sin objetos dentro, la suma de contenidos es 0; solo cuenta el peso propio |
| `test_WEIGH_not_created_object` | Objeto con `location=NOT_CREATED`, peso=7 → `flagno=7` | L.1447: _"The true weight of Object objno. is calculated"_ — el campo `.weight` existe independientemente del estado de creación |
| `test_WEIGH_nested_containers` | A(w=2) → B dentro de A(w=2, contenedor) → C dentro de B(w=2) → `flagno=6` | L.1448: _"nested containers stop adding their contents after **ten levels**"_ |
| `test_WEIGH_location_irrelevant` | Objeto portado (`LOC_CARRIED`, w=5) → `flagno=5` | L.1447: WEIGH calcula el peso del objeto, no considera su localización |

#### En `tests_daad_getObjectWeight.c` — función `getObjectWeight`

| Test a añadir | Escenario | Ref. manual 1991 |
|---|---|---|
| `test_getObjectWeight_nullword_no_objects` | NULLWORD sin nada portado/puesto → 0 | L.1542: WEIGHT _"calculates the true weight of **all objects carried and worn**"_ — suma vacía = 0 |
| `test_getObjectWeight_nullword_with_carried_container` | Player porta contenedor(w=5) con item(w=3) dentro → NULLWORD=8 | L.1543: _"any containers will have the weight of their contents added"_ |
| `test_getObjectWeight_nullword_magic_bag_carried` | Player porta magic bag(w=0) con item(w=50) dentro + item normal(w=3) → NULLWORD=3 | L.1451: _"container of zero weight, Flag flagno. will be cleared as objects in zero weight containers, **also weigh zero!**"_ |
| `test_getObjectWeight_empty_container` | Contenedor(w=10) sin items → getObjectWeight=10 | L.1447: sin objetos dentro, solo cuenta el peso propio del contenedor |
| `test_getObjectWeight_nested_containers` | A(w=2, contenedor) → B(w=3, contenedor) dentro de A → C(w=4) dentro de B → resultado=9 | L.1448: _"nested containers stop adding their contents after ten levels"_ |

### 6.4 Implementación — `tests_condacts4.c`

Añadir tras `test_WEIGH_zero_weight_container`:

```c
// =============================================================================
// Tests WEIGH - container with non-zero weight and contents
void test_WEIGH_container_with_contents()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given obj3 = non-zero container (w=5), obj4 inside (w=3)
    // Spec: "if it is a container, any objects inside have their weight added"
    objects[3].attribs.mask.isContainer = 1;
    objects[3].attribs.mask.weight = 5;
    objects[4].location = 3;    // inside container obj3
    objects[4].attribs.mask.weight = 3;

    // BDD when WEIGH 3 100
    static const char proc[] = { _WEIGH, 3, 100, 255 };
    do_action(proc);

    // BDD then flags[100] = 8 (5 + 3)
    ASSERT_EQUAL(flags[100], 8, "WEIGH container must include contents weight (5+3=8)");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

// =============================================================================
// Tests WEIGH - result is capped at 255
void test_WEIGH_cap_at_255()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given obj3 = container (w=63), objs 4-7 inside (w=63 each)
    // 63 + 4*63 = 315 > 255  -> must be capped at 255
    // Spec: "maximum value of 255 which will not be exceeded"
    objects[3].attribs.mask.isContainer = 1;
    objects[3].attribs.mask.weight = 63;
    objects[4].location = 3; objects[4].attribs.mask.weight = 63;
    objects[5].location = 3; objects[5].attribs.mask.weight = 63;
    objects[6].location = 3; objects[6].attribs.mask.weight = 63;
    objects[7].location = 3; objects[7].attribs.mask.weight = 63;

    // BDD when WEIGH 3 100
    static const char proc[] = { _WEIGH, 3, 100, 255 };
    do_action(proc);

    // BDD then flags[100] = 255 (capped)
    ASSERT_EQUAL(flags[100], 255, "WEIGH must cap result at 255");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

// =============================================================================
// Tests WEIGH - empty container (no contents)
void test_WEIGH_empty_container()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given obj3 = non-zero container (w=10) with no objects inside
    // WEIGH must return only the container's own weight
    objects[3].attribs.mask.isContainer = 1;
    objects[3].attribs.mask.weight = 10;

    // BDD when WEIGH 3 100
    static const char proc[] = { _WEIGH, 3, 100, 255 };
    do_action(proc);

    // BDD then flags[100] = 10 (container weight only, nothing inside)
    ASSERT_EQUAL(flags[100], 10, "WEIGH empty container must return only its own weight");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

// =============================================================================
// Tests WEIGH - NOT_CREATED object: location does not affect weight
void test_WEIGH_not_created_object()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given obj3 with location=NOT_CREATED but weight field=7
    // Spec does not filter by location; WEIGH reads the weight field directly.
    // A destroyed object still has its weight in the object table.
    objects[3].location = LOC_NOT_CREATED;
    objects[3].attribs.mask.weight = 7;

    // BDD when WEIGH 3 100
    static const char proc[] = { _WEIGH, 3, 100, 255 };
    do_action(proc);

    // BDD then flags[100] = 7 (weight is independent of creation status)
    ASSERT_EQUAL(flags[100], 7, "WEIGH must return weight regardless of object location");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

// =============================================================================
// Tests WEIGH - nested containers: recursive weight accumulation
void test_WEIGH_nested_containers()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given:
    //   obj3 = container, weight=2
    //   obj4 = inside obj3, container, weight=2
    //   obj5 = inside obj4, plain object, weight=2
    // Spec: "nested containers stop adding their contents after ten levels"
    // One level of nesting: obj3 contains obj4 (container), obj4 contains obj5.
    // Expected: 2 (obj3) + 2 (obj4) + 2 (obj5) = 6
    objects[3].attribs.mask.isContainer = 1;
    objects[3].attribs.mask.weight = 2;
    objects[4].location = 3;   // inside obj3
    objects[4].attribs.mask.isContainer = 1;
    objects[4].attribs.mask.weight = 2;
    objects[5].location = 4;   // inside obj4
    objects[5].attribs.mask.weight = 2;

    // BDD when WEIGH 3 100
    static const char proc[] = { _WEIGH, 3, 100, 255 };
    do_action(proc);

    // BDD then flags[100] = 6 (2 + 2 + 2, recursive sum)
    ASSERT_EQUAL(flags[100], 6, "WEIGH nested containers must accumulate weights recursively");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}

// =============================================================================
// Tests WEIGH - object location (CARRIED/WORN) does not affect result
void test_WEIGH_location_irrelevant()
{
    const char *_func = __func__;
    beforeEach();

    // BDD given obj3 carried (LOC_CARRIED), weight=5
    // WEIGH reads the .weight field; .location is irrelevant for WEIGH.
    objects[3].location = LOC_CARRIED;
    objects[3].attribs.mask.weight = 5;

    // BDD when WEIGH 3 100
    static const char proc[] = { _WEIGH, 3, 100, 255 };
    do_action(proc);

    // BDD then flags[100] = 5 (same result as if obj3 were at any location)
    ASSERT_EQUAL(flags[100], 5, "WEIGH result must be independent of object location");
    ASSERT(checkEntry, ERROR);
    SUCCEED();
}
```

#### Actualización del `main` en `tests_condacts4.c`

```c
    test_WEIGH_success(); test_WEIGH_indirection();
    test_WEIGH_zero_weight_container();
    test_WEIGH_container_with_contents();
    test_WEIGH_cap_at_255();
    test_WEIGH_empty_container();
    test_WEIGH_not_created_object();
    test_WEIGH_nested_containers();
    test_WEIGH_location_irrelevant();
```

### 6.5 Implementación — `tests_daad_getObjectWeight.c`

Añadir tras `test_getObjectWeight_cap_255`:

```c
// Test NULLWORD with no carried/worn objects
void test_getObjectWeight_nullword_no_objects()
{
    const char *_func = __func__;
    daad_beforeEach();

    // BDD given no objects carried or worn (all locations = 0 after reset,
    // but 0 is container-index territory, not LOC_CARRIED/LOC_WORN)
    // Explicitly: no object has location==LOC_CARRIED or LOC_WORN
    // (daad_beforeEach already memsets locations to 0, which != LOC_CARRIED/LOC_WORN)

    // BDD when getObjectWeight(NULLWORD, true)
    uint8_t result = getObjectWeight(NULLWORD, true);

    // BDD then result = 0 (nothing carried or worn)
    ASSERT_EQUAL(result, 0, "NULLWORD with nothing carried/worn must return 0");
    SUCCEED();
}

// Test NULLWORD with carried container (non-zero weight) that has contents
void test_getObjectWeight_nullword_with_carried_container()
{
    const char *_func = __func__;
    daad_beforeEach();

    // BDD given:
    //   obj1 = carried container (w=5), with obj2 inside (w=3)
    //   WEIGHT condact calls getObjectWeight(NULLWORD, true)
    //   Spec for WEIGHT: "any containers will have the weight of their
    //   contents added" (same recursive logic as WEIGH)
    objects[1].location = LOC_CARRIED;
    objects[1].attribs.mask.isContainer = 1;
    objects[1].attribs.mask.weight = 5;
    objects[2].location = 1;   // inside container obj1
    objects[2].attribs.mask.weight = 3;

    // BDD when getObjectWeight(NULLWORD, true)
    uint8_t result = getObjectWeight(NULLWORD, true);

    // BDD then result = 8 (container 5 + contents 3)
    ASSERT_EQUAL(result, 8, "NULLWORD must include contents of carried non-zero containers (5+3=8)");
    SUCCEED();
}

// Test NULLWORD with magic bag carried (zero-weight container): contents do not count
void test_getObjectWeight_nullword_magic_bag_carried()
{
    const char *_func = __func__;
    daad_beforeEach();

    // BDD given:
    //   obj1 = magic bag (carried, container, w=0), obj2 inside (w=50)
    //   obj3 = normal carried object (w=3)
    // Spec: "objects in zero weight containers, also weigh zero"
    // So obj1 (0) + obj2 (0, inside magic bag) + obj3 (3) = 3
    objects[1].location = LOC_CARRIED;
    objects[1].attribs.mask.isContainer = 1;
    objects[1].attribs.mask.weight = 0;    // magic bag
    objects[2].location = 1;              // inside magic bag
    objects[2].attribs.mask.weight = 50;  // must NOT count
    objects[3].location = LOC_CARRIED;
    objects[3].attribs.mask.weight = 3;

    // BDD when getObjectWeight(NULLWORD, true)
    uint8_t result = getObjectWeight(NULLWORD, true);

    // BDD then result = 3 (magic bag + its contents = 0; only obj3 counts)
    ASSERT_EQUAL(result, 3, "NULLWORD with magic bag: bag and contents weigh 0, only other carried items count");
    SUCCEED();
}

// Test container with no items inside
void test_getObjectWeight_empty_container()
{
    const char *_func = __func__;
    daad_beforeEach();

    // BDD given obj1 = container (w=10) with no objects inside
    // Note: use obj1 (not obj0) to avoid the location=0 ambiguity on reset
    objects[1].attribs.mask.isContainer = 1;
    objects[1].attribs.mask.weight = 10;

    // BDD when getObjectWeight(1, false)
    uint8_t result = getObjectWeight(1, false);

    // BDD then result = 10 (no contents to add)
    ASSERT_EQUAL(result, 10, "Empty container must return only its own weight");
    SUCCEED();
}

// Test nested containers: recursive weight accumulation
void test_getObjectWeight_nested_containers()
{
    const char *_func = __func__;
    daad_beforeEach();

    // BDD given:
    //   obj1 = container (w=2)
    //   obj2 = inside obj1, container (w=3)
    //   obj3 = inside obj2, plain object (w=4)
    // Spec: "nested containers stop adding their contents after ten levels"
    // Expected: 2 + 3 + 4 = 9
    objects[1].attribs.mask.isContainer = 1;
    objects[1].attribs.mask.weight = 2;
    objects[2].location = 1;   // inside obj1
    objects[2].attribs.mask.isContainer = 1;
    objects[2].attribs.mask.weight = 3;
    objects[3].location = 2;   // inside obj2
    objects[3].attribs.mask.weight = 4;

    // BDD when getObjectWeight(1, false)
    uint8_t result = getObjectWeight(1, false);

    // BDD then result = 9 (2 + 3 + 4, two levels of nesting)
    ASSERT_EQUAL(result, 9, "Nested containers must accumulate weights recursively (2+3+4=9)");
    SUCCEED();
}
```

#### Actualización del `main` en `tests_daad_getObjectWeight.c`

```c
    test_getObjectWeight_single();
    test_getObjectWeight_nullword_carried_worn();
    test_getObjectWeight_container_adds_contents();
    test_getObjectWeight_zero_weight_magic_bag();
    test_getObjectWeight_cap_255();
    test_getObjectWeight_nullword_no_objects();
    test_getObjectWeight_nullword_with_carried_container();
    test_getObjectWeight_nullword_magic_bag_carried();
    test_getObjectWeight_empty_container();
    test_getObjectWeight_nested_containers();
```

---

## 7. Impacto esperado

| Fichero | Tests actuales | Tests nuevos | Total esperado |
|---------|---------------|-------------|----------------|
| `condact1.com` | 36 | +25 | **61** |
| `condact3.com` | 62 | +12 | **74** |
| `condact4.com` | 44 | +6 | **50** |
| `objwght.com` | 5 | +5 | **10** |
| **Suite total** | **323** | **+48** | **~371** |

No se espera ningún cambio en el código de producción. Si algún test falla, señalará un bug en la implementación del condact correspondiente.

---

## 8. Notas de implementación

- La constante `LOC_NOT_CREATED` debe estar disponible en `condacts_stubs.h` (verificar que `NOT_CREATED=252` está definido o usar el valor numérico directamente).
- En los tests de contenedor, `objects[0]` se evita como contenedor porque `daad_beforeEach()` inicializa todas las `.location` a 0, lo que haría que todos los objetos parecieran estar dentro de obj0. Usar `objects[1]` o superior.
- El test `test_AUTOG_priority_here_over_carried` necesita dos objetos con el mismo `nounId`; verificar que `getObjectId` con prioridad HERE los distingue correctamente (cobertura del bug fix de PRP008).
- Los mensajes SM esperados para AUTO* se verifican consultando la tabla de mensajes del sistema en la spec (sección "System Messages").
- `test_WEIGH_nested_containers` y `test_getObjectWeight_nested_containers` validan la recursión a 2 niveles. La spec menciona un límite de 10 niveles; la implementación actual no tiene un contador de profundidad explícito — estos tests documentan el comportamiento observado.
