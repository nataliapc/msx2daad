# PRP015 — INC-01 + INC-02: Inconsistencia de extAttr en Obj2 y PRONOUN no implementado

- **ID**: PRP015
- **Bug ref**: [REPORT_DAAD_BUGS.md § INC-01](../REPORT_DAAD_BUGS.md) y [§ INC-02](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad/daad_parser_sentences.c](../src/daad/daad_parser_sentences.c)
- **Severidad**: INCONSISTENCY + BUG funcional
  - INC-01: baja (no hay HASAT2 en la spec, pero rompe la simetría con flags 58/59 y con PCDAAD)
  - INC-02: media-alta (los pronombres "IT/THEM/LO/LA" en inglés no funcionan en absoluto)
- **Fecha**: 2026-04-17
- **Estado**: Completado ✅

---

## 1. Resumen

`populateLogicalSentence()` ([src/daad/daad_parser_sentences.c:120-201](../src/daad/daad_parser_sentences.c#L120-L201)) tiene dos defectos independientes que conviene corregir juntos porque ambos afectan al mismo bloque:

1. **INC-01 — Orden de bytes de `extAttr` invertido respecto a `referencedObject`**: para el Objeto 2 (flags 39-40), los bytes se escriben en orden `[extAttr1, extAttr2]`, pero para el Objeto Actual (flags 58-59, escritos por `referencedObject`) van en orden `[extAttr2, extAttr1]`. PCDAAD (`objects.pas:179-180`) confirma que la convención correcta es `flag_par = extAttr2`, `flag_impar = extAttr1`.

2. **INC-02 — Palabras de tipo PRONOUN ignoradas por el parser**: la función no tiene rama para `type==PRONOUN`. Las palabras "IT", "THEM", etc. declaradas en el vocabulario del DDB no producen ningún efecto. Los flags 46 (`fCPNoun`) y 47 (`fCPAdject`), que según el manual DAAD 1991 deben contener el Noun/Adjective de referencia del pronombre, se **borran** en cada llamada a `populateLogicalSentence` (línea 130), lo que imposibilita toda persistencia de pronombre — incluso si un día se añadiera la rama PRONOUN, los flags se limpiarían antes de poder usarse.

---

## 2. Código actual

[src/daad/daad_parser_sentences.c:120-201](../src/daad/daad_parser_sentences.c#L120-L201):

```c
bool populateLogicalSentence()
{
    char *p, type, id, adj;
    bool ret;
    p = lsBuffer0;
    adj = fAdject1;
    ret = false;

    // Clear parser flags
    flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] = flags[fNoun2] = flags[fAdject2] =
        flags[fCPNoun] = flags[fCPAdject] = NULLWORD;   // ← INC-02: fCPNoun/fCPAdject no deben limpiarse aquí
    ...
    while (*p && *(p+1)!=CONJUNCTION) {
        id = *p;
        type = *(p+1);
        if (type==VERB && flags[fVerb]==NULLWORD) { ... }
        else if (type==NOUN && flags[fNoun1]==NULLWORD) { ... }
        else if (type==NOUN && flags[fNoun2]==NULLWORD) { ... }
        else if (type==ADVERB && flags[fAdverb]==NULLWORD) { ... }
        else if (type==PREPOSITION && flags[fPrep]==NULLWORD) { ... }
        else if (type==ADJECTIVE && adj==fAdject1 && flags[fAdject1]==NULLWORD) { ... }
        else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) { ... }
        // ← INC-02: falta la rama type==PRONOUN
        p+=2;
    }

    if (flags[fNoun2]!=NULLWORD) {
        uint8_t obj = getObjectId(flags[fNoun2], flags[fAdject2], LOC_HERE);
        if (obj!=NULLWORD) {
            flags[fO2Num] = obj;
            flags[fO2Loc] = objects[obj].location;
            flags[fO2Con] = objects[obj].attribs.mask.isContainer << 7;
            flags[fO2Att]   = objects[obj].extAttr1;    // ← INC-01: debería ser extAttr2
            flags[fO2Att+1] = objects[obj].extAttr2;    // ← INC-01: debería ser extAttr1
        } else { ... }
    }
    // ← INC-02: falta la actualización de fCPNoun/fCPAdject al final si Noun1 no es propio
    ...
}
```

---

## 3. Especificación de referencia

### 3.1. INC-01 — Orden de extAttr

**DAAD 1991 Manual** (`docs/DAAD_Manual_1991.md:2858`):

> *"Flag 46 holds the current pronoun ('IT' usually) Noun"*
> *"[...] (see also HASAT)"*

**DAAD Ready V2** (`docs/DAAD_Ready_Documentation_V2.md:535`):

> *"The truth is HASAT and HASNAT do not check object attributes, they actually check bits in some flags. It happens that HASAT 0-7 checks bits in flag 59, and 8-15 checks bits in flag 58. As flags 58 and 59 contain a copy of the attributes of the currently referenced, HASAT seems to check the attributes, but it's actually checking the flags."*

Es decir: atributo 0 = bit 0 de flag 59; atributo 7 = bit 7 de flag 59; atributo 8 = bit 0 de flag 58; atributo 15 = bit 7 de flag 58.

El código de `do_HASAT` en [src/daad_condacts.c:600](../src/daad_condacts.c#L600) implementa exactamente esa fórmula:

```c
flagValue = flags[(fCOAtt+1)-(value>>3)] & bit;
// value 0-7:  value>>3 = 0 → flags[fCOAtt+1] = flags[59] → atributos 0-7 (extAttr1)
// value 8-15: value>>3 = 1 → flags[fCOAtt]   = flags[58] → atributos 8-15 (extAttr2)
```

**Por lo tanto**:
- `flag 58 (fCOAtt)   = extAttr2` (byte alto: bits 8-15)
- `flag 59 (fCOAtt+1) = extAttr1` (byte bajo: bits 0-7)

**Verificación en PCDAAD** (`objects.pas:179-180`):

```pascal
setFlag(FREFOBJATTR2, getByte(DDBHeader.objAttributesPos + objno * 2));      // flag 59 = primer byte DDB
setFlag(FREFOBJATTR1, getByte(DDBHeader.objAttributesPos + objno * 2 + 1));  // flag 58 = segundo byte DDB
```

Donde en PCDAAD `FREFOBJATTR1=58`, `FREFOBJATTR2=59`. El efecto neto es idéntico: flag 58 contiene los bits 8-15 y flag 59 los bits 0-7.

**Verificación en `referencedObject`** ([src/daad/daad_objects.c:55-56](../src/daad/daad_objects.c#L55-L56)):

```c
flags[fCOAtt]   = objRef->extAttr2;    // Flag 58 = extAttr2  ✓
flags[fCOAtt+1] = objRef->extAttr1;    // Flag 59 = extAttr1  ✓
```

Este mapeo es correcto y coherente con `do_HASAT`.

**Divergencia en `populateLogicalSentence`** ([src/daad/daad_parser_sentences.c:186-187](../src/daad/daad_parser_sentences.c#L186-L187)):

```c
flags[fO2Att]   = objects[obj].extAttr1;    // Flag 39 — orden contrario a flag 58
flags[fO2Att+1] = objects[obj].extAttr2;    // Flag 40 — orden contrario a flag 59
```

Es exactamente el orden **contrario**. Si un día se añadiera un condact `HASAT2` (o el autor lee flags 39/40 con la fórmula de `do_HASAT`), los bits estarían invertidos.

Ningún manual documenta específicamente el orden de flags 39/40 (son una extensión DRC/DAAD V2 que no existía en DAAD 1), pero la coherencia con 58/59 es la convención natural y la única que PCDAAD aplica.

### 3.2. INC-02 — PRONOUN vocabulary type

**DAAD 1991 Manual** (`docs/DAAD_Manual_1991.md:2722-2725`):

> *"A Pronoun ('IT' usually) can be used to refer to the Noun/Adjective used in the previous Phrase - even if this was a separate input. Nouns with word values less than 50 are Proper Nouns and will not affect the Pronoun."*

**DAAD 1991 Manual** (`docs/DAAD_Manual_1991.md:2858-2859`):

> *"Flag 46 holds the current pronoun ('IT' usually) Noun"*
> *"Flag 47 holds the current pronoun ('IT' usually) Adjective"*

**DAAD Ready V2** (`docs/DAAD_Ready_Documentation_V2.md:1826-1830`):

> *"In English, a Pronoun ('IT' usually) can be used to refer to the Noun/Adjective used in the previous Phrase - even if this was a separate input. Nouns with word values less than 50 are Proper Nouns and will not affect the Pronoun. Spanish implements same feature with pronominal suffixes like -lo, -la, -los, -las."*

**Tres reglas implícitas** en la especificación:

1. **Persistencia cross-input**: "even if this was a separate input" → los flags 46/47 **no** deben limpiarse al empezar un nuevo `populateLogicalSentence`. Deben sobrevivir a cada llamada hasta que se actualicen con un nuevo noun no-propio.

2. **Sustitución**: cuando el input contiene un token de tipo PRONOUN y `fNoun1` sigue sin asignar, el parser debe rellenar `fNoun1 = flags[fCPNoun]` y `fAdject1 = flags[fCPAdject]`.

3. **Actualización**: al terminar de parsear, si `fNoun1` quedó asignado a un noun con `id >= 50` (no propio), actualizar `flags[fCPNoun] = fNoun1` y `flags[fCPAdject] = fAdject1` para futuras referencias.

**Referencia PCDAAD** (`parser.pas:533-600`):

```pascal
{If English, pronouns work independently, if Spanish, pronouns are applied as pronominal suffixes}
else if (not IsSpanish) and (AWordRecord.AType = VOC_PRONOUN) and (not PronounInSentence) then
begin
 PronounInSentence := true;
 if getFlag(FNOUN)=NO_WORD then
 begin
  setFlag(FNOUN, getFlag(FPRONOUN));
  setFlag(FADJECT, getFlag(FPRONOUN_ADJECT));
 end;
end;
...
{Save noun and adject from LS to maybe be used in future orders, unless they are proper names ( < 50 )}
if (getFlag(FNOUN)>=LAST_PROPER_NOUN) and (getFlag(FNOUN)<>NO_WORD) then
begin
 setFlag(FPRONOUN, getFlag(FNOUN));
 setFlag(FPRONOUN_ADJECT, getFlag(FADJECT));
end;
```

Donde `LAST_PROPER_NOUN = 50` (`global.pas`).

### 3.3. Ámbito explícitamente fuera de este PRP

- **Pronombres enclíticos en español** (`-lo`, `-la`, `-los`, `-las`): requieren análisis de la terminación del verbo. PCDAAD los soporta (`parser.pas:546-567`) pero son una extensión lingüística independiente. Se deja para un PRP futuro.
- **Flag `F53_NOPRONOUN`** ([include/daad.h:55](../include/daad.h#L55)): bit 2 de `fOFlags` que limita los pronombres enclíticos a verbos ≤ 239 en DAAD V3. PRP013 ya lo documenta como N/A porque el soporte base de pronombres no existía. Con INC-02 aplicado, sigue siendo N/A para esta implementación (sólo afecta a encíclicos ES).
- **"Previous verb"** (1991 manual: *"If the Verb is omitted then the LS will assume the previously used Verb is required"*): también está sin implementar, pero es un mecanismo independiente del pronombre. Se deja para un PRP futuro.

---

## 4. Análisis detallado

### 4.1. Impacto de INC-01

Dado que no existe un condact `HASAT2` oficial en DAAD, el impacto directo es **bajo**: ningún juego puede comprobar bits específicos de extAttr en Obj2 sin escribir código condact custom.

No obstante:
- Un autor que lea directamente flags 39 o 40 con `LET 100 39` y espere la misma semántica que flag 58 obtendría resultados invertidos.
- Rompe una simetría que debería ser invariante del motor, dificultando futuras extensiones (p. ej. si se añadiera un `HASAT2` DSF-level).
- Es **trivial corregir** (dos swaps) y alinea con PCDAAD, así que vale la pena aprovechar este PRP para cerrarlo.

### 4.2. Impacto de INC-02

**Alto en inglés**: cualquier aventura en inglés que espere pronombres pierde una característica documentada explícitamente en el manual 1991.

Ejemplo de aventura típica ("Aventura Original EN", vocabulario hipotético):
```
Player: GET TORCH
  → fNoun1=100 (TORCH, id≥50)
  → fCPNoun should be updated to 100  (pero no lo es)

Player: EXAMINE IT
  → IT is a PRONOUN in vocab → ignored
  → fNoun1=NULLWORD → EXAMINE fails with SM6 "I didn't understand"
  → Spec says: IT should resolve to TORCH
```

**Bajo en español**: en castellano, los pronombres DAAD se implementan como sufijos encíclicos (`-lo`, `-la`), que NO son palabras de tipo PRONOUN en el vocabulario. El soporte base (flags 46/47 + rama `type==PRONOUN`) es igualmente necesario por si el autor declara pronombres sueltos en el vocabulario, pero no cubre el caso "cógelo" en español (que requiere el análisis del verbo). Este PRP implementa la rama base; los encíclicos quedan fuera de alcance.

### 4.3. Estado actual del flag `fOFlags` bit `F53_NOPRONOUN`

Definido en [include/daad.h:55](../include/daad.h#L55) y documentado en PRP013 § V3-10 como "no aplicable porque los pronombres no están implementados". Tras este PRP, el soporte base existe pero sigue sin depender de ese bit (que sólo afecta a encíclicos ES).

---

## 5. Solución propuesta

### 5.1. INC-01 — Alinear orden de bytes

Cambio en [src/daad/daad_parser_sentences.c:186-187](../src/daad/daad_parser_sentences.c#L186-L187):

```diff
         flags[fO2Num] = obj;
         flags[fO2Loc] = objects[obj].location;
         flags[fO2Con] = objects[obj].attribs.mask.isContainer << 7;
-        flags[fO2Att] = objects[obj].extAttr1;
-        flags[fO2Att+1] = objects[obj].extAttr2;
+        flags[fO2Att]   = objects[obj].extAttr2;    // Consistente con flag 58 (fCOAtt)
+        flags[fO2Att+1] = objects[obj].extAttr1;    // Consistente con flag 59 (fCOAtt+1)
```

La rama `else` (obj no encontrado) no cambia: ya pone ambos flags a 0.

### 5.2. INC-02 — Implementar PRONOUN

Tres cambios en `populateLogicalSentence`:

**Cambio 1**: Quitar `fCPNoun` y `fCPAdject` de la limpieza inicial ([línea 129-130](../src/daad/daad_parser_sentences.c#L129-L130)):

```diff
-    flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] = flags[fNoun2] = flags[fAdject2] =
-        flags[fCPNoun] = flags[fCPAdject] = NULLWORD;
+    flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] = flags[fNoun2] = flags[fAdject2] = NULLWORD;
```

**Cambio 2**: Añadir rama `type==PRONOUN` dentro del while loop, después de ADJ2 y antes de la rama V3 de UNKNOWN_WORD ([línea 171](../src/daad/daad_parser_sentences.c#L171)):

```diff
         } else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) {     // ADJ2
             flags[fAdject2] = id;
             ret = true;
+        } else if (type==PRONOUN) {                                                      // PRONOUN
+            // Replace current noun/adject with the saved pronoun if no noun was given yet
+            if (flags[fNoun1]==NULLWORD && flags[fCPNoun]!=NULLWORD) {
+                flags[fNoun1]   = flags[fCPNoun];
+                flags[fAdject1] = flags[fCPAdject];
+            }
+            ret = true;
         }
 #ifdef DAADV3
           else if (type==UNKNOWN_WORD) {
```

**Cambio 3**: Actualizar fCPNoun/fCPAdject al final si Noun1 es un noun no-propio (id ≥ 50) ([línea 193, tras el bloque if(fNoun2)](../src/daad/daad_parser_sentences.c#L193)):

```diff
     if (flags[fNoun2]!=NULLWORD) {
         ...
     }
+
+    // Save non-proper noun as pronoun reference for future sentences (spec: id >= 50)
+    if (flags[fNoun1]!=NULLWORD && flags[fNoun1]>=50) {
+        flags[fCPNoun]   = flags[fNoun1];
+        flags[fCPAdject] = flags[fAdject1];
+    }
 #ifdef VERBOSE2
     cprintf("VERB:%u NOUN1:%u ...", ...);
 #endif
     nextLogicalSentence();
```

### 5.3. Código resultante (fragmento relevante)

```c
bool populateLogicalSentence()
{
    char *p, type, id, adj;
    bool ret;
    p = lsBuffer0;
    adj = fAdject1;
    ret = false;

    // Clear parser flags (NOTE: fCPNoun/fCPAdject persist across sentences — spec DAAD 1991)
    flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep]
                 = flags[fNoun2] = flags[fAdject2] = NULLWORD;
#ifdef DAADV3
    if (ISV3) flags[fOFlags] &= ~(F53_UNRECWRD | F53_PREPFIRST);
#endif
    while (*p && *(p+1)!=CONJUNCTION) {
        id = *p;
        type = *(p+1);
        if (type==VERB && flags[fVerb]==NULLWORD) {
            flags[fVerb] = id;
            ret = true;
        } else if (type==NOUN && flags[fNoun1]==NULLWORD) {
            if (id<20 && flags[fVerb]==NULLWORD) {
                flags[fVerb] = id;
            } else {
                flags[fNoun1] = id;
            }
            ret = true;
        } else if (type==NOUN && flags[fNoun2]==NULLWORD) {
            flags[fNoun2] = id;
            adj = fAdject2;
            ret = true;
        } else if (type==ADVERB && flags[fAdverb]==NULLWORD) {
            flags[fAdverb] = id;
            ret = true;
        } else if (type==PREPOSITION && flags[fPrep]==NULLWORD) {
            flags[fPrep] = id;
#ifdef DAADV3
            if (ISV3 && flags[fNoun1] == NULLWORD) flags[fOFlags] |= F53_PREPFIRST;
#endif
            ret = true;
        } else if (type==ADJECTIVE && adj==fAdject1 && flags[fAdject1]==NULLWORD) {
            flags[fAdject1] = id;
            ret = true;
        } else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) {
            flags[fAdject2] = id;
            ret = true;
        } else if (type==PRONOUN) {
            if (flags[fNoun1]==NULLWORD && flags[fCPNoun]!=NULLWORD) {
                flags[fNoun1]   = flags[fCPNoun];
                flags[fAdject1] = flags[fCPAdject];
            }
            ret = true;
        }
#ifdef DAADV3
          else if (type==UNKNOWN_WORD) {
            flags[fOFlags] |= F53_UNRECWRD;
        }
#endif
        p+=2;
    }

    if (flags[fNoun2]!=NULLWORD) {
        uint8_t obj = getObjectId(flags[fNoun2], flags[fAdject2], LOC_HERE);
        if (obj!=NULLWORD) {
            flags[fO2Num] = obj;
            flags[fO2Loc] = objects[obj].location;
            flags[fO2Con] = objects[obj].attribs.mask.isContainer << 7;
            flags[fO2Att]   = objects[obj].extAttr2;    // INC-01: alineado con flag 58
            flags[fO2Att+1] = objects[obj].extAttr1;    // INC-01: alineado con flag 59
        } else {
            flags[fO2Num] = LOC_NOTCREATED;
            flags[fO2Loc] = LOC_NOTCREATED;
            flags[fO2Con] = flags[fO2Att] = flags[fO2Att+1] = 0;
        }
    }

    // Save non-proper noun as pronoun reference (spec DAAD 1991: id >= 50)
    if (flags[fNoun1]!=NULLWORD && flags[fNoun1]>=50) {
        flags[fCPNoun]   = flags[fNoun1];
        flags[fAdject1]  = flags[fAdject1];  // (ya lo contiene, pero simétrico)
        flags[fCPAdject] = flags[fAdject1];
    }

    nextLogicalSentence();
    return ret;
}
```

*(Corrección en el borrador: la línea duplicada `flags[fAdject1] = flags[fAdject1]` es un lapsus — se elimina en la implementación final. El bloque correcto es simplemente los dos sets a `fCPNoun`/`fCPAdject`.)*

### 5.4. Verificación de corrección

**INC-01**: tras el cambio, `do_HASAT` sobre el objeto actual y una fórmula equivalente sobre `flags[39..40]` darían el mismo bit para el mismo índice de atributo. Simétrico con flag 58/59.

**INC-02, escenarios**:

1. `initFlags()` ahora pone todos los flags a 0 (PRP005). Tras el init, `flags[fCPNoun]=0`, `flags[fCPAdject]=0`. 
2. Primera orden: `GET TORCH` (TORCH id=100). fNoun1=100, NULLWORD check pasa, id=100>=50 → `flags[fCPNoun]=100`, `flags[fCPAdject]=NULLWORD`.
3. Segunda orden: `EXAMINE IT`. IT es PRONOUN → fNoun1==NULLWORD y fCPNoun=100!=NULLWORD → `fNoun1=100, fAdject1=NULLWORD`. EXAMINE se resuelve correctamente.
4. Orden con noun propio: `TALK WIZARD` (WIZARD id=30<50). fCPNoun/fCPAdject se quedan en su valor previo (100, NULLWORD). "IT" seguirá refiriéndose a TORCH. ✓ (coincide con spec)

### 5.5. Compatibilidad

- **API**: sin cambios externos. Ningún condact cambia de signatura.
- **DDB binario**: sin cambios.
- **Aventuras existentes**:
  - En inglés: funcionalidad nueva "IT/THEM" activada; puede hacer que algunas órdenes antes imposibles ahora funcionen. Cambio positivo.
  - En español: sin cambios visibles (los encíclicos siguen sin soporte; las aventuras típicas no declaran pronombres sueltos en el vocabulario).
- **Binario resultante**: estimado ~30-40 bytes adicionales (una rama nueva en el `while` + una comprobación post-loop).

---

## 6. Alternativas consideradas

### 6.1. Implementar soporte completo de encíclicos españoles

Descartada para este PRP. Requiere:
- Acceso al texto literal del verbo tokenizado (PCDAAD lo tiene; msx2_daad sólo tiene `voc->id` después del matching).
- `LAST_PRONOMINAL_VERB` constante (239 en PCDAAD).
- Lógica de detección `-lo/-la/-los/-las` en la terminación.

Justifica un PRP independiente (PRP016 propuesto) si/cuando se decida dar soporte completo.

### 6.2. No limpiar fCPNoun en `populateLogicalSentence` pero sí en `clearLogicalSentences`

Descartada. `clearLogicalSentences` se llama desde `NEWTEXT` y el inicio de `parser()`. Si se limpiasen ahí, el NEWTEXT borraría el pronombre, lo que contradice "even if this was a separate input". PCDAAD no los toca en ninguna fase — se quedan desde que se actualizan hasta la siguiente.

### 6.3. Mantener el orden invertido en Obj2 y ajustar un hipotético HASAT2

Descartada. Hoy no hay HASAT2 y el orden canónico en flags 58/59 es claro. La inconsistencia sólo causa problemas futuros.

---

## 7. Riesgos

- **Regresión muy baja**: la rama PRONOUN solo se activa cuando una palabra de tipo PRONOUN aparece en el input. Aventuras sin pronombres en vocab no ven ningún cambio.
- **Flags 39/40 ya leídos directamente por alguna aventura**: muy improbable. Si existiera una aventura con `LET flagX 39` que dependa del orden invertido, se rompería. Se asume el riesgo por coherencia con spec y PCDAAD.
- **Binario Z80**: la rama adicional añade ~30-40 bytes. Sin impacto crítico.

---

## 8. Tests unitarios

Se amplía `tests_daad_parser_sentences.c` (el binario `parser.com` ya creado en PRP014) con tests para `populateLogicalSentence`. Esto requiere:

1. **Enlazar `daad_objects.rel`** al binario `parser.com` — `populateLogicalSentence` llama a `getObjectId`.
2. **Quitar el stub de `getObjectId`** de `parser_stubs.c` (lo proporciona `daad_objects.rel`).
3. **Inicializar `objects` y `hdr->numObjDsc`** en los tests para que `getObjectId` encuentre los objetos de prueba.

### 8.1. Tests para INC-01

#### TEST 8 — `test_populateLS_obj2_extAttr_byte_order_matches_CO`

**Given**:
- `objects[3]`: `nounId=100`, `adjectiveId=NULLWORD`, `location=LOC_HERE/playerLoc`, `extAttr1=0xAB`, `extAttr2=0xCD`
- `lsBuffer0 = [1, VERB, 2, NOUN, 4, PREPOSITION, 100, NOUN, 0]` — "VERB NOUN1 PREP NOUN2"
- `flags[fPlayer] = 5`, `objects[3].location = 5`

**When**: `populateLogicalSentence()`

**Then**:
- `flags[fO2Num] == 3`
- `flags[fO2Att] == 0xCD` — extAttr2 en byte alto (consistente con `fCOAtt`)
- `flags[fO2Att+1] == 0xAB` — extAttr1 en byte bajo (consistente con `fCOAtt+1`)

Refleja la convención del manual DRC V2 (flag 58 = atributos 8-15, flag 59 = atributos 0-7).

#### TEST 9 — `test_populateLS_obj2_extAttr_cleared_when_obj_not_found`

**Given**: Noun2 en lsBuffer0 no coincide con ningún objeto.

**When**: `populateLogicalSentence()`

**Then**: `flags[fO2Att] == 0` y `flags[fO2Att+1] == 0` (rama `else` intacta).

Verifica que el cambio de INC-01 no altera el comportamiento de fallback.

### 8.2. Tests para INC-02

#### TEST 10 — `test_populateLS_pronoun_replaces_empty_noun1`

**Given**:
- `flags[fCPNoun] = 100`, `flags[fCPAdject] = 5` (pronombre guardado de una frase anterior)
- `lsBuffer0 = [30, VERB, 99, PRONOUN, 0]` — "EXAMINE IT"

**When**: `populateLogicalSentence()`

**Then**:
- `flags[fVerb] == 30`
- `flags[fNoun1] == 100` (rellenado desde fCPNoun)
- `flags[fAdject1] == 5`

#### TEST 11 — `test_populateLS_pronoun_ignored_if_noun1_already_set`

**Given**:
- `flags[fCPNoun] = 100`, `flags[fCPAdject] = 5`
- `lsBuffer0 = [30, VERB, 99, PRONOUN, 200, NOUN, 0]` — "EXAMINE IT SWORD"

**When**: `populateLogicalSentence()`

**Then**:
- `flags[fNoun1] == 200` — el noun explícito gana
- `fCPNoun` se actualiza después a 200 (porque id=200 >= 50)

#### TEST 12 — `test_populateLS_pronoun_when_cpnoun_is_nullword`

**Given**:
- `flags[fCPNoun] = NULLWORD` (no hay pronombre guardado todavía)
- `lsBuffer0 = [30, VERB, 99, PRONOUN, 0]`

**When**: `populateLogicalSentence()`

**Then**: `flags[fNoun1] == NULLWORD` — no hay sustitución si fCPNoun no está seteado.

#### TEST 13 — `test_populateLS_saves_non_proper_noun_as_pronoun`

**Given**:
- `flags[fCPNoun] = 50`, `flags[fCPAdject] = 0` (valor previo)
- `lsBuffer0 = [30, VERB, 100, NOUN, 7, ADJECTIVE, 0]` — "GET TORCH RED"
  - noun id=100 (≥ 50, NO propio)

**When**: `populateLogicalSentence()`

**Then**:
- `flags[fCPNoun] == 100` (actualizado)
- `flags[fCPAdject] == 7`

#### TEST 14 — `test_populateLS_proper_noun_doesnt_update_pronoun`

**Given**:
- `flags[fCPNoun] = 100`, `flags[fCPAdject] = 7` (pronombre de una frase anterior)
- `lsBuffer0 = [30, VERB, 30, NOUN, 0]` — "TALK WIZARD" (WIZARD id=30, propio)

**When**: `populateLogicalSentence()`

**Then**:
- `flags[fCPNoun] == 100` (sin cambiar — proper noun no actualiza pronoun)
- `flags[fCPAdject] == 7`

#### TEST 15 — `test_populateLS_does_not_clear_cpnoun_on_each_call`

**Given**:
- `flags[fCPNoun] = 100`, `flags[fCPAdject] = 5`
- `lsBuffer0 = [30, VERB, 0]` — sólo verbo, sin noun ni pronoun

**When**: `populateLogicalSentence()`

**Then**:
- `flags[fCPNoun] == 100` — persistencia cross-input
- `flags[fCPAdject] == 5`

Este test es crítico: cubre la regresión donde la limpieza en línea 130 borraba fCPNoun cada llamada.

#### TEST 16 — `test_populateLS_pronoun_boundary_id_49_is_proper`

**Given**: `lsBuffer0 = [30, VERB, 49, NOUN, 0]` (id justo en el límite)

**When**: `populateLogicalSentence()`

**Then**: `flags[fCPNoun]` no se actualiza (id=49 < 50 → proper noun).

Verifica el borde exacto del umbral "id ≥ 50" del manual.

#### TEST 17 — `test_populateLS_pronoun_boundary_id_50_is_non_proper`

**Given**: `lsBuffer0 = [30, VERB, 50, NOUN, 0]`

**When**: `populateLogicalSentence()`

**Then**: `flags[fCPNoun] == 50` (id=50 ≥ 50 → no proper → actualiza pronoun).

Verifica el otro lado del borde.

### 8.3. Actualizaciones de infraestructura

**`parser_stubs.c`**:

```diff
-// getObjectId — called by populateLogicalSentence (in daad_parser_sentences.c)
-uint8_t getObjectId(uint8_t noun, uint8_t adjc, uint16_t location) { return NULLWORD; }
```

**`unitTests/Makefile`** — añadir `daad_objects.rel` al target `parser.com`:

```diff
-$(OBJDIR)parser.com: $(PARSER_BASE_LIBS) $(OBJDIR)daad_parser_sentences.rel $(SRCDIR)tests_daad_parser_sentences.c
+$(OBJDIR)parser.com: $(PARSER_BASE_LIBS) $(OBJDIR)daad_parser_sentences.rel $(OBJDIR)daad_objects.rel $(OBJDIR)daad_getObjectWeight.rel $(SRCDIR)tests_daad_parser_sentences.c
```

`daad_objects.rel` necesita `daad_getObjectWeight.rel` por dependencia transitiva (referencedObject no se llama aquí, pero daad_objects.c incluye ambas funciones compiladas juntas — mismo patrón que `daadobjs.com`).

**Flag `fPlayer`** debe setearse en los tests que usan `LOC_HERE` para la búsqueda de objetos.

### 8.4. Resumen de tests añadidos

| # | Nombre | Cubre |
|---|--------|-------|
| 8 | `extAttr_byte_order_matches_CO` | INC-01: orden correcto |
| 9 | `extAttr_cleared_when_obj_not_found` | INC-01: regresión rama else |
| 10 | `pronoun_replaces_empty_noun1` | INC-02: sustitución |
| 11 | `pronoun_ignored_if_noun1_already_set` | INC-02: noun explícito gana |
| 12 | `pronoun_when_cpnoun_is_nullword` | INC-02: no-op si no hay prev |
| 13 | `saves_non_proper_noun_as_pronoun` | INC-02: update post-parse |
| 14 | `proper_noun_doesnt_update_pronoun` | INC-02: id < 50 es proper |
| 15 | `does_not_clear_cpnoun_on_each_call` | INC-02: persistencia |
| 16 | `boundary_id_49_is_proper` | INC-02: borde inferior |
| 17 | `boundary_id_50_is_non_proper` | INC-02: borde superior |

**Total**: 10 nuevos tests (7 existentes de PRP014 + 10 = 17 tests en `parser.com`).

---

## 9. Plan de implementación

| Paso | Acción | Fichero |
|------|--------|---------|
| 1 | Corregir orden de bytes en `populateLogicalSentence` | [src/daad/daad_parser_sentences.c:186-187](../src/daad/daad_parser_sentences.c#L186-L187) |
| 2 | Eliminar limpieza de `fCPNoun/fCPAdject` del bloque inicial | [src/daad/daad_parser_sentences.c:129-130](../src/daad/daad_parser_sentences.c#L129-L130) |
| 3 | Añadir rama `type==PRONOUN` en el while loop | [src/daad/daad_parser_sentences.c:170](../src/daad/daad_parser_sentences.c#L170) |
| 4 | Añadir bloque de actualización de pronoun al final | [src/daad/daad_parser_sentences.c:193](../src/daad/daad_parser_sentences.c#L193) |
| 5 | Quitar stub de `getObjectId` de `parser_stubs.c` | [unitTests/src/parser_stubs.c](../unitTests/src/parser_stubs.c) |
| 6 | Añadir `daad_objects.rel` (+ `daad_getObjectWeight.rel`) al target `parser.com` del Makefile | [unitTests/Makefile](../unitTests/Makefile) |
| 7 | Añadir 10 tests a `tests_daad_parser_sentences.c` | [unitTests/src/tests_daad_parser_sentences.c](../unitTests/src/tests_daad_parser_sentences.c) |
| 8 | Actualizar `REPORT_DAAD_BUGS.md` — marcar INC-01 ✅ e INC-02 ✅ | [REPORT_DAAD_BUGS.md](../REPORT_DAAD_BUGS.md) |
| 9 | `make clean && make test` — verificar 0 FAIL y los 10 nuevos tests OK | — |

---

## 10. Referencias

| Fuente | Localización |
|--------|-------------|
| Bug report | [REPORT_DAAD_BUGS.md § INC-01](../REPORT_DAAD_BUGS.md), [§ INC-02](../REPORT_DAAD_BUGS.md) |
| Código afectado | [src/daad/daad_parser_sentences.c:120-201](../src/daad/daad_parser_sentences.c#L120-L201) |
| Implementación hermana correcta | [src/daad/daad_objects.c:46-57](../src/daad/daad_objects.c#L46-L57) (`referencedObject`) |
| Condact que consume flags 58/59 | [src/daad_condacts.c:600](../src/daad_condacts.c#L600) (`do_HASAT`) |
| DAAD 1991 — parser (pronombres) | `docs/DAAD_Manual_1991.md:2697-2744` |
| DAAD 1991 — flags 46/47 | `docs/DAAD_Manual_1991.md:2858-2859` |
| DAAD Ready V2 — HASAT/flags 58/59 | `docs/DAAD_Ready_Documentation_V2.md:535` |
| DAAD Ready V2 — pronombres | `docs/DAAD_Ready_Documentation_V2.md:1822-1830` |
| PCDAAD — orden extAttr | https://github.com/Utodev/PCDAAD — `objects.pas:179-180` |
| PCDAAD — lógica pronombres | https://github.com/Utodev/PCDAAD — `parser.pas:533-600` |
| PCDAAD — constante LAST_PROPER_NOUN | https://github.com/Utodev/PCDAAD — `global.pas` (`= 50`) |
| Test infra previa | [PRP014](PRP014_Fix_nextLogicalSentence.md) (binario `parser.com`) |
