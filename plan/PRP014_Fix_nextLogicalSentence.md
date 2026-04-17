# PRP014 — BUG-10: `nextLogicalSentence` — búsqueda incorrecta de conjunción y lectura fuera de buffer

- **ID**: PRP014
- **Bug ref**: [REPORT_DAAD_BUGS.md § BUG-10](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad/daad_parser_sentences.c:268-278](../src/daad/daad_parser_sentences.c#L268-L278)
- **Severidad**: BUG crítico — afecta a todas las frases con conjunción y a frases sin ella
- **Fecha**: 2026-04-17
- **Estado**: Completado ✅

---

## 1. Resumen

`nextLogicalSentence()` tiene **dos defectos combinados** que hacen que el avance del buffer de frases lógicas sea incorrecto en todos los casos:

1. **Búsqueda de conjunción en el byte incorrecto** — la búsqueda comprueba el byte de *id* en lugar del byte de *tipo*. Las conjunciones reales tienen `id=2` (valor asignado por el autor), no `id=5` (valor del enum `CONJUNCTION`). El while loop nunca encuentra la conjunción y avanza siempre hasta el terminador nulo.

2. **Avance sin guardia de fin de buffer** — cuando no hay más frases (buffer agotado o terminador nulo alcanzado), `p+=2` avanza 2 bytes más allá del terminador, y el bucle `for` copia memoria arbitraria hacia el inicio de `lsBuffer0`, corrompiendo el estado del parser.

**Resultado**: cualquier entrada con conjunción ("COGE LA ESPADA Y VE AL SUR") produce lsBuffer0 corrupto. Las entradas de frase única también dejan lsBuffer0 corrupto (con basura de memoria) en lugar de dejarlo vacío.

---

## 2. Código actual

[src/daad/daad_parser_sentences.c:259-278](../src/daad/daad_parser_sentences.c#L259-L278):

```c
void nextLogicalSentence()
{
    char *p, *c;
    p = lsBuffer0;
    c = lsBuffer0;

    while (*p!=CONJUNCTION && *p!=0) p+=2;  // BUG-A: comprueba id-byte, no type-byte
    p+=2;                                    // BUG-B: avanza sin comprobar *p==0
    for (;;) {
        *c++ = *p;
        *c++ = *(p+1);
        if (!*p) break;
        p+=2;
    }
    *c++ = 0;
    *c = 0;
}
```

---

## 3. Especificación de referencia

### 3.1. Manual DAAD 1991 (fuente primaria)

`docs/DAAD_Manual_1991.md:2730` y `2737`:

> *"Phrases are separated by conjugations ('AND' & 'THEN' usually) and by any punctuation."*
> *"GET ALL. OPEN THE DOOR AND GO SOUTH THEN GET THE BUCKET AND LOOK IN IT."*
> *"which will become five LS's: GET ALL / OPEN DOOR / SOUTH / GET BUCKET / LOOK BUCKET (from IT) IN"*

La especificación establece que el intérprete DEBE ser capaz de separar múltiples frases lógicas en una misma entrada. `nextLogicalSentence` es la única función encargada de este avance.

### 3.2. Formato del buffer de frases lógicas

`parser()` en [src/daad/daad_parser_sentences.c:74-75](../src/daad/daad_parser_sentences.c#L74-L75) almacena:

```c
*lsBuffer++ = voc->id;    // byte par   [0, 2, 4, ...]: id de la palabra
*lsBuffer++ = voc->type;  // byte impar [1, 3, 5, ...]: tipo (VOC_TYPE)
```

La estructura `Vocabulary` ([include/daad.h:189-193](../include/daad.h#L189-L193)):

```c
typedef struct {
    uint8_t word[5];
    uint8_t id;    // byte 5 del DDB: código de la palabra (0-254)
    uint8_t type;  // byte 6 del DDB: tipo (VERB=0...CONJUNCTION=5...PRONOUN=6)
} Vocabulary;
```

El buffer tiene pares `[id, type]`. Para detectar una conjunción hay que comprobar el byte **impar** (`type`), no el par (`id`).

### 3.3. Valores reales de conjunciones en DDB

Verificado en `img/zakil/drc/zakil_xmes_ES.dsf` (fuente DSF compilada a DDB):

```dsf
Y       2       conjugation
ENTONCE 2       conjugation
LUEGO   2       conjugation
```

Confirmado parseando el binario `img/zakil/drc/zakil_xmes_ES.DDB`:
```
'Y'      id=2  type=CONJUNCTION(5)
'ENTON'  id=2  type=CONJUNCTION(5)
'LUEGO'  id=2  type=CONJUNCTION(5)
```

Las conjunciones tienen `id=2`, NO `id=5`. El while loop nunca se detiene en una conjunción.

### 3.4. Función simétrica `populateLogicalSentence`

[src/daad/daad_parser_sentences.c:137](../src/daad/daad_parser_sentences.c#L137) comprueba correctamente el byte de tipo:

```c
while (*p && *(p+1)!=CONJUNCTION) {   // *(p+1) = type byte ✓
    id = *p;
    type = *(p+1);
```

`nextLogicalSentence` debería usar la misma convención.

---

## 4. Análisis detallado

### 4.1. Escenario A: entrada con conjunción

Entrada del jugador: `"COGE ESPADA Y VE SUR"`

`parser()` produce en `lsBuffer0` (suponiendo ids típicos de vocab):

```
pos: 0  1   2   3   4   5        6  7   8   9   10
val: 20 0   5   2   2   5(CONJ)  15 0   3   2   0
     ^VERB  ^NOUN   ^CONJ id=2,type=5   ^VERB ^NOUN ^FIN
```

Ejecución de `nextLogicalSentence` con BUG:
- `p=0`: `*p=20` ≠ CONJUNCTION(5), ≠ 0 → `p+=2`
- `p=2`: `*p=5` == CONJUNCTION(5) → **STOP prematuramente en NOUN (noun con id=5)**
- `p+=2` → `p=4`: apunta al par `[2, CONJUNCTION]` (la conjunción real)
- El for loop copia desde la posición 4 en adelante → frase resultado: `[2, CONJUNCTION, 15, VERB, 3, NOUN, 0]`
- La segunda frase procesada incluye el marcador de conjunción al principio → falla al parsearla

Si el noun no tiene `id=5` (más frecuente), el loop continúa hasta `*p==0`:
- El loop alcanza el terminador nulo
- `p+=2` pasa 2 bytes más allá del nulo
- El for loop copia memoria arbitraria hasta que "por azar" encuentra un byte 0

### 4.2. Escenario B: entrada sin conjunción (frase única)

Entrada: `"COGE ESPADA"`

`lsBuffer0 = [20, VERB, 5, NOUN, 0, ...]`

- Loop: `p=0` → `*p=20` ≠ 5, ≠ 0 → `p+=2`
- `p=2`: `*p=5` == CONJUNCTION(5) → STOP (falso positivo: NOUN con id=5)

Si NOUN no tiene id=5:
- `p=2`: `*p=5` pasamos, `p=4`: `*p=0` → STOP correctamente en el nulo
- `p+=2` → pasa 2 bytes después del nulo → BUG-B
- el for loop copia basura

**El único caso en que ambos bugs "se cancelan"** sería que hubiera exactamente un noun con `id=5` justo antes del terminador, y su contenido coincidiese con lo esperado. Esto es coincidencia, no corrección.

### 4.3. Impacto real

- Toda entrada multi-frase (con "Y", "LUEGO", "ENTONCES", "AND", "THEN") queda procesada incorrectamente. La segunda frase recibe datos erróneos.
- Toda entrada de frase única deja `lsBuffer0` con basura en las posiciones [0..3] si el siguiente ciclo de juego accede al buffer antes de un nuevo parse.
- **populateLogicalSentence** llama a **nextLogicalSentence** incondicionalmente al final, por lo que el bug se dispara en **cada ciclo de input**.

---

## 5. Solución propuesta

### 5.1. Corrección mínima y correcta

Dos cambios en [src/daad/daad_parser_sentences.c:268-270](../src/daad/daad_parser_sentences.c#L268-L270):

**BUG-A**: cambiar la comprobación de `*p` (id byte) a `*(p+1)` (type byte):

```diff
-    while (*p!=CONJUNCTION && *p!=0) p+=2;
+    while (*p!=0 && *(p+1)!=CONJUNCTION) p+=2;
```

Nota: el orden de las condiciones también se invierte — se comprueba primero `*p!=0` para no leer `*(p+1)` cuando `*p==0` (posible acceso fuera de bounds si el buffer está en el último byte asignable).

**BUG-B**: añadir guardia de fin de buffer tras el while:

```diff
+    if (!*p) { *c++ = 0; *c = 0; return; }   // no hay más frases
     p+=2;
```

### 5.2. Código resultante completo

```c
void nextLogicalSentence()
{
#ifdef VERBOSE2
cputs("nextLogicalSentence()\n");
#endif
    char *p, *c;
    p = lsBuffer0;
    c = lsBuffer0;

    while (*p!=0 && *(p+1)!=CONJUNCTION) p+=2;
    if (!*p) { *c++ = 0; *c = 0; return; }
    p+=2;
    for (;;) {
        *c++ = *p;
        *c++ = *(p+1);
        if (!*p) break;
        p+=2;
    }
    *c++ = 0;
    *c = 0;
}
```

### 5.3. Verificación de la corrección

Con la corrección, para `"COGE ESPADA Y VE SUR"`:

```
lsBuffer0 = [20, VERB, 5, NOUN, 2, CONJUNCTION, 15, VERB, 3, NOUN, 0]
```

Ejecución de nextLogicalSentence:
- `p=0`: `*p=20` ≠ 0, `*(p+1)=VERB` ≠ CONJUNCTION → `p+=2`
- `p=2`: `*p=5` ≠ 0, `*(p+1)=NOUN` ≠ CONJUNCTION → `p+=2`
- `p=4`: `*p=2` ≠ 0, `*(p+1)=CONJUNCTION` == CONJUNCTION → **STOP** ✓
- `*p=2` ≠ 0 → no early return
- `p+=2` → `p=6` (inicio de segunda frase: `[15, VERB, 3, NOUN, 0]`)
- El for loop copia correctamente → `lsBuffer0 = [15, VERB, 3, NOUN, 0]` ✓

Para frase única `"COGE ESPADA"`:

```
lsBuffer0 = [20, VERB, 5, NOUN, 0]
```

- `p=0`: `*p=20` ≠ 0, `*(p+1)=VERB` ≠ CONJUNCTION → `p+=2`
- `p=2`: `*p=5` ≠ 0, `*(p+1)=NOUN` ≠ CONJUNCTION → `p+=2`
- `p=4`: `*p=0` → condición `*p!=0` falla → STOP ✓
- `!*p` → early return, `lsBuffer0 = [0, 0]` ✓

### 5.4. Compatibilidad con el resto del parser

- `populateLogicalSentence` comprueba `*(p+1)!=CONJUNCTION` para iterar la frase. Ahora ambas funciones son consistentes en el uso del byte de tipo.
- `clearLogicalSentences` no se ve afectado.
- No hay cambios de API, de formato DDB ni de semántica externa.

---

## 6. Alternativas consideradas

### 6.1. Reparar sólo el BUG-B (guardia de nulo) manteniendo la búsqueda incorrecta

```c
while (*p!=CONJUNCTION && *p!=0) p+=2;
if (!*p) { *c++ = 0; *c = 0; return; }   // añadir sólo esto
p+=2;
```

**Descartada**: corrige el crash por lectura fuera de bounds, pero la segunda frase sigue siendo inaccesible porque el loop nunca encontrará conjunciones reales (id=2≠5). Multi-sentence quedaría roto.

### 6.2. Cambiar sólo BUG-A (tipo de byte) sin guardia de nulo

```c
while (*p!=0 && *(p+1)!=CONJUNCTION) p+=2;
p+=2;  // si *p==0, p+=2 sigue leyendo fuera
```

**Descartada**: funciona para multi-sentence pero el BUG-B permanece para frases sin conjunción. Aplicar ambas correcciones es la solución correcta y completa.

---

## 7. Riesgos y compatibilidad

- **Cambio de comportamiento observable**: Los juegos que probaran conjunciones verían un comportamiento mejorado. No hay riesgo de regresión en aventuras que NO usan conjunciones (el early return para el buffer vacío es seguro).
- **Juegos existentes**: ZakilWood, Aventura Original, etc., ya probados en la plataforma. Con la corrección las entradas multi-frase funcionarán por primera vez.
- **DDB format**: sin cambios.
- **PCDAAD**: no tiene una función equivalente a `nextLogicalSentence` (su parser trabaja por frase completa). La corrección es independiente y no genera divergencia de comportamiento.

---

## 8. Tests unitarios

### 8.1. Nuevo binario: `parser.com`

**Problema de infraestructura**: `daad_stubs.c` ya stub-iza `nextLogicalSentence()` como función vacía. No puede enlazarse junto a `daad_parser_sentences.rel` (definición duplicada).

**Solución**: crear un stub dedicado `unitTests/src/parser_stubs.c` que contenga todas las dependencias necesarias para linkear `daad_parser_sentences.rel` **excepto** las funciones del propio módulo (`nextLogicalSentence`, `populateLogicalSentence`, `parser`, etc.).

`nextLogicalSentence` no llama a ninguna función externa (sólo opera sobre `lsBuffer0`), así que el stub mínimo es muy reducido.

También es necesario exponer `lsBuffer0` al test. Se añadirá una declaración `extern` en [include/daad.h](../include/daad.h) o directamente en el fichero de test.

**Nueva entrada en `unitTests/Makefile`**:

```makefile
DAAD_PROGRAMS = objwght.com daadobjs.com gettoken.com initobj.com parser.com

$(OBJDIR)parser.com: $(addprefix $(OBJDIR), crt0msx_msxdos_advanced.rel heap.rel assert.rel parser_stubs.rel daad_parser_sentences.rel) \
                     $(addprefix $(LIBDIR), $(LIBS)) \
                     $(SRCDIR)tests_daad_parser_sentences.c
    @echo "######## Compiling $@"
    @$(DIR_GUARD)
    @$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $^ -o $(subst .com,.ihx,$@) ;
    @$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
    @echo "**** Copying .COM files to DSK/"
    @cp $@ dsk/
```

### 8.2. Fichero de tests: `unitTests/src/tests_daad_parser_sentences.c`

Se proponen los siguientes 7 tests (todos de caja negra sobre `lsBuffer0`):

---

#### TEST 1 — `test_nextLS_empty_buffer`

**Given**: `lsBuffer0 = [0, 0]` (buffer vacío)  
**When**: `nextLogicalSentence()`  
**Then**: `lsBuffer0[0] == 0` (sigue vacío, no crash)

Verifica que la función se comporta correctamente con buffer ya vacío (segunda llamada tras procesar la última frase).

---

#### TEST 2 — `test_nextLS_single_sentence_no_conjunction`

**Given**: `lsBuffer0 = [20, VERB, 5, NOUN, 0]` — una sola frase  
**When**: `nextLogicalSentence()`  
**Then**: `lsBuffer0[0] == 0` — buffer queda vacío (no hay más frases)

Verifica BUG-B: sin la guardia, `p+=2` avanzaría más allá del nulo y copiaría basura.

---

#### TEST 3 — `test_nextLS_single_sentence_conjunction_id_not_5`

**Given**: `lsBuffer0 = [20, VERB, 2, NOUN, 0]` — noun con id=2 (NO confundible con CONJUNCTION tras el fix)  
**When**: `nextLogicalSentence()`  
**Then**: `lsBuffer0[0] == 0` — buffer vacío correctamente

Verifica que `id=2` (valor real de conjunciones en DDB) no se confunde con una conjunción una vez se usa `*(p+1)`.

---

#### TEST 4 — `test_nextLS_two_sentences_advances_correctly`

**Given**:  
```c
lsBuffer0 = [20, VERB, 5, NOUN,    // frase 1
              2, CONJUNCTION,        // conjunción (id=2, type=CONJUNCTION)
             15, VERB, 3, NOUN, 0]  // frase 2
```
**When**: `nextLogicalSentence()`  
**Then**:  
- `lsBuffer0[0] == 15` (VERB de frase 2)
- `lsBuffer0[1] == VERB`
- `lsBuffer0[2] == 3` (NOUN de frase 2)
- `lsBuffer0[3] == NOUN`
- `lsBuffer0[4] == 0` (terminador)

Verifica el caso principal de BUG-A: con `id=2` para la conjunción, el fix hace el avance correcto.

---

#### TEST 5 — `test_nextLS_three_sentences_advances_to_second`

**Given**: tres frases separadas por dos conjunciones  
```c
lsBuffer0 = [20, VERB, 2, CONJUNCTION,
             15, VERB, 2, CONJUNCTION,
             10, VERB, 0]
```
**When**: `nextLogicalSentence()`  
**Then**:  
- `lsBuffer0[0] == 15` (inicio de frase 2, no frase 3)
- `lsBuffer0[2] == 2` (conjunción, inicio de frase 3 aún presente)
- El buffer desde pos 0 contiene la segunda frase con su conjunción encadenada

Verifica que el avance es de UNA frase, no de todas.

---

#### TEST 6 — `test_nextLS_second_sentence_is_empty_after_conjunction`

**Given**: conjunción al final sin frase siguiente  
```c
lsBuffer0 = [20, VERB, 2, CONJUNCTION, 0]
```
**When**: `nextLogicalSentence()`  
**Then**: `lsBuffer0[0] == 0` — buffer vacío (la segunda frase es vacía/no existe)

Verifica comportamiento en edge case: conjunción colgante al final.

---

#### TEST 7 — `test_nextLS_single_verb_no_conjunction`

**Given**: frase mínima de una sola palabra  
```c
lsBuffer0 = [5, VERB, 0]
```
**When**: `nextLogicalSentence()`  
**Then**: `lsBuffer0[0] == 0`

Verifica que una frase de un solo token también funciona.

---

### 8.3. Actualización de REPORT_DAAD_BUGS.md

Actualizar la descripción de BUG-10 para reflejar que el problema es más grave que lo inicialmente documentado: el bug afecta a **toda** entrada con conjunción (no sólo a la "sin conjunción"), y se trata de un doble defecto (byte incorrecto + falta de guardia).

---

## 9. Plan de implementación

| Paso | Acción | Fichero |
|------|--------|---------|
| 1 | Añadir `extern uint8_t lsBuffer0[];` en `include/daad.h` | [include/daad.h](../include/daad.h) |
| 2 | Aplicar la corrección en `nextLogicalSentence` (2 líneas) | [src/daad/daad_parser_sentences.c:268-269](../src/daad/daad_parser_sentences.c#L268-L269) |
| 3 | Crear `unitTests/src/parser_stubs.c` con stubs mínimos para el módulo parser | nuevo fichero |
| 4 | Crear `unitTests/src/tests_daad_parser_sentences.c` con los 7 tests | nuevo fichero |
| 5 | Actualizar `unitTests/Makefile` — añadir target `parser.com` y `parser_stubs.rel` | [unitTests/Makefile](../unitTests/Makefile) |
| 6 | Actualizar `REPORT_DAAD_BUGS.md` — refinar descripción de BUG-10 | [REPORT_DAAD_BUGS.md](../REPORT_DAAD_BUGS.md) |
| 7 | Compilar y ejecutar suite completa → 0 FAIL | `make -C unitTests test` |

**Coste de binario estimado**: la corrección añade una instrucción de carga y una rama (el early return). En Z80 ~5-8 bytes y 10-15 ciclos por llamada. Impacto despreciable.

---

## 10. Referencias

| Fuente | Localización |
|--------|-------------|
| Bug report | [REPORT_DAAD_BUGS.md § BUG-10](../REPORT_DAAD_BUGS.md) |
| Código afectado | [src/daad/daad_parser_sentences.c:259-278](../src/daad/daad_parser_sentences.c#L259-L278) |
| Función simétrica | [src/daad/daad_parser_sentences.c:120-201](../src/daad/daad_parser_sentences.c#L120-L201) (`populateLogicalSentence`) |
| Struct Vocabulary | [include/daad.h:189-193](../include/daad.h#L189-L193) |
| VOC_TYPE enum | [include/daad.h:196-204](../include/daad.h#L196-L204) |
| DAAD 1991 — parser | `docs/DAAD_Manual_1991.md:2697-2770` |
| DAAD 1991 — conjunciones | `docs/DAAD_Manual_1991.md:2730` ("Phrases are separated by conjugations") |
| DRC vocab fuente (zakil ES) | `img/zakil/drc/zakil_xmes_ES.dsf:354-356` (Y/ENTONCE/LUEGO id=2) |
| DDB binario verificado | `img/zakil/drc/zakil_xmes_ES.DDB` — id=2, type=CONJUNCTION(5) |
| DDB format spec | `.agents/skills/daad-system/references/ddb-format.md:65-66` |
