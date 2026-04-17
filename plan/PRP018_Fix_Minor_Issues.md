# PRP018 — MIN-01..05: Correcciones menores

- **ID**: PRP018
- **Bug ref**: [REPORT_DAAD_BUGS.md § MIN-01..05](../REPORT_DAAD_BUGS.md)
- **Ficheros afectados**: [src/daad/daad_msg.c](../src/daad/daad_msg.c), [src/daad_condacts.c](../src/daad_condacts.c)
- **Severidad**: MINOR — ninguno causa crash o corrupción de estado observable
- **Fecha**: 2026-04-17
- **Estado**: Completado ✅

---

## 1. Resumen de los 5 issues

| ID | Descripción | Acción |
|----|-------------|--------|
| MIN-01 | `strnicmp("una ", tmpMsg, 3)` — compara 3 chars en vez de 4 | **Fix** (1 carácter) |
| MIN-02 | English `@` modifier sin efecto | **Cerrar como no-bug** (ver §3.2) |
| MIN-03 | `#elif LANG_EN` frágil con defines vacíos | **Fix** (`#elif defined`) |
| MIN-04 | `do_COPYFO` escribe location inválida antes de errorCode | **Fix** (reordenar) |
| MIN-05 | `printMsg` / `tmpMsg` frágil bajo reentranda | **Documentar** (no code change) |

---

## 2. Código actual y fixes propuestos

### MIN-01 — `strnicmp` con length 3 en vez de 4

**Fichero**: [src/daad/daad_msg.c:120](../src/daad/daad_msg.c#L120)

```c
if (!strnicmp("una ", tmpMsg, 3))   // ← compara "una" sin el espacio
```

Un objeto llamado `"uno cosa"` matchearía incorrectamente porque `"uno"` y `"una"` comparten los 3 primeros caracteres. La comparación debe incluir el espacio para distinguir la palabra completa.

```diff
-if (!strnicmp("una ", tmpMsg, 3))
+if (!strnicmp("una ", tmpMsg, 4))
```

---

### MIN-02 — English `@` modifier — NO ES BUG

**Fichero**: [src/daad/daad_msg.c:128-135](../src/daad/daad_msg.c#L128-L135)

El report indicaba que ambos modificadores `_` y `@` se comportan idénticamente en inglés (ambos eliminan el artículo). Tras verificar la documentación oficial:

**`docs/DAAD_Ready_Documentation_V2.md:2072`**:

> *"| @ | Same as the underscore, but the article has its first letter uppercased. **Only works for Spanish interpreter.** |"*

El comportamiento actual es **correcto por diseño**. El `@` modifier para inglés es intencionalmente idéntico a `_`. La línea `modif;` en la función es la supresión explícita del warning "unused parameter". **No se requiere ningún cambio de código.** Se actualiza el REPORT para reflejar que es comportamiento esperado.

---

### MIN-03 — `#elif LANG_EN` frágil

**Fichero**: [src/daad/daad_msg.c:128](../src/daad/daad_msg.c#L128)

Si se compila con `-DLANG_EN` (sin `=1`), `LANG_EN` se define como cadena vacía y `#elif LANG_EN` evalúa a `#elif 0`, dejando el branch inglés como código muerto. La forma segura usa `defined()`:

```diff
-#elif LANG_EN
+#elif defined(LANG_EN)
```

El Makefile actual usa `-DLANG_EN=1`, así que no hay regresión. El fix es preventivo.

---

### MIN-04 — `do_COPYFO` escribe location inválida antes de errorCode

**Fichero**: [src/daad_condacts.c:1342-1344](../src/daad_condacts.c#L1342-L1344)

```c
void do_COPYFO()
{
    uint8_t flagValue = flags[getValueOrIndirection()];
    (objects + *pPROC++)->location = flagValue;   // ← escribe 255 ANTES del check
    if (flagValue == 255) errorCode(2);
}
```

Actualmente `errorCode(2)` entra en un bucle infinito, así que el estado corrupto nunca se observa. Pero cualquier refactor futuro de `errorCode` (p. ej. lanzar un error recuperable) convertiría esto en corrupción silenciosa.

Fix: avanzar `pPROC`, comprobar, y sólo entonces escribir la localización:

```diff
 void do_COPYFO()
 {
     uint8_t flagValue = flags[getValueOrIndirection()];
-    (objects + *pPROC++)->location = flagValue;
-    if (flagValue == 255) errorCode(2);
+    uint8_t objno = *pPROC++;
+    if (flagValue == 255) errorCode(2);
+    objects[objno].location = flagValue;
 }
```

La separación del `*pPROC++` también hace el código más legible.

---

### MIN-05 — `printMsg` / `tmpMsg` reentranda — SÓLO DOCUMENTACIÓN

**Fichero**: [src/daad/daad_print.c:88-140](../src/daad/daad_print.c#L88-L140)

`printMsg` escribe en el buffer global `tmpMsg`. La única ruta de reentrada conocida (`checkPrintedLines`, que llama a `printSystemMsg(32)`) ya está protegida en [daad_print.c:183-208](../src/daad/daad_print.c#L183-L208) mediante `checkPrintedLines_inUse` y `safeMemoryAllocate()`.

No hay cambio de código: el sistema funciona correctamente. El fix consiste en **añadir un comentario** que documente explícitamente el modelo de reentrada para futuros cambios, evitando que alguien añada una nueva ruta de reentrada sin tomar precauciones.

```diff
+// REENTRANCY NOTE: printMsg writes to the global tmpMsg buffer.
+// The only known reentrant caller is checkPrintedLines (via printSystemMsg(32)),
+// which is guarded by checkPrintedLines_inUse + safeMemoryAllocate().
+// Any new reentrant call path MUST save/restore tmpMsg the same way.
 void printMsg(char *p, bool print)
```

---

## 3. Justificación de fuentes

### 3.1. MIN-01 — strnicmp 3 vs 4

El manual 1991 ([docs/DAAD_Manual_1991.md](../docs/DAAD_Manual_1991.md)) describe las transformaciones de artículo como cambio de la palabra completa "una" → "la". La cadena a distinguir es `"una "` (con espacio) de `"uno "`, lo que requiere comparar los 4 caracteres. La función hermana para "un " ya usa `strnicmp(..., 3)` correctamente (el artículo masculino tiene sólo 3 chars sin espacio ambiguo, pero el femenino "una" sí tiene un lookalike "uno").

### 3.2. MIN-02 — `@` modifier por diseño en inglés

**`docs/DAAD_Ready_Documentation_V2.md:2072`**:

> *"@ — Same as the underscore, but the article has its first letter uppercased. **Only works for Spanish interpreter.**"*

La documentación oficial de DRC es explícita. No hay nada que corregir en el branch inglés.

### 3.3. MIN-03 — `#defined(LANG_EN)`

Práctica estándar de C: `#elif defined(X)` es siempre más robusto que `#elif X`. El Makefile (`Makefile:CXXFLAGS`) define `-DLANG_ES=1` y `-DLANG_EN=1` con valor explícito, por lo que ambas formas funcionan actualmente. El fix es puramente preventivo.

### 3.4. MIN-04 — COPYFO orden de operaciones

Principio general de programación defensiva: no modificar estado hasta haber validado los parámetros. El comentario original en el código del condact COPYFO dice "*This presents no danger to the operation of PAW*" (histórico), lo que reconoce implícitamente que el riesgo es bajo hoy — pero no mañana.

---

## 4. Tests unitarios

Los 4 fixes de código son triviales y no requieren tests de regresión adicionales:
- MIN-01: cambio de un literal numérico — el comportamiento erróneo requería un nombre de objeto comenzando por "uno " que en la práctica no existe en los juegos actuales.
- MIN-03: cambio de directiva de preprocesador — verificable en compilación.
- MIN-04: el test de `do_COPYFO` con valor válido ya existe en `condact5.com`. El caso flagValue=255 llama a `errorCode(2)` (bucle infinito) que no es testeable con la infra actual (documentado en `AGENTS.md`).
- MIN-05: sólo se añade un comentario.

---

## 5. Plan de implementación

| Paso | Acción | Fichero |
|------|--------|---------|
| 1 | `strnicmp("una ", tmpMsg, 3)` → `4` | [src/daad/daad_msg.c:120](../src/daad/daad_msg.c#L120) |
| 2 | `#elif LANG_EN` → `#elif defined(LANG_EN)` | [src/daad/daad_msg.c:128](../src/daad/daad_msg.c#L128) |
| 3 | Reordenar `do_COPYFO`: extraer objno, check, luego write | [src/daad_condacts.c:1342-1344](../src/daad_condacts.c#L1342-L1344) |
| 4 | Añadir comentario de reentranda en `printMsg` | [src/daad/daad_print.c:88](../src/daad/daad_print.c#L88) |
| 5 | Actualizar `REPORT_DAAD_BUGS.md` — marcar MIN-01 ✅, MIN-02 (by design), MIN-03 ✅, MIN-04 ✅, MIN-05 (documented) | [REPORT_DAAD_BUGS.md](../REPORT_DAAD_BUGS.md) |
| 6 | `make test` — verificar 0 FAIL | — |

---

## 6. Referencias

| Fuente | Localización |
|--------|-------------|
| Bug report | [REPORT_DAAD_BUGS.md § MIN-01..05](../REPORT_DAAD_BUGS.md) |
| MIN-02 diseño confirmado | `docs/DAAD_Ready_Documentation_V2.md:2072` (`@` only works for Spanish) |
| Código afectado MIN-01/02/03 | [src/daad/daad_msg.c:106-137](../src/daad/daad_msg.c#L106-L137) |
| Código afectado MIN-04 | [src/daad_condacts.c:1339-1346](../src/daad_condacts.c#L1339-L1346) |
| Código afectado MIN-05 | [src/daad/daad_print.c:88-140](../src/daad/daad_print.c#L88-L140) |
| DAAD 1991 — artículos ES | `docs/DAAD_Manual_1991.md` (transformaciones de artículo) |
