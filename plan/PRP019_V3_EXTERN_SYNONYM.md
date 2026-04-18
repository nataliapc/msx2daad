# PRP019 — V3: EXTERN skips MALUVA + SYNONYM no longer marks DONE

- **ID**: PRP019
- **Tipo**: Nueva funcionalidad V3 + bugfix
- **Ficheros afectados**:
  - [`src/daad_condacts.c`](../src/daad_condacts.c)
  - [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c)
  - [`unitTests/src/tests_condacts6.c`](../unitTests/src/tests_condacts6.c)
  - [`unitTests/src/tests_condacts_v3.c`](../unitTests/src/tests_condacts_v3.c)
- **Severidad**: MINOR — no hay crash, pero SYNONYM no marca DONE en V2 (comportamiento incorrecto)
- **Fecha**: 2026-04-18
- **Estado**: Completado ✅
- **Fuente**: [`assets/V3/DAAD_V3_full.md §6 y §7`](../assets/V3/DAAD_V3_full.md)

---

## 1. Resumen de los 2 issues

| ID | Descripción | Acción |
|----|-------------|--------|
| V3-11 | `do_EXTERN()`: en DDB V3 no llamar a rutinas MALUVA | Comentario incorrecto + verificar estado actual |
| V3-12 | `do_SYNONYM()`: debe marcar DONE en V2 (Z80/6502); en V3 no | **Fix**: añadir `isDone = true` bajo `#ifndef DAADV3 / !ISV3` |

---

## 2. Análisis y código

### V3-11 — `do_EXTERN()`: MALUVA deprecated en V3

**Fichero**: [`src/daad_condacts.c:2388-2428`](../src/daad_condacts.c#L2388-L2428)

**Especificación** ([`assets/V3/DAAD_V3_full.md:276-286`](../assets/V3/DAAD_V3_full.md#L276-L286)):
> If the DDB is V3, MALUVA functions are not called inside EXTERN.  
> Any DDB that contains a MALUVA call will produce undefined behaviour on a V3 interpreter.

**Estado actual**: La lógica de skip **ya está implementada**:

```c
void do_EXTERN()	// value routine
{
    uint16_t value = (uint16_t)getValueOrIndirection();
    uint8_t  routine = *pPROC++;

#ifdef DAADV3
    if (!ISV3)	// ← COMENTARIO INCORRECTO: dice "Only execute if we're in V3"
                //   pero la condición correcta es "skip if V3" (= ejecutar si NO V3)
#endif
    {
        switch (routine) { ... }
    }
}
```

La condición `if (!ISV3)` es **correcta**: cuando ISV3=true (DDB V3), salta el switch (MALUVA no ejecuta). El comentario es lo que está mal.

**Caso especial — `EXTERN <lsb> 3 <msb>` (XMES vía MALUVA) en DDB V3**:
Si un DDB V3 contuviera un `EXTERN` con `routine=3`, el byte extra `msb` (dentro de case 3: `value |= (*pPROC++)<<8`) NO sería consumido → desincronización de `pPROC`. Sin embargo, la spec es explícita: esto es **comportamiento indefinido** y los DDBs V3 usan el opcode nativo 120 (`do_XMES`) para mensajes externos. No se requiere fix de pPROC.

**Fix requerido**: solo corrección del comentario incorrecto.

```diff
 #ifdef DAADV3
-    if (!ISV3)	// Only execute the switch if we're in V3
+    if (!ISV3)	// V3: skip MALUVA routines (undefined behavior per spec §6)
 #endif
```

---

### V3-12 — `do_SYNONYM()`: no marca DONE en V2

**Fichero**: [`src/daad_condacts.c:2149-2157`](../src/daad_condacts.c#L2149-L2157)

**Especificación** ([`assets/V3/DAAD_V3_full.md:289-292`](../assets/V3/DAAD_V3_full.md#L289-L292)):
> In V3, `SYNONYM` no longer marks DONE. This was found to already be the case in the 68k sources, where `SYNONYM` had stopped marking DONE (unlike the Z80 and 6502 versions, which still did).

**Estado actual**: SYNONYM no marca DONE en ningún caso — correcto para V3/68k, **incorrecto para V2 (Z80/6502)**:

```c
void do_SYNONYM()	// verb noun
{
    uint8_t value = getValueOrIndirection();
    if (value!=NULLWORD) flags[fVerb] = value;
    value = *pPROC++;
    if (value!=NULLWORD) flags[fNoun1] = value;
    // ← falta isDone = true para V2
}
```

**Comportamiento esperado por plataforma**:

| Plataforma | `isDone` tras SYNONYM | Notas |
|-----------|----------------------|-------|
| Z80 (DAAD V2) | **true** | Comportamiento original; actualmente falta |
| 6502 (DAAD V2) | **true** | Idem |
| 68k (DAAD V2) | false | Las fuentes 68k ya no marcaban DONE |
| V3 (cualquier plataforma) | false | Nuevo comportamiento V3 explícito |

**Fix**:

```diff
 void do_SYNONYM()	// verb noun
 {
     uint8_t value = getValueOrIndirection();
     if (value!=NULLWORD) flags[fVerb] = value;
     value = *pPROC++;
     if (value!=NULLWORD) flags[fNoun1] = value;
+#ifdef DAADV3
+    if (!ISV3) isDone = true;
+#else
+    isDone = true;
+#endif
 }
```

Desglose:
- Sin `-DDAADV3`: `isDone = true` siempre → comportamiento Z80/6502 V2 ✓
- Con `-DDAADV3` + DDB V2 (`ISV3=false`): `isDone = true` → preserva comportamiento V2 ✓
- Con `-DDAADV3` + DDB V3 (`ISV3=true`): no establece `isDone` → comportamiento V3 ✓

---

## 3. Tests unitarios

### V3-12 — SYNONYM DONE

Los tests de SYNONYM ya existen en `unitTests/src/tests_condacts6.c` (funciones `test_SYNONYM_success`, `test_SYNONYM_success_verb`, `test_SYNONYM_success_noun`). Actualmente no comprueban `isDone`. Hay que añadir el assert:

```diff
 void test_SYNONYM_success()
 {
     // ... setup ...
     do_action(proc);

     ASSERT_EQUAL(flags[fVerb],  3, "SYNONYM verb must be set");
     ASSERT_EQUAL(flags[fNoun1], 4, "SYNONYM noun must be set");
+    ASSERT_EQUAL(isDone, true,     "SYNONYM must mark DONE in V2");
     SUCCEED();
 }
```

Añadir el assert a los 3 tests existentes (`test_SYNONYM_success`, `test_SYNONYM_success_verb`, `test_SYNONYM_success_noun`).

Para el comportamiento V3, añadir en `unitTests/src/tests_condacts_v3.c`:

```c
/* SYNONYM in V3 DDB: must NOT mark DONE */
void test_SYNONYM_v3_no_done()
{
    const char *_func = __func__;
    beforeEach();
    setV3();                                    // ISV3 = true
    flags[fVerb]  = 1;
    flags[fNoun1] = 2;
    isDone = false;

    static char proc[] = { _SYNONYM, 3, 4, 255 };
    do_action(proc);

    ASSERT_EQUAL(flags[fVerb],  3,     "SYNONYM must substitute verb");
    ASSERT_EQUAL(flags[fNoun1], 4,     "SYNONYM must substitute noun");
    ASSERT_EQUAL(isDone,        false, "SYNONYM must NOT mark DONE in V3");
    SUCCEED();
}
```

---

## 4. Plan de implementación

| Paso | Acción | Fichero | Estado |
|------|--------|---------|--------|
| 1 | Corregir comentario en `do_EXTERN()` (V3-11) | [`src/daad_condacts.c:2396`](../src/daad_condacts.c#L2396) | ✅ |
| 2 | Añadir `isDone = true` en `do_SYNONYM()` con guards `#ifdef DAADV3` (V3-12) | [`src/daad_condacts.c:2155`](../src/daad_condacts.c#L2155) | ✅ |
| 3 | Añadir assert `isDone == true` a los 3 tests SYNONYM V2 existentes | [`unitTests/src/tests_condacts6.c:245-295`](../unitTests/src/tests_condacts6.c#L245-L295) | ✅ |
| 4 | Añadir `test_SYNONYM_v3_no_done()` en `tests_condacts_v3.c` | [`unitTests/src/tests_condacts_v3.c`](../unitTests/src/tests_condacts_v3.c) | ✅ |
| 5 | `make test` — 423 OK, 0 FAIL | — | ✅ |

### Cambio adicional — `GFX_CLEAR_BACK/PHYS` fuera de `#ifndef DISABLE_GFX`

**Fichero**: [`src/daad_platform_msx2.c:960-997`](../src/daad_platform_msx2.c#L960-L997)

Los casos `GFX_CLEAR_BACK` (6) y `GFX_CLEAR_PHYS` (5) estaban dentro del bloque `#ifndef DISABLE_GFX`, pero su implementación solo usa `bitBlt` y `getColor` — funciones disponibles independientemente de `DISABLE_GFX`. Al moverlos fuera del bloque, funcionan también en compilaciones sin GFX (e.g. targets de texto puro). No afecta a builds con GFX activo.

---

## 5. Referencias

| Fuente | Localización |
|--------|-------------|
| Especificación V3 §6 | [`assets/V3/DAAD_V3_full.md:276-286`](../assets/V3/DAAD_V3_full.md#L276-L286) |
| Especificación V3 §7 | [`assets/V3/DAAD_V3_full.md:289-292`](../assets/V3/DAAD_V3_full.md#L289-L292) |
| Código `do_EXTERN()` | [`src/daad_condacts.c:2388-2428`](../src/daad_condacts.c#L2388-L2428) |
| Código `do_SYNONYM()` | [`src/daad_condacts.c:2149-2157`](../src/daad_condacts.c#L2149-L2157) |
| Tests SYNONYM existentes | [`unitTests/src/tests_condacts6.c:245-295`](../unitTests/src/tests_condacts6.c#L245-L295) |
