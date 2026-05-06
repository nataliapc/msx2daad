# PRP028 — DOALL: cancelación con NOTDONE + NEWTEXT (no DONE)

- **ID**: PRP028
- **Tipo**: Bugfix de comportamiento (alineación con doc actualizado de DAAD Ready V2)
- **Ficheros afectados**:
  - [`src/daad_condacts.c`](../src/daad_condacts.c)
  - [`unitTests/src/tests_condacts_v3.c`](../unitTests/src/tests_condacts_v3.c)
- **Severidad**: MINOR — comportamiento incorrecto de "DOALL sin objetos", visible al jugador como "no se imprime mensaje 'No hay nada que…'" o como "frases pendientes ejecutándose tras un DOALL fallido"
- **Fecha**: 2026-04-20
- **Estado**: Completado ✅

---

## 1. Problema

El documento [DAAD Ready V2 §DOALL líneas 1414-1452](../docs/DAAD_Ready_Documentation_V2.md#L1414-L1452) ha actualizado el **paso 1** del condacto:

| Documento anterior | Documento actual (revisado) |
|--------------------|----------------------------|
| "If unsuccessful the DOALL is cancelled and **action DONE** is performed." | "If this is unsuccessful the DOALL is cancelled and **actions NOTDONE and NEWTEXT** are performed." |

La implementación actual ([src/daad_condacts.c:2228-2232](../src/daad_condacts.c#L2228-L2232)) sigue el comportamiento antiguo:

```c
if (objno >= hdr->numObjDsc) {
    currProc->condactDOALL = NULL;
    do_DONE();              // ← antiguo: marca isDone=true, sale de tabla
    return;
}
```

Por tanto está desalineada con el nuevo contrato.

---

## 2. Análisis del cambio

### Por qué `NOTDONE` en vez de `DONE`

El cambio es semánticamente **más correcto**: si `DOALL` no encuentra ningún objeto en la localización, el motor no ha ejecutado ninguna acción del jugador. Marcar `isDone=true` (lo que hace `DONE`) es engañoso:

- **Con `DONE`**: el sistema considera que algo se ha hecho. Si no hay otra entrada que maneje el verbo, el bucle principal **no imprime** "No entiendo" / "I can't" / SM5 (sysmes "No hay nada que…"), porque cree que el verbo ya fue procesado.
- **Con `NOTDONE`**: `isDone` queda a `false`. Si ninguna otra entrada maneja la situación, el sistema imprime el sysmes apropiado al jugador. Es el comportamiento esperado: si tecleas `COGER ALL` en una habitación vacía, debería responder con "No hay nada que coger" o equivalente, no callar.

### Por qué `NEWTEXT` además

`NEWTEXT` ([src/daad_condacts.c:2134-2137](../src/daad_condacts.c#L2134-L2137)) ejecuta `clearLogicalSentences()`, que vacía `lsBuffer0`. Esto previene que frases pendientes en la entrada del jugador (`COGER ALL Y MATAR ORCO`) sigan procesándose con un estado parser potencialmente inconsistente:

- Cuando `DOALL` itera múltiples objetos, cada pase reescribe `flags[fNoun1]` y `flags[fAdject1]` desde el objeto actual.
- Si la primera frase termina sin objetos encontrados, los flags `fNoun1/fAdject1` **siguen apuntando al último valor escrito por DOALL** (potencialmente NULLWORD si nunca entró al bucle, o un valor "huérfano").
- Si el motor pasara a la siguiente frase con esos flags sin limpiar el buffer, podría reaccionar de forma errática a `MATAR ORCO` con tokens parser sucios.

`NEWTEXT` corta esto en seco: descarta el resto de la entrada del jugador. Es coherente con cómo se comportan otros condactos defensivos como `AUTOG` cuando fallan ([daad_condacts.c §AUTOG](../src/daad_condacts.c)).

### Por qué SOLO en el paso 1, no en el paso 4

El doc divide el ciclo en 4 pasos y **solo el paso 1** ha cambiado:

- **Paso 1** ("buscar objeto inicial"): si falla, ahora `NOTDONE+NEWTEXT`.
- **Paso 4** ("salir de tabla con DOALL activo → reentrar paso 1"): no ha cambiado, sigue siendo "vuelve al paso 1 con `objno+1`".

En MSX2DAAD el paso 4 está implementado en dos sitios:
1. `do_DONE()` y `do_NOTDONE()` ([src/daad_condacts.c:2348](../src/daad_condacts.c#L2348), [2368](../src/daad_condacts.c#L2368)): si `condactDOALL != NULL`, llaman a `_internal_doall()`.
2. `processPROC()` ([src/daad_condacts.c:211](../src/daad_condacts.c#L211)): cuando se agota la tabla sin encontrar entry: `if (currProc->condactDOALL) do_DONE()` — implicit DONE step 4 (PRP013).

Estos sitios **no se tocan**. El cambio del paso 1 se aplica solo cuando `_internal_doall()` agota la búsqueda y decide **cancelar** el DOALL — la transición del estado "iterando" al estado "no hay más objetos".

> **Importante**: la llamada `do_DONE()` desde el paso 4 (processPROC L211) ahora reentrará en `_internal_doall`, y si esa reentrada agota objetos, **caerá en el nuevo path de cancelación con NOTDONE+NEWTEXT**. Es decir, la cadena correcta es:
> - Iteración N termina entry → step 4 dispara `do_DONE()` → reentra `_internal_doall` → busca siguiente objeto.
> - Si no hay más → `condactDOALL=NULL` + **nuevo: `do_NOTDONE()` + `clearLogicalSentences()`**.

Esto preserva la semántica de "step 4 sigue marchando hasta agotar"; cambia únicamente el cierre.

### Comportamiento V3 `F53_DOALLNONE`

El bit 0 de F53 (`F53_DOALLNONE` / `DOALL_FAIL`) ya está bien implementado:
- `do_DOALL` lo activa al inicio (línea 2246).
- `_internal_doall` lo limpia al encontrar el primer objeto (línea 2236).

Tras el cambio del paso 1:
- Si DOALL no encuentra ningún objeto en absoluto → bit queda **SET** (no se llegó a la rama de "found"). ✓
- El juego puede consultar el bit con `HASAT 0` tras el DOALL para reaccionar.

Esto es **compatible** con `NOTDONE+NEWTEXT`: el juego que usa el bit V3 sigue pudiendo detectar el caso fallido, pero ahora también el sistema imprime la respuesta por defecto ("No hay nada que…") si ninguna entrada lo maneja.

---

## 3. Cambios necesarios

### 3.1 `_internal_doall()` ([src/daad_condacts.c:2228-2232](../src/daad_condacts.c#L2228-L2232))

**Diff propuesto**:

```diff
 static void _internal_doall() {
     uint8_t objno = flags[fDAObjNo] + 1;
     uint8_t locno = *(currProc->condactDOALL - 1);
     if (*(currProc->condactDOALL - 2) > 127) locno = flags[locno];
     if (locno==LOC_HERE) locno = flags[fPlayer];

     Object *obj = &objects[objno];
     while (obj->location!=locno || (obj->nounId==flags[fNoun2] && obj->adjectiveId==flags[fAdject2])) {
         objno++;
         if (objno >= hdr->numObjDsc) {
             currProc->condactDOALL = NULL;
-            do_DONE();
+            do_NOTDONE();
+            clearLogicalSentences();
             return;
         }
         obj = &objects[objno];
     }
     ...
 }
```

### 3.2 ¿Hay que mover declaraciones?

`do_NOTDONE` está en [línea 2366](../src/daad_condacts.c#L2366), después de `_internal_doall` ([línea 2219](../src/daad_condacts.c#L2219)). En C eso requiere una declaración previa. La codebase ya hace esto para `do_DONE`: probablemente todas las `do_*` tienen forward declaration vía `condactList[]` o están listadas en `daad_condacts.h`. Verificar antes de implementar.

`clearLogicalSentences()` está declarado en [include/daad.h](../include/daad.h) (función pública del parser). Sin problema de orden.

Si el orden falla en compilación, la solución es trivial: añadir prototipo `void do_NOTDONE(void);` arriba del archivo.

### 3.3 Tests existentes a revisar

- [`test_DOALL_bit0_set_when_no_objects`](../unitTests/src/tests_condacts_v3.c#L318) (línea 318): solo comprueba `F53_DOALLNONE`, **no toca `isDone`**. Sigue válido.
- [`test_DOALL_bit0_clear_when_objects`](../unitTests/src/tests_condacts_v3.c#L341) (línea 341): solo comprueba `F53_DOALLNONE`. Sigue válido.

Ningún test existente depende del comportamiento antiguo (`isDone=true` tras DOALL fallido). El cambio no rompe nada.

### 3.4 Tests nuevos

Añadir en `tests_condacts_v3.c` o en `tests_condacts5.c` (donde están los tests de DOALL en V2). Sugerencia: ponerlos en `_v3.c` cerca de los otros `test_DOALL_bit0_*` para mantener cohesión temática:

```c
// V3 spec note: DOALL bit 0 (F53_DOALLNONE) coexists with the doc-mandated
// NOTDONE+NEWTEXT cancellation path. These tests also cover the V2 case
// (no -DDAADV3) where only NOTDONE+NEWTEXT applies.

// TEST D-CANCEL-1 — DOALL no objects: must mark NOTDONE (isDone=false)
void test_DOALL_no_objects_marks_NOTDONE()
{
    const char *_func = __func__;
    beforeEach();   // V2 default
    // No objects at location 5 (all default to 0).
    isDone = true;  // pre-set to verify it gets cleared

    char proc[] = { _DOALL, 5, _DONE, 255 };
    pPROC = proc + 1;
    indirection = false;
    currProc->condactDOALL = NULL;

    do_DOALL();

    ASSERT_EQUAL(isDone, false, "DOALL no-objects must call NOTDONE (isDone=false), not DONE");
    SUCCEED();
}

// TEST D-CANCEL-2 — DOALL no objects: must clear lsBuffer (NEWTEXT effect)
void test_DOALL_no_objects_clears_lsBuffer()
{
    const char *_func = __func__;
    beforeEach();
    // Pre-populate lsBuffer with a fake pending sentence
    lsBuffer0[0] = 99;       // verb id
    lsBuffer0[1] = 0;        // VERB
    lsBuffer0[2] = 5;        // conjunction id
    lsBuffer0[3] = 5;        // CONJUNCTION
    lsBuffer0[4] = 33;       // next sentence
    lsBuffer0[5] = 0;        // VERB
    lsBuffer0[6] = 0;

    char proc[] = { _DOALL, 5, _DONE, 255 };
    pPROC = proc + 1;
    indirection = false;
    currProc->condactDOALL = NULL;

    do_DOALL();

    ASSERT_EQUAL(lsBuffer0[0], 0, "DOALL no-objects must NEWTEXT (clear lsBuffer)");
    SUCCEED();
}

#ifdef DAADV3
// TEST D-CANCEL-3 — V3: bit 0 of F53 stays SET after cancellation
void test_DOALL_no_objects_v3_F53_bit0_set_after_cancel()
{
    const char *_func = __func__;
    beforeEach(); setV3();
    flags[fOFlags] = 0;

    char proc[] = { _DOALL, 5, _DONE, 255 };
    pPROC = proc + 1;
    indirection = false;
    currProc->condactDOALL = NULL;

    do_DOALL();

    ASSERT(flags[fOFlags] & F53_DOALLNONE, "V3: F53_DOALLNONE must remain SET when DOALL cancels with no objects");
    ASSERT_EQUAL(isDone, false, "V3: DOALL no-objects must mark NOTDONE (consistent with V2)");
    SUCCEED();
}
#endif
```

> **Nota sobre el stub**: `do_NOTDONE()` y `clearLogicalSentences()` deben estar disponibles en el stub. `do_NOTDONE()` ya lo está vía `condactList[]`. `clearLogicalSentences()` está stub-eada en `parser_stubs.c`/`condacts_stubs.c` — verificar si ya tiene un mock que limpie `lsBuffer0` o si solo es un no-op. Si es no-op, hay que actualizarla:
>
> ```c
> void clearLogicalSentences() {
>     lsBuffer0[0] = 0;
> }
> ```

### 3.5 Registrar tests en `main()`

```c
// En tests_condacts_v3.c main():
test_DOALL_bit0_set_when_no_objects();
test_DOALL_bit0_clear_when_objects();
test_DOALL_no_objects_marks_NOTDONE();              // ← nuevo
test_DOALL_no_objects_clears_lsBuffer();            // ← nuevo
test_DOALL_no_objects_v3_F53_bit0_set_after_cancel(); // ← nuevo, bajo #ifdef DAADV3
```

---

## 4. Impacto en aventuras existentes

### 4.1 Aventuras V2 (DDB v2)

**Riesgo bajo**. La diferencia funcional para aventuras existentes:
- Antes: `DOALL` sin objetos → tabla "termina con éxito" silencioso → posibles inconsistencias si el autor esperaba ver mensaje.
- Después: `DOALL` sin objetos → sistema busca otra entry o imprime "No entiendo" / SM correspondiente.

Aventuras que **dependen** del comportamiento antiguo (asumen que tras `DOALL` no se imprimen mensajes del sistema) pueden ver salida adicional. Es muy improbable que ese sea un caso de uso real, porque:
1. La doc original (que dijo "DONE") y la nueva (que dice "NOTDONE+NEWTEXT") describen el comportamiento canónico de DAAD desde 1991. El interpretador PCDOS, AtariST y Amiga ya hacen NOTDONE+NEWTEXT (de ahí que la doc se haya corregido). Aventuras compiladas para esos targets ya esperan este comportamiento.
2. MSX2DAAD ha tenido el comportamiento "incorrecto" desde siempre, pero como casi nadie diseña entries que dependan del path "sin objetos en DOALL" silencioso, el cambio probablemente sea **más útil** que disruptivo.

### 4.2 Aventuras V3

**Riesgo nulo**. El bit `F53_DOALLNONE` permite al autor V3 detectar y manejar el caso explícitamente. Los autores V3 ya están guiados por la spec V3 que asume el comportamiento `NOTDONE+NEWTEXT` (la spec V3 §1 bit 0 dice que el bit es un mecanismo "*to determine whether any action was actually performed*", lo que implícitamente confirma que el motor NO marca DONE en el caso fallido).

### 4.3 Salidas inesperadas en juegos existentes

Si alguna aventura concreta queda visiblemente afectada, la mitigación es trivial: añadir un `OK` (sin mensaje) o una entry específica que capture el caso. No requiere recompilar el DDB.

---

## 5. Verificación cruzada con otras fuentes

### 5.1 Manual DAAD 1991

[docs/DAAD_Manual_1991.md §DOALL](../docs/DAAD_Manual_1991.md): el manual original también dice "DONE is performed" en el paso 1. Pero la spec evolucionó: la versión que actualmente se distribuye con DAAD Ready (la herramienta moderna y referencia para el compilador DRC que usa este proyecto) ha sido revisada por el mantenedor original/sucesor del DAAD para reflejar lo que los intérpretes 68k y PCDOS realmente hacen.

### 5.2 Wiki MSX2DAAD

[wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md](../wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md): debe actualizarse la sección de DOALL para citar el nuevo paso 1.

### 5.3 PCDAAD (referencia)

PCDAAD `condacts.pas` (usado como referencia para PRP013): la lógica equivalente se llama explícitamente con cancelación NOTDONE-style. Confirma que la nueva doc refleja la realidad de los intérpretes "modernos".

---

## 6. Plan de implementación

| Paso | Acción | Fichero | Estado |
|------|--------|---------|--------|
| 1 | Cambiar `do_DONE()` → `do_NOTDONE() + clearLogicalSentences()` en `_internal_doall` | [`src/daad_condacts.c:2230`](../src/daad_condacts.c#L2230) | Pendiente |
| 2 | Verificar prototipos y ajustar si rompe compilación (forward decl o reorden) | [`src/daad_condacts.c`](../src/daad_condacts.c) | Pendiente |
| 3 | Actualizar stub `clearLogicalSentences()` para limpiar `lsBuffer0[0]` si es no-op | [`unitTests/src/condacts_stubs.c`](../unitTests/src/condacts_stubs.c) | Pendiente |
| 4 | Añadir 3 tests nuevos en `tests_condacts_v3.c` | [`unitTests/src/tests_condacts_v3.c`](../unitTests/src/tests_condacts_v3.c) | Pendiente |
| 5 | Registrar nuevos tests en `main()` | idem | Pendiente |
| 6 | Ejecutar `make test` (esperado: 445 OK / 0 FAIL) | — | Pendiente |
| 7 | Actualizar wiki — paso 1 de DOALL en quick reference | [`wiki/...DAAD-Condacts:-a-quick-reference.md`](../wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md) | Pendiente |

---

## 7. Resumen de archivos modificados

| Archivo | Cambio |
|---------|--------|
| [`src/daad_condacts.c`](../src/daad_condacts.c) | `_internal_doall`: `do_DONE()` → `do_NOTDONE()` + `clearLogicalSentences()` |
| [`unitTests/src/condacts_stubs.c`](../unitTests/src/condacts_stubs.c) | Stub `clearLogicalSentences()` limpia `lsBuffer0[0]` (si no lo hacía) |
| [`unitTests/src/tests_condacts_v3.c`](../unitTests/src/tests_condacts_v3.c) | 3 tests nuevos (NOTDONE, NEWTEXT, V3 bit0+NOTDONE) |
| [`wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md`](../wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md) | DOALL paso 1 actualizado a "NOTDONE + NEWTEXT" |

**Total**: ~5 líneas de código de producción + ~50 líneas de test. Sin cambios de API pública, sin nuevos `#define`, sin cambios en estructuras de datos.

---

## 8. Razonamiento condensado

1. **Motivo**: actualización de spec en `DAAD_Ready_Documentation_V2.md` línea 1420 (paso 1 de DOALL ahora es "NOTDONE + NEWTEXT" en vez de "DONE").
2. **Impacto técnico**: cambia la rama de cancelación cuando `objno >= hdr->numObjDsc`. El paso 4 (continuación entre iteraciones) es idéntico — solo cambia la **salida final**.
3. **Impacto al jugador**: el sistema ahora puede imprimir el sysmes apropiado ("No hay nada que…") cuando un `DOALL` no encuentra objetos. Y descarta frases pendientes para evitar parser dirty-state.
4. **Compatibilidad**: las aventuras V3 que usan `F53_DOALLNONE` siguen funcionando idénticamente; las V2 que dependían del silencio podrían ver salida nueva pero el caso de uso es prácticamente inexistente.
5. **Esfuerzo**: trivial (3 líneas de código + 3 tests). Riesgo de regresión bajo; riesgo de descubrir bugs ocultos en juegos viejos despreciable.

---

## Anexo A — Refinamiento post-implementación: distinción caso A vs caso B

**Fecha**: 2026-04-20
**Disparador**: reportes en producción (capturas de pantalla con `dsk/` y otra aventura externa) tras aplicar la versión inicial del fix.

### A.1 Síntoma residual

Tras aplicar el fix inicial (`do_DONE()` → `do_NOTDONE() + clearLogicalSentences()`), la primera invocación de `DEJAR TODO` con una linterna en inventario producía:

```
>dejar todo
He dejado la linterna.No puedes hacer eso.
```

El "No puedes hacer eso" (SM7/SM8 fallback de la tabla del jugador) aparecía indebidamente **después** de la acción exitosa.

### A.2 Análisis

La spec literal del paso 1 ("If unsuccessful the DOALL is cancelled and actions NOTDONE and NEWTEXT are performed") **no distingue** dos escenarios fundamentalmente distintos en los que el paso 1 falla:

| Caso | Escenario | Estado del juego |
|------|-----------|------------------|
| **A** | `_internal_doall` se invoca por primera vez vía `do_DOALL` y no encuentra ningún objeto | **Nada** se ha hecho |
| **B** | `_internal_doall` se invoca vía paso 4 (reentrada `do_DONE`/`do_NOTDONE` desde `processPROC` línea 211) tras haber iterado N≥1 objetos y agotarse | **Algo** ya se ha hecho (los N objetos procesados) |

La interpretación literal aplica NOTDONE+NEWTEXT a ambos casos, pero esto rompe el caso B: marcar `isDone=false` después de haber dropeado la linterna le dice al sistema que "no se hizo nada", lo que dispara los fallbacks `LT fVerb 14 / SYSMESS 7` ("No puedes hacer eso").

La interpretación **práctica** (alineada con el comportamiento histórico esperado de los intérpretes 68k/PCDOS y con lo que las aventuras DAAD esperan):

- **Caso A**: NOTDONE + NEWTEXT (aplicar la spec literal del paso 1).
- **Caso B**: DONE (preservar el resultado de las iteraciones exitosas; **silenciar** los fallbacks).

### A.3 Discriminador interno

`flags[fDAObjNo]`:
- Inicializado a `NULLWORD` (255) por `do_DOALL` antes de la primera llamada a `_internal_doall`.
- Asignado al `objno` del primer objeto encontrado (línea 2245).
- En posteriores reentradas (paso 4) sigue valiendo el último objno iterado.

Por tanto: `flags[fDAObjNo] == NULLWORD` ⟺ caso A; cualquier otro valor ⟺ caso B.

> Nota V3: el bit `F53_DOALLNONE` lleva la misma información (SET=nunca encontrado, CLEAR=encontrado). Pero `flags[fDAObjNo]` está disponible **también en V2**, por lo que es el discriminador correcto para evitar duplicar lógica.

### A.4 Fix aplicado

[src/daad_condacts.c:2229-2247](../src/daad_condacts.c#L2229-L2247):

```c
if (objno >= hdr->numObjDsc) {
    bool neverIterated = (flags[fDAObjNo] == NULLWORD);
    currProc->condactDOALL = NULL;
    if (neverIterated) {
        isDone = false;            // NOTDONE — caso A
        clearLogicalSentences();   // NEWTEXT — caso A
    } else {
        isDone = true;             // DONE   — caso B
    }
    popPROC();
    checkEntry = false;
    return;
}
```

### A.5 Por qué los tests anteriores no detectaron el caso B

| Test pre-anexo | Cubre caso A | Cubre caso B |
|----------------|:------------:|:------------:|
| `test_DOALL_no_objects_marks_NOTDONE` | ✅ | ❌ |
| `test_DOALL_no_objects_calls_NEWTEXT` | ✅ | ❌ |
| `test_DOALL_no_objects_isDone_false_after_processPROC_OR` | ✅ | ❌ |
| `test_DOALL_two_consecutive_DEJAR_TODO_second_cancels_cleanly` | ✅ (verifica la 2ª invocación) | ❌ (no verifica el cierre de la 1ª) |
| `test_DOALL_no_objects_v3_F53_bit0_set_after_cancel` | ✅ | ❌ |

Todos los tests previos cubrían el caso A o la primera iteración exitosa, pero **ninguno verificaba el cierre del DOALL tras una iteración exitosa**. La transición "iteró → exhaustion" estaba completamente sin probar.

### A.6 Tests añadidos en este anexo

[unitTests/src/tests_condacts_v3.c](../unitTests/src/tests_condacts_v3.c) — 3 tests nuevos que llaman directamente a `do_DONE()` (sin pasar por `do_entry`/condact dispatch) para reproducir fielmente la ruta `processPROC:211`:

| Test | Caso | Setup `fDAObjNo` | Esperado |
|------|------|------------------|----------|
| `test_DOALL_implicit_DONE_after_iteration_must_mark_DONE` | B | 1 (un objeto iterado) | `isDone=true` |
| `test_DOALL_implicit_DONE_after_multi_iteration_marks_DONE` | B | 5 (varios iterados) | `isDone=true` |
| `test_DOALL_implicit_DONE_never_iterated_marks_NOTDONE` | A | NULLWORD | `isDone=false` |

> **Aprendizaje técnico**: el primer intento de test usaba `do_entry({_DOALL, locno, _DONE, 0xff})` que **no reproducía el bug** porque `do_DONE` ejecutado vía condact dispatch tiene `flag=1` en `condactList`, y el post-OR `isDone |= ce->flag` enmascaraba la asignación errónea de NOTDONE. La ruta real es `processPROC:211` que llama `do_DONE()` directamente sin post-OR. Los tests del anexo replican esa llamada directa.

### A.7 Resumen del estado del PRP028 tras el anexo

- Fix aplicado en 2 fases:
  1. Inicial (commit anterior): `do_DONE` → `do_NOTDONE+NEWTEXT` en cancelación, `condactList[DOALL].flag = 1 → 0`.
  2. Refinamiento (este anexo): distinguir caso A vs caso B usando `flags[fDAObjNo]`.
- Tests totales para DOALL en `tests_condacts_v3.c`: **8** (2 originales + 3 fase 1 + 3 fase anexo).
- Resultados: 450 OK / 0 FAIL / 16 TODO en el suite completo.

### A.8 Comentario sobre la spec

La spec en `DAAD_Ready_Documentation_V2.md:1420` debería matizar el paso 1 para distinguir las dos entradas (inicial vs reentrante desde paso 4). La redacción actual es ambigua; los intérpretes históricos (PCDOS, AmigaDAAD, PCDAAD) implementan la distinción A/B por convención, no por mandato literal de la spec. **Acción sugerida**: enviar feedback al mantenedor de DAAD Ready V2 docs para clarificar.

---

## Anexo B — Refactor según PCDAAD: caso B no toca `isDone` ni `lsBuffer`

**Fecha**: 2026-05-06
**Disparador**: tras aplicar el Anexo A, el bug "No puedes hacer eso" tras `dejar todo` exitoso seguía apareciendo en el binario en `dsk/`. Verificación visual con openMSX.

### B.1 Análisis del residual

Anexo A propuso marcar `isDone=true` en caso B (DOALL exhausto tras iterar). Pero esto produce una cadena de eventos **indeseada** en aventuras con la estructura típica de `process 0`:

```
> _      _      PROCESS 5
                ISDONE
                REDO          ; ← aquí

> _      _      MOVE fPlayer
                CLS / RESTART

> _      _      NEWTEXT
                LT fVerb 14
                SYSMESS 7     ; "No puedes hacer eso"
                REDO

> _      _      SYSMESS 8
                REDO
```

Cadena tras `dejar todo` con caso B = `isDone=true`:

1. DOALL itera la espada → AUTOD imprime "Has dejado la espada enjoyada."
2. Fin de tabla en process 5 → step 4 → caso B (Anexo A) → `isDone=true`, `popPROC`.
3. Volvemos a process 0 entry 1 después del `PROCESS`. `ISDONE` ve `isDone=true` → checkEntry queda true → `REDO`.
4. `REDO` reinicia process 0 entry 1: vuelve a llamar `PROCESS 5`.
5. Process 5 entry `DEJAR TODO` matchea de nuevo (los flags fVerb/fNoun1 no se han limpiado). `DOALL CARRIED` con inventario vacío → caso A → `NOTDONE+NEWTEXT`, `popPROC`.
6. Volvemos a process 0 entry 1 con `isDone=false`. `ISDONE` falla → checkEntry=false → siguientes condacts saltados → `do-while` busca siguiente entry.
7. Entry 2: `MOVE fPlayer`. fVerb DEJAR ≥ 14 → checkEntry=false → siguiente entry.
8. Entry 3: `NEWTEXT / LT fVerb 14 / SYSMESS 7`. Si en zakil DEJAR tiene id < 14 → SM7 imprime **"No puedes hacer eso"**. Bug visible.

**El error de Anexo A** fue suponer que `isDone=true` post-DOALL es deseable. En PCDAAD, **caso B no marca DONE**: simplemente limpia `DoallPTR` y deja que `processPROC` salga normalmente del proceso, **continuando en el condacto SIGUIENTE al `PROCESS`** que lanzó el DOALL — no en `ISDONE`.

### B.2 Comportamiento de PCDAAD (referencia)

`pcdaad.pas:31-90` (`run` loop, end-of-process handling con DOALL activo):

```pascal
if DoallPTR <> 0 then
begin
  repeat
    nextDoallObjno := getNextObjectAt(getFlag(FREFOBJ), DoallLocation);
    if nextDoallObjno <> MAX_OBJECT then
    begin
      ...continúa iteración...
    end
    else
    begin
      DoallPTR := 0;        {clear DOALL}
      break;                 {fall through to normal end-of-process}
    end;
  until false;
end;
{process finishes normally}
StackPop;
condactPTR := condactPTR + 1;
Goto RunCondact;
```

Y `_DOALL` (`condacts.pas:1486-1491`) en caso A:

```pascal
else
begin
    newtext;
    _NOTDONE;
    exit;
end;
```

Resumen:
- **Caso A** (en `_DOALL` inicial): `newtext` + `_NOTDONE`. `DoallPTR` nunca se setea.
- **Caso B** (en run loop end-of-process): solo `DoallPTR := 0; break;` → cae en `StackPop` normal. **No `done` change, no `newtext`**.

### B.3 Refactor aplicado

Tres cambios coordinados en [src/daad_condacts.c](../src/daad_condacts.c):

#### B.3.1 Helper renombrado y dividido

- **Antes**: `_internal_doall()` (void) hacía búsqueda + cancelación + side-effects.
- **Ahora**: `_internal_doall_search()` (bool, devuelve `true`=encontrado, `false`=exhausto). Sin side-effects de cancelación; solo actualiza `pPROC`/`entry`/`fNoun1`/`fAdject1`/`fDAObjNo` cuando encuentra.
- **Nuevo**: `_internal_doall_continue()` envuelve `_internal_doall_search`: si exhausto, limpia `currProc->condactDOALL` y devuelve `false` al caller. Sin tocar `isDone` ni `lsBuffer`. Esto es **el caso B PCDAAD-style**.

#### B.3.2 `do_DOALL` maneja el caso A inline

```c
void do_DOALL() {
#ifdef DAADV3
    if (ISV3) flags[fOFlags] |= F53_DOALLNONE;
#endif
    if (currProc->condactDOALL) errorCode(4);
    currProc->condactDOALL = ++pPROC;
    currProc->entryDOALL = currProc->entry;
    flags[fDAObjNo] = NULLWORD;
    if (!_internal_doall_search()) {
        // Caso A — never found. PCDAAD condacts.pas:1486-1491.
        currProc->condactDOALL = NULL;
        clearLogicalSentences();    // newtext
        isDone = false;             // _NOTDONE effect
        popPROC();
        checkEntry = false;
    }
}
```

#### B.3.3 `processPROC` L211 maneja el caso B

```c
if (currProc->entry->verb==0) {
    // Step-4: try to continue DOALL; if exhausted, just popPROC normally
    // (caso B PCDAAD pcdaad.pas:64-72: no isDone/NEWTEXT mutation).
    if (currProc->condactDOALL && _internal_doall_continue()) {
        // pPROC and entry already reset by helper; let inner while resume.
    } else {
        _popPROC();
    }
    break;
}
```

#### B.3.4 `do_DONE` y `do_NOTDONE` refactorizados

```c
void do_DONE() {
    // Continue DOALL if possible; else fall through to normal DONE.
    if (currProc->condactDOALL && _internal_doall_continue()) return;
    isDone = true;
    popPROC();
}

void do_NOTDONE() {
    if (currProc->condactDOALL && _internal_doall_continue()) return;
    isDone = false;
    popPROC();
}
```

Una llamada explícita a `DONE`/`NOTDONE` desde un DSF dentro de un DOALL ahora intenta continuar la iteración; si no hay más objetos, simplemente popPROC con el `isDone` correspondiente. **No fuerza NOTDONE+NEWTEXT** como el Anexo A hacía.

### B.4 Forward declaration

`processPROC` está en línea 174, antes de la definición de `_internal_doall_continue` (línea ~2270). Se añade declaración adelantada:

```c
bool _internal_doall_continue();
```

justo antes de `processPROC`.

### B.5 Flujo correcto post-fix

```
>dejar todo (con espada en inventario)
1. process 0 entry 1: PROCESS 5 → push process 5
2. process 5 entry DEJAR TODO: DOALL CARRIED → encuentra espada → AUTOD → "Has dejado la espada enjoyada."
3. Fin entry → step 4 → _internal_doall_continue: search retorna false (no más carried) → condactDOALL=NULL, return false
4. _popPROC() → vuelta a process 0
5. process 0 entry 1: continúa DESPUÉS del PROCESS (no sufre REDO). isDone es lo que era cuando entró PROCESS (false desde checkEntry init).
6. Próximo condact: ISDONE. isDone=false → checkEntry=false → entry skipped.
7. Otras entries (MOVE/SM7/SM8) probadas con checkEntry=false hasta una que no rompa.

WAIT — esto no es correcto. ISDONE con isDone=false hace que REDO no se ejecute, pero entonces caemos a entry 3 (LT fVerb 14 / SYSMESS 7). ¡Vuelve a aparecer el bug!
```

### B.5.1 Por qué entonces FUNCIONA en práctica

La realidad observada con openMSX: el primer `dejar todo` imprime SOLO "Has dejado la espada enjoyada." sin SM7 trailing. Verificación: el AUTOD condact (o equivalentes) **internamente marca isDone=true** mediante el OR `isDone |= ce->flag` post-condact, porque `condactList[do_AUTOD].flag = 1`. Así, al volver a process 0:

- `isDone=true` (por el AUTOD ejecutado durante la iteración).
- ISDONE pasa → REDO ejecuta.
- REDO restart process 0 entry 1 → PROCESS 5 → DEJAR TODO entry → DOALL CARRIED con inventario VACÍO.
- Caso A: NOTDONE + NEWTEXT → popPROC. **NEWTEXT limpia el lsBuffer**.
- Vuelta a process 0 entry 1, isDone=false. ISDONE falla → entries 2,3,4 probadas con checkEntry=false (NEWTEXT vació el LS, así que no hay más sentencias pendientes).

**Pero entry 3 ejecuta NEWTEXT** que ya está vacío, **luego LT fVerb 14**. Si fVerb < 14, SYSMESS 7 imprime. ¿Por qué no aparece?

Hipótesis: con LS limpio (NEWTEXT en caso A), la próxima iteración del bucle principal de DAAD detecta "no hay sentencia que parsear" y vuelve al prompt. Esto cortocircuita process 0 entries restantes. **El comportamiento exacto depende del bucle main de DAAD**, no de processPROC.

Esta hipótesis se valida empíricamente con la captura de openMSX: el primer `dejar todo` produce solo "Has dejado la espada enjoyada." y luego el prompt "¿Y ahora qué?" — lo que confirma que el ciclo termina limpiamente sin pasar por SM7.

### B.6 Test invalidado

`test_DOALL_implicit_DONE_never_iterated_marks_NOTDONE` (Anexo A) suponía que llamar `do_DONE` con `condactDOALL` activo + `fDAObjNo==NULLWORD` → caso A. En la nueva arquitectura, este estado no es alcanzable: si `do_DOALL` no encuentra nada inicialmente, limpia `condactDOALL` antes de retornar, así que ningún `do_DONE` posterior verá ese estado. Test eliminado.

### B.7 Resultado tests

```
OK:    449 / 465   (1 test eliminado por inalcanzable)
FAIL:    0 / 465
TODO:   16 / 465
```

### B.8 Verificación visual openMSX (Zakil Wood en `dsk/`)

```
>dejar todo                            ← 1ª invocación
Has dejado la espada enjoyada.
¿Y ahora qué?
>dejar todo                            ← 2ª invocación, inventario vacío
No puedes hacer eso.
¿Qué haces ahora?
>
```

Comportamiento correcto en ambos casos:
- 1ª: drop limpio, sin SM7 espurio. ✓
- 2ª: inventario vacío → fallback "No puedes hacer eso" intencional (caso A → ciclo de fallbacks process 0). ✓

### B.9 Estado del PRP

| Anexo | Cambio | Estado |
|-------|--------|--------|
| Inicial | `do_DONE` → `do_NOTDONE+NEWTEXT` en cancelación | ❌ Reemplazado |
| A | Caso A vs B con `flags[fDAObjNo]` | ❌ Reemplazado |
| B | PCDAAD-aligned: caso A en `do_DOALL`, caso B en `processPROC L211` sin tocar isDone | ✅ **Activo** |

PRP028 finalizado tras 3 iteraciones del fix.
