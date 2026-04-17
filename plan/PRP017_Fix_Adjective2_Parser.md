# PRP017 — INC-04: `populateLogicalSentence` — Adjective2 se pierde si precede a Noun2

- **ID**: PRP017
- **Bug ref**: [REPORT_DAAD_BUGS.md § INC-04](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad/daad_parser_sentences.c:197-202](../src/daad/daad_parser_sentences.c#L197-L202)
- **Severidad**: INCONSISTENCY — impacto medio; misma solución para inglés y español
- **Fecha**: 2026-04-17
- **Estado**: Completado ✅

---

## 1. Resumen

La variable local `adj` de `populateLogicalSentence` sólo cambia a `fAdject2` en el momento en que se ve Noun2. En inglés, los adjetivos **preceden** al sustantivo (`"GET RED LAMP FROM RED BOX"`), por lo que el segundo adjetivo (`"RED"`) llega al bucle **antes** de que `adj` haya cambiado, y cae en el espacio de `fAdject1` (ya ocupado) sin poder irse a `fAdject2` → se descarta silenciosamente.

PCDAAD (`parser.pas:523-525`) usa la estrategia de "primer slot libre" (`FADJECT` → `FADJECT2`), sin dependencia del orden relativo al sustantivo. Esta misma estrategia funciona igual de bien para español: en español los adjetivos siguen al sustantivo, pero el primer adjetivo visto llena `fAdject1` (vacío) y el segundo `fAdject2` (vacío) — el resultado es idéntico al del código actual. **La solución es única y válida para ambos idiomas sin ningún condicional de idioma**.

---

## 2. Código actual

[src/daad/daad_parser_sentences.c:154-202](../src/daad/daad_parser_sentences.c#L154-L202), fragmento relevante:

```c
bool populateLogicalSentence()
{
    char *p, type, id, adj;          // ← variable adj
    ...
    adj = fAdject1;                  // ← inicialización

    while (*p && *(p+1)!=CONJUNCTION) {
        id = *p; type = *(p+1);
        ...
        } else if (type==NOUN && flags[fNoun2]==NULLWORD) {  // NOUN2
            flags[fNoun2] = id;
            adj = fAdject2;                                  // ← sólo aquí cambia
            ...
        } else if (type==ADJECTIVE && adj==fAdject1 && flags[fAdject1]==NULLWORD) {  // ADJ1
            flags[fAdject1] = id;
        } else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) {  // ADJ2
            flags[fAdject2] = id;
        }
    }
}
```

---

## 3. Especificación de referencia

### 3.1. DAAD 1991 Manual — formato de la frase lógica

`docs/DAAD_Manual_1991.md:2729`:

> *"The Logical Sentence format is as follows:  
> `(Adverb)Verb(Adjective1(Noun1))(preposition)(Adjective2(Noun2))`  
> where bracketed types are optional."*

El spec define explícitamente que **Adjective2 precede a Noun2** en inglés. La implementación actual sólo puede rellenar `fAdject2` si Noun2 ya fue procesado, contradiciendo el orden descrito.

### 3.2. DAAD 1991 Manual — parser español

`docs/DAAD_Manual_1991.md:2782`:

> *"The Spanish Parser deals with NOUNS, PRONOUNS and ADJECTIVES differently to the English. Specifically it assumes that adjectives FOLLOW nouns."*

En español los adjetivos siguen al sustantivo. La implementación actual funciona correctamente para español (el adjetivo llega después de Noun2, cuando `adj` ya es `fAdject2`). El fix "primer slot libre" produce el mismo resultado para español.

### 3.3. PCDAAD — implementación de referencia

`parser.pas:523-525`:

```pascal
if (AWordRecord.AType = VOC_ADJECT) and (getFlag(FADJECT) = NO_WORD) then
    setFlag(FADJECT, AWordRecord.ACode)
else if (AWordRecord.AType = VOC_ADJECT) and (getFlag(FADJECT2) = NO_WORD) then
    setFlag(FADJECT2, AWordRecord.ACode)
```

Sin variable de posición. Sin dependencia del orden de los sustantivos.

---

## 4. Análisis detallado

### 4.1. Traza del fallo (inglés)

Frase: `"GET RED LAMP FROM RED BOX"` — lsBuffer: `[GET,V, RED,A, LAMP,N, FROM,P, RED,A, BOX,N]`

| Token | Procesado | Estado fAdject1 | Estado fAdject2 | adj |
|-------|-----------|-----------------|-----------------|-----|
| GET   | fVerb=GET | NULL | NULL | fAdject1 |
| RED   | ADJ, adj==fAdj1, fAdj1==NULL ✓ → fAdject1=RED | RED | NULL | fAdject1 |
| LAMP  | fNoun1=LAMP | RED | NULL | fAdject1 |
| FROM  | fPrep=FROM | RED | NULL | fAdject1 |
| RED   | ADJ, adj==fAdj1, fAdj1≠NULL ✗ ; adj==fAdj2? **NO** (adj es fAdject1) ✗ → **DESCARTADO** | RED | NULL | fAdject1 |
| BOX   | fNoun2=BOX, adj←fAdject2 | RED | NULL | fAdject2 |

Resultado: `fAdject2=NULLWORD` — BOX aparece como sustantivo sin calificativo. `RED` para BOX se pierde.

### 4.2. Traza del fix (inglés)

| Token | Procesado | fAdject1 | fAdject2 |
|-------|-----------|----------|----------|
| RED   | fAdject1==NULL → fAdject1=RED | RED | NULL |
| LAMP  | fNoun1=LAMP | RED | NULL |
| FROM  | fPrep=FROM | RED | NULL |
| RED   | fAdject1≠NULL; fAdject2==NULL → fAdject2=RED | RED | RED |
| BOX   | fNoun2=BOX | RED | RED |

Resultado: `fAdject1=RED (para LAMP), fAdject2=RED (para BOX)` ✓

### 4.3. Verificación con español

Frase española: `"COGE LAMPARA VERDE DE CAJA ROJA"` — lsBuffer: `[COGE,V, LAMPARA,N, VERDE,A, DE,P, CAJA,N, ROJA,A]`

**Con código actual (funciona):**
- LAMPARA → fNoun1, adj=fAdject1
- VERDE → adj==fAdject1, fAdject1==NULL → fAdject1=VERDE ✓
- CAJA → fNoun2, adj=fAdject2
- ROJA → adj==fAdject2, fAdject2==NULL → fAdject2=ROJA ✓

**Con fix:**
- LAMPARA → fNoun1
- VERDE → fAdject1==NULL → fAdject1=VERDE ✓
- CAJA → fNoun2
- ROJA → fAdject1≠NULL, fAdject2==NULL → fAdject2=ROJA ✓

**Resultado idéntico.** El fix no rompe el parser español.

### 4.4. Variable `adj` — ahora sin uso

Con el fix, la variable `adj` deja de ser necesaria en los dos ramos ADJECTIVE. Las asignaciones `adj = fAdject1` y `adj = fAdject2` pasan a ser código muerto. Se elimina para evitar el warning de SDCC "unused variable".

---

## 5. Solución propuesta

### 5.1. Diff

[src/daad/daad_parser_sentences.c:154, 185-186, 197-202](../src/daad/daad_parser_sentences.c#L154-L202):

```diff
-    char *p, type, id, adj;
+    char *p, type, id;
     ...
-    adj = fAdject1;
     ...
         } else if (type==NOUN && flags[fNoun2]==NULLWORD) {    // NOUN2
             flags[fNoun2] = id;
-            adj = fAdject2;
             ret = true;
         ...
-        } else if (type==ADJECTIVE && adj==fAdject1 && flags[fAdject1]==NULLWORD) {  // ADJ1
+        } else if (type==ADJECTIVE && flags[fAdject1]==NULLWORD) {                   // ADJ1
             flags[fAdject1] = id;
             ret = true;
-        } else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) {  // ADJ2
+        } else if (type==ADJECTIVE && flags[fAdject2]==NULLWORD) {                   // ADJ2
             flags[fAdject2] = id;
             ret = true;
```

### 5.2. Coste Z80

- Se elimina la variable `adj` (1 byte menos en stack).
- Se eliminan 2 asignaciones (1 instrucción cada una).
- Cada ramo ADJECTIVE pierde 1 comparación adicional.
- Ahorro neto estimado: **~8-12 bytes**.

---

## 6. Riesgos y compatibilidad

- **Aventuras en español y en inglés**: el fix produce resultados idénticos a los actuales para las frases que ya funcionaban (§4.3), y corrige las que fallaban. No es necesario ningún `#ifdef LANG_ES` — la lógica "primer slot libre" es correcta y consistente en ambos idiomas.
- **Frases ambiguas** (`"GET LAMP FROM RED BOX"` — adjetivo entre dos sustantivos): `RED` va a `fAdject1` (primer slot libre) en lugar de `fAdject2`. PCDAAD tiene el mismo comportamiento. Es una limitación inherente al parser sin información de dependencia sintáctica; queda documentada, no se intenta resolver.

---

## 7. Tests unitarios

Se añaden al binario `parser.com` (fichero [unitTests/src/tests_daad_parser_sentences.c](../unitTests/src/tests_daad_parser_sentences.c)):

### TEST 29 — `test_populateLS_adj2_captured_before_noun2` (inglés)

**Given**: lsBuffer `[VERB, ADJ1, NOUN1, PREP, ADJ2, NOUN2]` — "GET RED LAMP FROM RED BOX"  
**When**: `populateLogicalSentence()`  
**Then**: `fAdject2 != NULLWORD` — el segundo adjetivo es capturado.

### TEST 30 — `test_populateLS_adj1_and_adj2_both_captured`

**Given**: lsBuffer `[VERB, ADJ1, NOUN1, ADJ2, NOUN2]`  
**When**: `populateLogicalSentence()`  
**Then**: `fAdject1 != NULLWORD` y `fAdject2 != NULLWORD`.

### TEST 31 — `test_populateLS_spanish_adj_after_nouns_unchanged`

Regresión: "COGE LAMPARA VERDE DE CAJA ROJA" — adjetivos SIGUEN a sus sustantivos.  
**Then**: `fAdject1 = VERDE`, `fAdject2 = ROJA` — sin cambio respecto al comportamiento previo.

---

## 8. Plan de implementación

| Paso | Acción | Fichero |
|------|--------|---------|
| 1 | Eliminar `adj` de la declaración de variables | [src/daad/daad_parser_sentences.c:154](../src/daad/daad_parser_sentences.c#L154) |
| 2 | Eliminar asignación `adj = fAdject1` inicial | [src/daad/daad_parser_sentences.c:157](../src/daad/daad_parser_sentences.c#L157) |
| 3 | Eliminar `adj = fAdject2` en rama NOUN2 | [src/daad/daad_parser_sentences.c:186](../src/daad/daad_parser_sentences.c#L186) |
| 4 | Simplificar condición ADJ1: quitar `adj==fAdject1 &&` | [src/daad/daad_parser_sentences.c:197](../src/daad/daad_parser_sentences.c#L197) |
| 5 | Simplificar condición ADJ2: quitar `adj==fAdject2 &&` | [src/daad/daad_parser_sentences.c:200](../src/daad/daad_parser_sentences.c#L200) |
| 6 | Añadir TEST 29-31 a `tests_daad_parser_sentences.c` | [unitTests/src/tests_daad_parser_sentences.c](../unitTests/src/tests_daad_parser_sentences.c) |
| 7 | Marcar INC-04 como resuelto en `REPORT_DAAD_BUGS.md` | [REPORT_DAAD_BUGS.md](../REPORT_DAAD_BUGS.md) |
| 8 | `make test` — verificar 0 FAIL | — |

---

## 9. Referencias

| Fuente | Localización |
|--------|-------------|
| Bug report | [REPORT_DAAD_BUGS.md § INC-04](../REPORT_DAAD_BUGS.md) |
| Código afectado | [src/daad/daad_parser_sentences.c:154-202](../src/daad/daad_parser_sentences.c#L154-L202) |
| DAAD 1991 — formato LS | `docs/DAAD_Manual_1991.md:2729` (`(Adjective2(Noun2))`) |
| DAAD 1991 — parser español | `docs/DAAD_Manual_1991.md:2782` (adjetivos siguen al sustantivo) |
| PCDAAD — adjective handling | https://github.com/Utodev/PCDAAD — `parser.pas:523-525` |
