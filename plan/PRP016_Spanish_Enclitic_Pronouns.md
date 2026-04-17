# PRP016 — Pronombres enclíticos españoles (`-lo`/`-la`/`-los`/`-las`)

- **ID**: PRP016
- **Bug ref**: Continuación de [REPORT_DAAD_BUGS.md § INC-02](../REPORT_DAAD_BUGS.md) — cerrado por [PRP015](PRP015_Fix_Parser_Inconsistencies.md), deja pendiente este caso explícitamente acotado.
- **Fichero afectado**: [src/daad/daad_parser_sentences.c](../src/daad/daad_parser_sentences.c) (función `parser()`)
- **Severidad**: Feature — carencia funcional para aventuras en español; no afecta a inglés.
- **Fecha**: 2026-04-17
- **Estado**: Completado ✅

---

## 1. Resumen

En español los pronombres tipo "IT" del manual 1991 no aparecen como palabras sueltas, sino como sufijos **enclíticos** adheridos al verbo: `cógelo`, `bébela`, `cómelos`, `ábrelas`. Una vez dicho *"coge la espada"*, el jugador puede seguir con *"cógela y ábrela"* y el intérprete debe resolver `LA` como la espada.

Tras [PRP015](PRP015_Fix_Parser_Inconsistencies.md), `populateLogicalSentence` sabe sustituir `flags[fCPNoun]/[fCPAdject]` cuando aparece un token de tipo `PRONOUN` en `lsBuffer0`. Falta el paso previo: que `parser()` **detecte la terminación enclítica** del verbo y **inyecte** un token `PRONOUN` virtual en el buffer, en la misma pasada en que matchea el verbo. Esta PRP implementa exactamente esa pieza.

El mecanismo debe respetar el bit `F53_NOPRONOUN` de DAAD V3 (ya definido en `include/daad.h:55`), que cuando está activo limita los enclíticos a verbos con `id ≤ 239`.

---

## 2. Código actual

[src/daad/daad_parser_sentences.c:63-83](../src/daad/daad_parser_sentences.c#L63-L83) — dentro de `parser()`, tras matchear un entry de vocabulario:

```c
//Search it in VOCabulary table
voc = (Vocabulary*)hdr->vocPos;
while (voc->word[0]) {
    if (!memcmp(tmpVOC, voc->word, 5)) {
#ifdef DAADV3
        if (ISV3) {
            if (voc->type == VERB)        verbSeen = true;
            if (voc->type == CONJUNCTION) verbSeen = false;
        }
#endif
        *lsBuffer++ = voc->id;
        *lsBuffer++ = voc->type;
        *lsBuffer = 0;
        break;
    }
    voc++;
}
```

No hay ninguna rama que examine la palabra original más allá de los 5 primeros caracteres usados para el matching. Cualquier sufijo `-LO/-LA/-LOS/-LAS` se pierde.

---

## 3. Especificación de referencia

### 3.1. DAAD 1991 Manual (`docs/DAAD_Manual_1991.md:2770-2781`)

Sección **4.4.1 Spanish**:

> *"If a Verb is less than 5 letters you will need to include the lo, la, los and las versions in the vocab. Obviously if it is four letters you only need 'l' ending as a synonym, if it is three letters you need 'lo' and 'la' synonyms and if it is one or two letters you need 'lo','la','los' & 'las'!"*

El manual original de 1991 documenta el **fenómeno** pero lo resuelve pidiéndole al autor que añada sinónimos manualmente en el vocabulario. Ventaja: funciona con la truncación a 5 caracteres del matcher. Desventaja: el autor debe declarar hasta 4 variantes por verbo corto, y el resultado aun así NO activa flag 46/47 automáticamente (son simples sinónimos del verbo, sin referencia pronominal).

Para verbos ≥ 5 letras (el caso más común: "COGER", "DEJAR", "ABRIR", "MIRAR"), la truncación a 5 caracteres ya absorbe el sufijo: *"COGERLO"* → los 5 primeros caracteres son `"COGER"`, que matchea directamente el verbo. Pero el efecto pronominal (que el objeto referenciado sea el último noun no-propio) se pierde.

### 3.2. DAAD Ready V2 (`docs/DAAD_Ready_Documentation_V2.md:1828-1830`)

> *"In English, a Pronoun ('IT' usually) can be used to refer to the Noun/Adjective used in the previous Phrase - even if this was a separate input. Nouns with word values less than 50 are Proper Nouns and will not affect the Pronoun. **Spanish implementes same feature with pronominal suffixes like -lo, -la, -los, -las**."*

Confirma que en español los pronombres se materializan como sufijos; la semántica es la misma que la del `IT` inglés (flags 46/47).

### 3.3. PCDAAD (`parser.pas:546-567`, `ddb.pas:141-143`, implementación de referencia)

```pascal
{If Spanish, check pronominal terminations}
if IsSpanish then
begin
  if (AWordRecord.AType = VOC_VERB) and (not PronounInSentence) then
  if (not LimitEnclicitPronouns) or (AWordRecord.ACode<=LAST_PRONOMINAL_VERB) then
  begin
    j := 0;
    while (j<4) and (not PronounInSentence) do
    begin
      {check if the verb ends with one of the pronominal suffixes}
      if CheckTermination(SPANISH_TERMINATIONS[j], StrToUpper(orderWords[i]))
      then
      begin
            PronounInSentence := true;
            if getFlag(FNOUN)=NO_WORD then
            begin
              setFlag(FNOUN, getFlag(FPRONOUN));
              setFlag(FADJECT, getFlag(FPRONOUN_ADJECT));
            end;
      end;
      j := j +1;
    end;
  end;
end;
```

Con `SPANISH_TERMINATIONS = ['LO', 'LA', 'LOS', 'LAS']` y `LAST_PRONOMINAL_VERB = 239` (constantes en `parser.pas:24` y `global.pas`). La función `CheckTermination` exige que la palabra sea **estrictamente más larga** que la terminación (para que al menos un carácter constituya el verbo real).

`LimitEnclicitPronouns` (que activa la restricción por `LAST_PRONOMINAL_VERB`) se define en `ddb.pas:141-143`:

```pascal
function LimitEnclicitPronouns: boolean;
begin
  LimitEnclicitPronouns := getFlag(FOBJECT_PRINT_FLAGS) and 4 <> 0;
end;
```

Donde `FOBJECT_PRINT_FLAGS = 53`. Es decir: **bit 2 del flag 53**. Esto corresponde exactamente a `F53_NOPRONOUN = 4` en msx2_daad (`include/daad.h:55`).

### 3.4. Input en mayúsculas — prerrequisito confirmado

[src/daad/daad_prompt.c:79](../src/daad/daad_prompt.c#L79) hace `*p++ = toupper(c);` sobre cada carácter del input antes de guardarlo en `tmpMsg`. Por tanto, la detección puede comparar contra literales mayúsculas `"LO"`, `"LA"`, `"LOS"`, `"LAS"` sin case-folding adicional.

---

## 4. Análisis detallado

### 4.1. Flujo completo con enclítico

Este PRP sólo puede detectar el enclítico en una palabra **que ya ha matcheado** en la tabla de vocabulario. La truncación a 5 caracteres del parser es el eje del análisis:

**Verbos ≥ 5 letras (caso mayoritario):** La truncación absorbe el sufijo automáticamente.
- `"COGERLO"` (7 chars) → primeros 5 = `"COGER"` → matchea el verbo → enclítico `"LO"` detectado en la palabra original ✓
- `"COGERLAS"` (8 chars) → primeros 5 = `"COGER"` → matchea → `"LAS"` detectado ✓

**Verbos ≤ 4 letras (requieren sinónimos per manual 1991):** La truncación NO absorbe el sufijo sola, pero cuando el autor añade los sinónimos indicados por el manual, el matching funciona Y esta PRP inyecta PRONOUN:
- `"ABRE"` (4 chars). El manual dice "if it is four letters you only need 'l' ending as a synonym". El autor declara `"ABREL"` (5 chars) como sinónimo de ABRIR en el vocab.
  - `"ABRELA"` (6 chars) → primeros 5 = `"ABREL"` → **matchea el sinónimo** → `"LA"` detectado en wordLen=6 ✓
  - `"ABRELO"` → primeros 5 = `"ABREL"` → matchea → `"LO"` detectado ✓
- `"DAR"` (3 chars). El manual dice "if it is three letters you need 'lo' and 'la' synonyms". El autor declara `"DARLO"` y `"DARLA"` (5 chars c/u).
  - `"DARLO"` (5 chars) → primeros 5 = `"DARLO"` → **matchea el sinónimo** → `"LO"` detectado en wordLen=5 > 2 ✓
  - `"DARLOS"` (6 chars) → primeros 5 = `"DARLO"` → matchea el sinónimo `"DARLO"` → `"OS"` ≠ `"LO"`, `"OS"` ≠ `"LA"`, `"LOS"` matchea en wordEnd-3 ✓
- Verbos ≤ 2 chars: análogo, con los 4 sinónimos que indica el manual.

**Sin sinónimos, verbos ≤ 4 letras fallan el matching antes de que este PRP pueda actuar.** La responsabilidad de declarar sinónimos sigue siendo del autor de la aventura, tal como indica el manual 1991. Con sinónimos declarados, esta PRP cubre automáticamente todos los casos de enclíticos.

### 4.2. Detección de terminación

Tras matchear con éxito un entry de tipo `VERB`, el parser conoce:
- `p` — puntero al inicio de la palabra original en `tmpMsg` (preservado)
- El tamaño truncado a 5, pero **no** el tamaño original

Hay que recorrer desde `p` hasta el separador (`' '`, `'"'`, `'\0'`) para obtener la longitud real del token tecleado. Entonces se compara `memcmp(end-suffLen, suffix, suffLen)` con las 4 terminaciones.

El coste Z80 es modesto: un bucle de detección de fin de palabra (~8 bytes) + cuatro `memcmp` de 2-3 bytes o — más compacto — un descriptor `{suffix, suffLen}` en un array y un bucle. El código resultante ~40-60 bytes; sólo compilado con `-DLANG_ES`.

### 4.3. Inyección del token `PRONOUN`

Tras detectar enclítico, `parser()` escribe un segundo par `[SYNTH_PRONOUN_ID, PRONOUN]` inmediatamente después del par verbo:

```
lsBuffer = [verbId, VERB, SYNTH_PRONOUN_ID, PRONOUN, ...]
```

`populateLogicalSentence` (tras [PRP015](PRP015_Fix_Parser_Inconsistencies.md)) ya procesa la rama `type==PRONOUN`: rellena `fNoun1`/`fAdject1` con `fCPNoun`/`fCPAdject` si no hay noun explícito todavía. No se requiere ningún cambio adicional en esa función.

El `id` del token PRONOUN no es consultado por la rama PRONOUN, pero **no puede ser 0**: `populateLogicalSentence` usa `while (*p && *(p+1)!=CONJUNCTION)` — `*p==0` se interpreta como terminador del buffer y el loop saldría antes de procesar el token. Se define `SYNTH_PRONOUN_ID = 1` (`include/daad.h`) como valor arbitrario no-cero. Ver §5.1 para la constante.

### 4.4. Interacción con `F53_NOPRONOUN` (DAAD V3)

PCDAAD respeta la restricción: si la bandera "limit enclitic pronouns" está activa, sólo se aplica para verbos con `id ≤ LAST_PRONOMINAL_VERB = 239`. En V2 el bit no existe y los enclíticos se aplican a todos los verbos.

En msx2_daad:
```c
#ifdef DAADV3
    bool limitEnclitic = ISV3 && (flags[fOFlags] & F53_NOPRONOUN);
    if (limitEnclitic && voc->id > 239) enclitic_allowed = false;
#endif
```

Este check sólo tiene sentido en V3. En V2 nunca limita.

### 4.5. Límite inferior: longitud de la palabra

PCDAAD (`CheckTermination`):
```pascal
CheckTermination := (Length(Word) > Length(Termination)) and ...
```

Palabra estrictamente más larga que el sufijo. Ej: `"LOS"` (3 chars) NO matchearía (aunque parezca "LOS" completo, no hay verbo). `"SLO"` (palabra 3 chars, sufijo "LO" 2 chars) SÍ matchearía (verbo "S", sufijo "LO" — aunque improbable en la práctica).

Es un chequeo defensivo ante inputs como `"LO"` suelto (que además no matcharía como verbo). Lo replicamos: `wordLen > suffLen`.

### 4.6. Coexistencia con pronombres PRONOUN explícitos

Si el vocabulario declara además palabras PRONOUN explícitas (caso inusual en español), el parser tras PRP015 las procesa. Este PRP no las desactiva: añade DETECCIÓN además de ellas. El efecto neto (rellenar `fNoun1`/`fAdject1` una vez) es idempotente; no hay conflicto porque `populateLogicalSentence` sólo sustituye si `fNoun1==NULLWORD`.

### 4.7. Adjetivos tras verbo enclítico

Ejemplo: *"CÓGELO AZUL"*. Tras el enclítico se rellena `fAdject1` con el adjetivo del pronombre; luego AZUL llega al bucle y encuentra `fAdject1` ocupado. Con `adj==fAdject1` (porque aún no hay Noun2), AZUL se pierde.

PCDAAD tiene el mismo comportamiento porque usa el mismo esquema de slots. En la práctica es aceptable: "CÓGELO AZUL" es agramatical — un pronombre ya es concreto, no admite adjetivo. Se documenta la limitación sin intentar "arreglarla".

### 4.8. Interacciones V3 completas

El código de `parser()` ya tiene dos bloques `#ifdef DAADV3` que deben analizarse en combinación con la nueva funcionalidad:

#### (a) `verbSeen` — sin conflicto

```c
#ifdef DAADV3
    bool verbSeen = false;
    ...
    if (ISV3) {
        if (voc->type == VERB)        verbSeen = true;  // ← se ejecuta ANTES del enclítico
        if (voc->type == CONJUNCTION) verbSeen = false;
    }
#endif
*lsBuffer++ = voc->id;
*lsBuffer++ = voc->type;   // escribe [verbId, VERB]
// [PRP016 escribe aquí [0, PRONOUN] si detecta enclítico]
```

`verbSeen` se pone a `true` **antes** de la inyección del PRONOUN. El token `[0, PRONOUN]` se añade al buffer pero no modifica `verbSeen`. El flujo de la siguiente palabra en el input no se ve afectado: si hay una palabra desconocida después del verbo enclítico, `verbSeen=true` sigue activo → se inserta `UNKNOWN_WORD` correctamente. ✓

#### (b) `UNKNOWN_WORD` y `F53_UNRECWRD` — sin conflicto

```c
if (!voc->word[0]) {          // palabra NO encontrada en vocab
#ifdef DAADV3
    if (ISV3 && verbSeen) {
        *lsBuffer++ = 0;
        *lsBuffer++ = UNKNOWN_WORD;  // type = 7
        *lsBuffer = 0;
    }
#endif
}
```

El token inyectado por este PRP tiene `type = PRONOUN = 6`, **no** `UNKNOWN_WORD = 7`. Por tanto:
- No activa `F53_UNRECWRD` en `populateLogicalSentence` (que comprueba `type==UNKNOWN_WORD`).
- El camino del "not found" no interfiere porque el enclítico solo se inyecta tras un **match exitoso**.

Escenario V3 completo: `"COGERLO CAJA"` donde "CAJA" no está en vocab:

```
lsBuffer0 = [verbId, VERB, 0, PRONOUN, 0, UNKNOWN_WORD, 0]
```

`populateLogicalSentence` en V3:
1. VERB → `fVerb`
2. PRONOUN → `fNoun1 = fCPNoun`
3. UNKNOWN_WORD → `flags[fOFlags] |= F53_UNRECWRD`

Comportamiento correcto: el objeto se resuelve por pronombre Y el flag de palabra desconocida se activa. ✓

#### (c) `F53_NOPRONOUN` (ya cubierto en §4.4)

Bit 2 del flag 53. Verifica con PCDAAD `ddb.pas:143`: `getFlag(FOBJECT_PRINT_FLAGS) and 4 <> 0`. En msx2_daad = `F53_NOPRONOUN = 4`. ✓  
Cuando activo en V3 y `voc->id > 239`, el enclítico no se inyecta.

#### (d) Pattern de `#ifdef` en condición `if` — corrección en §5.2

El diff original usaba `#ifdef DAADV3` dentro de la condición de un `if`, un patrón que es válido en C pero no idiomático y propenso a error con algunos compiladores. §5.2 usa en su lugar un bloque `#ifdef DAADV3` / `#else` duplicando sólo la condición.

---

## 5. Solución propuesta

### 5.1. Constante y arrays

En `include/daad.h` (junto a `NULLWORD`):

```c
#define SYNTH_PRONOUN_ID  1   // Synthetic id for parser-injected PRONOUN tokens (enclitic -lo/-la/-los/-las).
                              // Must be non-zero: lsBuffer scan loop treats id==0 as end-of-buffer.
                              // The actual value is arbitrary; PRONOUN branches never read the id.
```

En `src/daad/daad_parser_sentences.c`, antes de `parser()`:

```c
#ifdef LANG_ES
// Spanish enclitic suffixes — same order as PCDAAD: LO, LA, LOS, LAS
static const char SPANISH_SUFFIXES[4][4] = { "LO", "LA", "LOS", "LAS" };
static const uint8_t SPANISH_SUFFIX_LEN[4] = { 2, 2, 3, 3 };
#endif
```

El orden es fiel a PCDAAD (`parser.pas:24`: `('LO','LA','LOS','LAS')`). No existe overlap entre sufijos (último carácter de `"COGERLOS"` es `"OS"` ≠ `"LO"`; `"COGERLAS"` es `"AS"` ≠ `"LA"`), por lo que el orden no afecta al resultado pero la fidelidad a la referencia es preferible.

### 5.2. Punto exacto de inyección

Modificar el bloque de match exitoso en `parser()` ([daad_parser_sentences.c:66-82](../src/daad/daad_parser_sentences.c#L66-L82)):

```diff
 while (voc->word[0]) {
     if (!memcmp(tmpVOC, voc->word, 5)) {
 #ifdef DAADV3
         if (ISV3) {
             if (voc->type == VERB)        verbSeen = true;
             if (voc->type == CONJUNCTION) verbSeen = false;
         }
 #endif
         *lsBuffer++ = voc->id;
         *lsBuffer++ = voc->type;
         *lsBuffer = 0;
+#ifdef LANG_ES
+        // Enclitic pronoun detection: a verb ending in -LO/-LA/-LOS/-LAS
+        // injects a virtual PRONOUN token (populateLogicalSentence handles it)
+#ifdef DAADV3
+        if (voc->type == VERB && !(ISV3 && (flags[fOFlags] & F53_NOPRONOUN) && voc->id > 239))
+#else
+        if (voc->type == VERB)
+#endif
+        {
+            // Find end of the original typed word (p still points to its start)
+            const char *wordEnd = p;
+            while (*wordEnd!=' ' && *wordEnd!='"' && *wordEnd!='\0') wordEnd++;
+            uint8_t wordLen = (uint8_t)(wordEnd - p);
+            for (uint8_t k=0; k<4; k++) {
+                uint8_t sl = SPANISH_SUFFIX_LEN[k];
+                if (wordLen > sl && !memcmp(wordEnd-sl, SPANISH_SUFFIXES[k], sl)) {
+                    *lsBuffer++ = SYNTH_PRONOUN_ID;
+                    *lsBuffer++ = PRONOUN;
+                    *lsBuffer = 0;
+                    break;
+                }
+            }
+        }
+#endif
         break;
     }
     voc++;
 }
```

### 5.3. Coste estimado (binario Z80)

- Dos arrays constantes: 16 + 4 = 20 bytes en ROM/data.
- Loop de 4 iteraciones con memcmp de 2-3 bytes: ~40-50 bytes de código.
- Variables locales adicionales: sin impacto (reusa registros).
- Condicionalidad: sólo compilado con `-DLANG_ES`. Inglés sin impacto.

Total estimado: **~60-70 bytes adicionales** en builds españolas. Aceptable.

### 5.4. Verificación de la corrección

**Escenario canónico**: *"COGE ESPADA"* seguido de *"CÓGELO"* (o *"COGERLO"*).

1. Primera frase:
   - `tmpMsg = "COGE ESPADA"`
   - parser reconoce COGE=verb (id=X), ESPADA=noun (id≥50)
   - populateLogicalSentence: fVerb=X, fNoun1=ESPADA → `fCPNoun = ESPADA` (PRP015)
2. Segunda frase:
   - `tmpMsg = "COGERLO"` → parser trunca a "COGER" → matchea verbo COGER (asumiendo id≤239 o F53_NOPRONOUN no activo)
   - Enclítico detectado: wordLen=7, `wordEnd-2 = "LO"` → inyecta `[0, PRONOUN]`
   - `lsBuffer0 = [X, VERB, 0, PRONOUN, 0]`
   - populateLogicalSentence: fVerb=X, entra en rama PRONOUN → `fNoun1 = fCPNoun = ESPADA` ✓

**Escenario V3 restringido**: F53_NOPRONOUN activo, verbo id=240.
- La condición `!(limit && id>239)` es falsa → no se inyecta PRONOUN.
- `"COGERLO"` se procesa como verbo suelto sin pronombre.
- Coincide con la semántica documentada de PRP013 § V3-10.

### 5.5. Compatibilidad

- **Aventuras existentes en español**: las que seguían el workaround del manual 1991 (sinónimos manuales) siguen funcionando — los sinónimos se matchean como antes, y además el enclítico detectado inyecta PRONOUN. El efecto adicional es positivo: ahora también actualiza flags 46/47 según el objeto real del juego.
- **Aventuras en inglés**: sin cambios — el código vive bajo `#ifdef LANG_ES`.
- **DDB binario**: sin cambios.
- **Builds híbridos**: los Makefiles producen variantes por idioma. La variante EN ignora el código.

---

## 6. Alternativas consideradas

### 6.1. Mantener sólo el workflow 1991 (sinónimos en vocab)

Descartada. Exige al autor declarar hasta 4 sinónimos por verbo corto, y **nunca** activa `fCPNoun`/`fCPAdject`: los sinónimos son verbos puros, sin referencia al objeto. Implica que el autor también debe gestionar manualmente `LET 46 ...` tras cada acción, lo que contradice el principio de "pronombre automático" del manual.

### 6.2. Detección en `populateLogicalSentence` en vez de en `parser()`

Descartada. En populateLogicalSentence ya no se dispone del texto original — sólo pares `[id, type]`. La terminación debe detectarse antes de perder la palabra.

### 6.3. Aumentar el tamaño del buffer de matching a 7-8 chars

Descartada. Rompería el formato DDB (vocabulario es 7 bytes por entry: 5 word + id + type). Habría que tocar la estructura binaria, el DRC compiler y toda la cadena de herramientas.

### 6.4. Soportar verbos < 5 letras sin sinónimos del autor

Descartada por complejidad. Requeriría:
- Un vocabulario "suavizado" (matching con wildcard en posiciones de terminación).
- Lógica para intentar varios recortes al buscar (quitar el sufijo antes de matchear).

Fuera del alcance razonable de este PRP. Se documenta que los verbos ≤ 4 chars **requieren sinónimos manuales** en el vocabulario (conforme al manual 1991).

### 6.5. Inyectar PRONOUN **antes** del VERB

Descartada. `populateLogicalSentence` procesa por orden; si PRONOUN va primero, rellenaría fNoun1 con fCPNoun antes de asignar el verbo. No afecta al resultado final (el orden de VERB y PRONOUN es conmutativo porque cada uno rellena un slot distinto) pero es más natural verbo-primero. Mantenemos orden textual: la frase literal es *"COGE[PRONOUN]"*.

---

## 7. Riesgos y limitaciones

- **Riesgo de falso positivo**: un verbo legítimo de vocabulario que termine en `-LO/-LA/-LOS/-LAS` (p. ej. un hipotético `"CICLO"` como verbo) dispararía enclítico erróneamente. En la práctica los verbos imperativos españoles no tienen estas terminaciones naturales en forma imperativa de 2ª persona. Riesgo muy bajo.
- **Verbos ≤ 4 letras requieren sinónimos en vocab** (conforme al manual 1991): sin ellos el parser no matchea la palabra enclítica y este PRP no puede actuar. **Con los sinónimos declarados**, los enclíticos funcionan correctamente (ver §4.1). Se documenta esta dependencia en el comentario del código y en `AGENTS.md`/Known bugs.
- **"CÓGELO AZUL"**: el adjetivo tras enclítico se pierde. Ver §4.7.
- **Incremento de binario**: ~60-70 bytes en builds españolas.

---

## 8. Tests unitarios

Este PRP exige tests sobre `parser()`, que hasta ahora no se testeaba directamente. Hay dos requisitos de infra:

### 8.1. Infraestructura nueva

1. **`safeMemoryAllocate`/`safeMemoryDeallocate`** en `parser_stubs.c` actualmente devuelven basura (`{}`). `parser()` los usa para pedir `tmpVOC` (buffer de 5 bytes). Hay que proveer una implementación funcional:
   ```c
   static uint8_t tmpVOC_buf[8];
   void *safeMemoryAllocate() { return tmpVOC_buf; }
   void safeMemoryDeallocate(void *p) {}
   ```

2. **Vocabulario de test** — una tabla estática tipo `Vocabulary` encoded como en el DDB real:
   ```c
   // Each entry: 5 bytes word (encoded 255-char) + id + type
   static const uint8_t TEST_VOCAB[] = {
       // "COGER" id=20 type=VERB(0)
       255-'C', 255-'O', 255-'G', 255-'E', 255-'R',  20, VERB,
       // "ESPAD" id=100 type=NOUN(2)  (noun "ESPADA" truncated)
       255-'E', 255-'S', 255-'P', 255-'A', 255-'D', 100, NOUN,
       // "Y    " id=2 type=CONJUNCTION(5)
       255-'Y', 255-' ', 255-' ', 255-' ', 255-' ',   2, CONJUNCTION,
       // terminator
       0, 0, 0, 0, 0, 0, 0
   };
   ```
   Set via `hdr->vocPos = (uint16_t)TEST_VOCAB;` en `daad_beforeEach()` o dentro del test.

3. **`tmpMsg`** — el buffer ya está allocated en `daad_beforeAll` (100 bytes). Cada test hace `strcpy(tmpMsg, "INPUT");`.

### 8.2. Tests propuestos

Se añaden al fichero `tests_daad_parser_sentences.c` (binario `parser.com`).

#### TEST 18 — `test_parser_enclitic_LO_injects_pronoun`

**Given**:
- Vocab contiene `"COGER"` id=20 (VERB)
- `tmpMsg = "COGERLO"`

**When**: `parser()`

**Then**:
- `lsBuffer0[0] == 20` (id de COGER)
- `lsBuffer0[1] == VERB`
- `lsBuffer0[2] == 0` (id del PRONOUN inyectado)
- `lsBuffer0[3] == PRONOUN`
- `lsBuffer0[4] == 0` (terminador)

#### TEST 19 — `test_parser_enclitic_LA`

Igual que TEST 18 pero `tmpMsg = "COGERLA"`. Verifica que `"LA"` también se detecta.

#### TEST 20 — `test_parser_enclitic_LOS`

`tmpMsg = "COGERLOS"`. Verifica sufijo de 3 letras `"LOS"`.

#### TEST 21 — `test_parser_enclitic_LAS`

`tmpMsg = "COGERLAS"`. Verifica sufijo de 3 letras `"LAS"`.

#### TEST 22 — `test_parser_verb_without_enclitic_no_injection`

**Given**: `tmpMsg = "COGER"` (sin sufijo)

**Then**: `lsBuffer0 = [20, VERB, 0]` (sólo verb, sin PRONOUN extra).

Verifica que no se inyecta falsamente.

#### TEST 23 — `test_parser_noun_ending_in_LO_does_not_inject_pronoun`

**Given**:
- Vocab: `"ESPAD"` id=100 NOUN (sólo para hacer match)
- `tmpMsg = "ESPADO"` (hipotético noun terminado en "O")

**When**: `parser()`

**Then**: `lsBuffer0 = [100, NOUN, 0]` — sin PRONOUN.

La detección sólo se activa en VERB, no en NOUN (aunque terminara en sufijo).

Alternativa con caso más realista: añadir al vocab una palabra NOUN que realmente termine en `-LO` o `-LA` (p. ej. `"PALO "` id=110 NOUN) y comprobar que tecleando `"PALO"` no se inyecta PRONOUN.

#### TEST 24 — `test_parser_enclitic_respects_F53_NOPRONOUN_v3`

**Given**:
- V3 activo (ISV3 = true).
- `flags[fOFlags] |= F53_NOPRONOUN` (bit 2).
- Vocab: verbo con id=240 que se matchea contra la entrada.
- `tmpMsg = "VERBOLO"` (o similar que haga match + sufijo).

**When**: `parser()`

**Then**: **NO** se inyecta PRONOUN (porque id=240 > 239).

#### TEST 25 — `test_parser_enclitic_word_too_short`

**Given**:
- Vocab contiene `"LO   "` id=30 VERB (hipotético).
- `tmpMsg = "LO"`.

**When**: `parser()`

**Then**: `lsBuffer0 = [30, VERB, 0]`. No PRONOUN (word length 2 == suffix length 2, requiere estrictamente mayor).

Verifica el edge case `wordLen > suffLen`.

#### TEST 26 — `test_parser_enclitic_does_not_break_nonverb_matches`

Test de regresión: parsear una frase con VERB + NOUN sin enclíticos ("COGE ESPADA") y verificar que `lsBuffer0 = [20,VERB, 100,NOUN, 0]` sin inyecciones espurias.

#### TEST 27 — `test_parser_LAS_priority_over_LA`

**Given**: `tmpMsg = "TIRARLAS"`  
**When**: `parser()`  
**Then**: se inyecta **un solo** PRONOUN (no dos). Verifica que el `break` del loop de sufijos funciona y que `"LAS"` (3 chars) matchea antes de que se considere `"LA"` (2 chars) — o que si matchea `"LA"` primero, el `break` impide doble inyección.

### 8.3. Resumen

| # | Nombre | Cubre |
|---|--------|-------|
| 18 | `enclitic_LO_injects_pronoun` | Sufijo -LO detectado |
| 19 | `enclitic_LA` | Sufijo -LA detectado |
| 20 | `enclitic_LOS` | Sufijo -LOS (3 chars) detectado |
| 21 | `enclitic_LAS` | Sufijo -LAS (3 chars) detectado |
| 22 | `verb_without_enclitic_no_injection` | Sin sufijo, sin falso positivo |
| 23 | `noun_ending_in_LO_does_not_inject` | Sólo VERB dispara enclítico |
| 24 | `respects_F53_NOPRONOUN_v3` | Restricción V3 honrada |
| 25 | `word_too_short` | `wordLen > suffLen` estricto |
| 26 | `does_not_break_nonverb_matches` | Regresión general |
| 27 | `LAS_priority_over_LA` | Única inyección por palabra |

**Total**: 10 nuevos tests (17 existentes + 10 = 27 en `parser.com`).

### 8.4. Integración test populate + parser

Opcional pero recomendado: un test de extremo a extremo en el mismo fichero:

#### TEST 28 — `test_parser_to_populate_enclitic_flow`

**Given**:
- `flags[fCPNoun] = 77`, `flags[fCPAdject] = 3` (estado de frase anterior)
- Vocab con verbo COGER id=20 VERB
- `tmpMsg = "COGERLO"`

**When**:
```c
parser();
populateLogicalSentence();
```

**Then**:
- `flags[fVerb] == 20`
- `flags[fNoun1] == 77` (rellenado por PRONOUN desde fCPNoun)
- `flags[fAdject1] == 3`

Verifica el flujo completo parser→populate integrado.

---

## 9. Plan de implementación

| Paso | Acción | Fichero |
|------|--------|---------|
| 1 | Añadir arrays constantes `SPANISH_SUFFIXES` y `SPANISH_SUFFIX_LEN` en `parser_sentences.c` bajo `#ifdef LANG_ES` | [src/daad/daad_parser_sentences.c](../src/daad/daad_parser_sentences.c) |
| 2 | Insertar bloque de detección de enclítico en `parser()` tras el match exitoso | [src/daad/daad_parser_sentences.c:76](../src/daad/daad_parser_sentences.c#L76) |
| 3 | Proveer `safeMemoryAllocate`/`safeMemoryDeallocate` funcionales en `parser_stubs.c` | [unitTests/src/parser_stubs.c](../unitTests/src/parser_stubs.c) |
| 4 | Añadir vocabulario de test (array estático encoded) en `tests_daad_parser_sentences.c` | [unitTests/src/tests_daad_parser_sentences.c](../unitTests/src/tests_daad_parser_sentences.c) |
| 5 | Añadir los 10 tests (TEST 18-27) al fichero de tests | ídem |
| 6 | Añadir opcionalmente TEST 28 (integración parser→populate) | ídem |
| 7 | Actualizar `REPORT_DAAD_BUGS.md` — nota sobre soporte enclítico añadido | [REPORT_DAAD_BUGS.md](../REPORT_DAAD_BUGS.md) |
| 8 | Documentar limitación "verbos ≤ 4 chars requieren sinónimos" en `AGENTS.md` sección Known bugs | [AGENTS.md](../AGENTS.md) |
| 9 | `make clean && make test` — verificar 0 FAIL | — |

---

## 10. Referencias

| Fuente | Localización |
|--------|-------------|
| Bug report (continuación) | [REPORT_DAAD_BUGS.md § INC-02](../REPORT_DAAD_BUGS.md) |
| PRP previo (base) | [PRP015_Fix_Parser_Inconsistencies.md](PRP015_Fix_Parser_Inconsistencies.md) (soporte PRONOUN en populateLogicalSentence) |
| DAAD 1991 — parser español | `docs/DAAD_Manual_1991.md:2770-2792` (sección 4.4.1) |
| DAAD Ready V2 — enclíticos | `docs/DAAD_Ready_Documentation_V2.md:1826-1830` |
| Input uppercase | [src/daad/daad_prompt.c:79](../src/daad/daad_prompt.c#L79) |
| PCDAAD — implementación | https://github.com/Utodev/PCDAAD — `parser.pas:546-567` (rama Spanish), `parser.pas:397-400` (CheckTermination) |
| PCDAAD — constantes | `global.pas` — `LAST_PRONOMINAL_VERB=239`, `SPANISH_TERMINATIONS=['LO','LA','LOS','LAS']` (`parser.pas:24`) |
| PCDAAD — LimitEnclicitPronouns | `ddb.pas:141-143` — `getFlag(FOBJECT_PRINT_FLAGS) and 4 <> 0` = bit 2 del flag 53 |
| F53_NOPRONOUN bit | [include/daad.h:55](../include/daad.h#L55), [PRP013 § V3-10](PRP013_DAAD_V3_Support.md) |
| Código afectado | [src/daad/daad_parser_sentences.c:25-111](../src/daad/daad_parser_sentences.c#L25-L111) (`parser()`) |
| Rama PRONOUN aguas abajo | [src/daad/daad_parser_sentences.c:171-178](../src/daad/daad_parser_sentences.c#L171-L178) (`populateLogicalSentence`, tras PRP015) |
| Test infra existente | [PRP014](PRP014_Fix_nextLogicalSentence.md) (`parser.com` binary), [PRP015](PRP015_Fix_Parser_Inconsistencies.md) (enlace con `daad_objects.rel`) |
