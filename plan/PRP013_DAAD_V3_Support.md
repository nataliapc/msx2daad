# PRP013 — Soporte DAAD V3 (`-DDAADV3`)

- **ID**: PRP013
- **Tipo**: Nueva funcionalidad
- **Ficheros afectados**:
  - [`include/daad.h`](../include/daad.h)
  - [`include/daad_condacts.h`](../include/daad_condacts.h)
  - [`src/daad_condacts.c`](../src/daad_condacts.c)
  - [`src/daad/daad_parser_sentences.c`](../src/daad/daad_parser_sentences.c)
  - [`Makefile`](../Makefile)
- **Severidad**: Nueva funcionalidad mayor
- **Fecha**: 2026-04-16
- **Estado**: Implementado ✅
- **Fuente**: [`assets/V3/DAAD_V3.txt`](../assets/V3/DAAD_V3.txt) — especificación original
- **Especificación expandida**: [`assets/V3/DAAD_V3_full.md`](../assets/V3/DAAD_V3_full.md) — documentación completa V3 con tablas HASAT
- **Implementación de referencia**: [PCDAAD](https://github.com/Utodev/PCDAAD) — `condacts.pas`, `parser.pas` (Pascal/Turbo Pascal, DOS VGA)

> **Nota arquitectural**: PCDAAD detecta V3 en **tiempo de ejecución** (`DDBHeader.version = 3`).
> MSX2DAAD usa un flag de **compilación** (`-DDAADV3`) para reducir el tamaño del binario Z80.
> El comportamiento funcional es equivalente siempre que el DDB sea V3.

---

## 1. Resumen

DAAD V3 introduce un conjunto de nuevas características que amplían el sistema original (V2). Todas se activan mediante la macro de compilación `-DDAADV3`, siguiendo el mismo patrón de compilación condicional ya presente en el proyecto (e.g. `-DLANG_ES`, `-DMSX2`, `DISABLE_*`).

Las características son:

| ID | Característica | Ficheros | Complejidad |
|----|----------------|----------|-------------|
| V3-01 | Flag 53: 5 nuevos bits | `daad.h` | Baja |
| V3-02 | Parser: bit 5 (palabra no reconocida tras verbo) | `daad_parser_sentences.c` | Media |
| V3-03 | Parser: bit 4 (preposición antes de primer sustantivo) | `daad_parser_sentences.c` | Baja |
| V3-04 | `_internal_hasat`: flags alternativas 60-91 (bit 1 de fOFlags) | `daad_condacts.c` | Baja |
| V3-05 | DOALL: bit 0 de fOFlags si no hay objetos | `daad_condacts.c` | Baja |
| V3-06 | CONDACTO 120: `do_XMES` (mensaje con offset 16-bit) | `daad_condacts.c`, `daad_condacts.h` | Media |
| V3-07 | CONDACTO 122: `do_INDIR` (indirección en 2.º parámetro) | `daad_condacts.c`, `daad_condacts.h` | Media |
| V3-08 | CONDACTO 124: `do_SETAT` (set/clear/toggle bit de atributo) | `daad_condacts.c`, `daad_condacts.h` | Media |
| V3-09 | `do_PAUSE(0)` = GETKEY (esperar tecla) | `daad_condacts.c` | Baja |
| V3-10 | Bit 2 de fOFlags: sin pronombres enclíticos para verbos≥240 | N/A | No aplica (*) |

(*) El soporte de pronombres enclíticos (español) no está implementado en el intérprete actual; V3-10 queda documentado pero sin cambio de código.

---

## 2. Arquitectura unificada: compilación + versión en tiempo de ejecución

### Motivación

PCDAAD detecta V3 en **tiempo de ejecución** leyendo `DDBHeader.version == 3`. MSX2DAAD usa flags de **compilación** (`-DDAADV3`) para reducir el binario Z80. La solución propuesta unifica ambos:

- **Sin `-DDAADV3`**: el código V3 no se compila. Binario V2 puro, mínimo tamaño.
- **Con `-DDAADV3`**: el código V3 se compila **pero sólo se activa** si el DDB cargado es versión 3. El mismo binario puede ejecutar DDBs V2 y V3.

### Variable global `isV3` y macro `ISV3`

Implementada en [`include/daad.h`](../include/daad.h) como alias de una variable global booleana precomputada:

```c
#ifdef DAADV3
  extern bool isV3;       // definida en daad_global_vars.c
  #define ISV3 isV3
#endif
```

La variable se inicializa en `initDAAD()` **antes** del chequeo de versión, que a su vez aprovecha `isV3` directamente ([`src/daad/daad_init.c`](../src/daad/daad_init.c)):

```c
#ifdef DAADV3
    isV3 = (hdr->version == 3);
#endif
    if (hdr->version != 2 && !isV3)   // ← reutiliza isV3, no re-evalúa
        return false;
```

**Motivación Z80**: `hdr` es un puntero global, por lo que `hdr->version == 3` implica doble indirección en cada evaluación (~30 T-estados, 6 bytes). Un `bool isV3` global es acceso directo a dirección fija (~17 T-estados, 4 bytes) — ahorro de ~13 T-estados y 2 bytes por uso (11 call sites en el intérprete).

### Patrón de uso en el código

```c
// Condacto existente que cambia de comportamiento en V3:
void do_PAUSE() {
    uint16_t value = getValueOrIndirection();
#ifdef DAADV3
    if (ISV3 && !value) {       // ← compilado sólo con DAADV3; activo sólo en DDB V3
        while (!checkKeyboardBuffer());
        flags[fKey1] = getKeyInBuffer();
        flags[fKey2] = 0;
        return;
    }
#endif
    if (!value) value = 256;    // ← comportamiento V2 siempre presente
    setTime(0);
    while (getTime() < value);
}

// Condacto nuevo exclusivo de V3:
#ifdef DAADV3
void do_SETAT() {
    if (!ISV3) { pPROC += 2; return; }  // DDB V2: saltar 2 args y no hacer nada
    // ... implementación V3 ...
}
#endif

// Modificación de lógica existente con rama V3:
static void _internal_hasat(uint8_t value, bool negate) {
    uint8_t bit = 1 << (value & 7);
    uint8_t flagValue;
#ifdef DAADV3
    {
        uint8_t baseFlag = (ISV3 && (flags[fOFlags] & F53_ALTFLAGS)) ? 91 : 59;
        flagValue = flags[baseFlag - (value >> 3)] & bit;
    }
#else
    flagValue = flags[(fCOAtt+1)-(value>>3)] & bit;
#endif
    if (negate) flagValue = !flagValue;
    checkEntry = flagValue;
}
```

> **Excepción — condact list**: los punteros de función en `condactList[]` se asignan en tiempo de compilación. Las entradas 120/122/124 apuntan a `do_XMES`/`do_INDIR`/`do_SETAT` cuando `DAADV3` está definido. Cuando un DDB V2 se carga, estas funciones no serán llamadas (V2 no genera esos opcodes), por lo que el `if (!ISV3)` al inicio de cada una es sólo una salvaguarda.

---

## 3. Activación por defecto

Añadir al Makefile la opción `DAADV3` al grupo de flags opcionales:

**[`Makefile:26-27`](../Makefile#L26-L27)**

```makefile
ifndef CXXFLAGS
    CXXFLAGS := -DLANG_ES -DMSX2 -DDAADV3
```

> El flag puede quitarse para obtener un intérprete compatible V2 puro. El binario resultante debe funcionar igual que hoy si `DAADV3` no está definido.
> Con `-DDAADV3` activo, un DDB V2 (`hdr->version == 2`) ejecuta comportamiento V2 clásico en todo momento; sólo un DDB V3 activa las nuevas rutas.

---

## 4. Detalles de cada cambio

---

### V3-01 — Macro `ISV3` y nuevas constantes para flag 53 (fOFlags)

**Fichero**: [`include/daad.h`](../include/daad.h)

**1a) Variable `isV3` y macro `ISV3`** — junto a las constantes `MACHINE_*`:
```c
#ifdef DAADV3
  extern bool isV3;
  #define ISV3 isV3
#endif
```
Definición en `daad_global_vars.c`; inicialización en `initDAAD()` antes del chequeo de versión.

**1b) Constantes `F53_*`** — inmediatamente después de la línea `#define F53_LISTED 128`:

```c
#ifdef DAADV3
// V3: new bits of fOFlags (flag 53)
#define F53_DOALLNONE    1  // (fOFlags) Bitmask: DOALL sin objetos
#define F53_ALTFLAGS     2  // (fOFlags) Bitmask: flags alternativas 60-91
#define F53_NOPRONOUN    4  // (fOFlags) Bitmask: no pronombres enclíticos
#define F53_PREPFIRST   16  // (fOFlags) Bitmask: preposición antes de noun1
#define F53_UNRECWRD    32  // (fOFlags) Bitmask: palabra no reconocida tras verbo
#endif //DAADV3
```

---

### V3-02 — Parser: bit 5 de fOFlags (palabra no reconocida tras verbo)

**Fichero**: [`src/daad/daad_parser_sentences.c`](../src/daad/daad_parser_sentences.c)

**Referencia PCDAAD** (`parser.pas:572`):
```pascal
// en el bloque de "palabra no encontrada en vocabulario":
if V3CODE then if (getFlag(FVERB) <> NO_WORD) then SetFlagBit(FOBJECT_PRINT_FLAGS, 5);
```

PCDAAD realiza la búsqueda de vocabulario y el poblado de flags en **un único paso**; cuando llega a una palabra no encontrada, FVERB ya puede estar establecido en los flags. Nuestro intérprete separa los dos pasos (`parser()` tokeniza, `populateLogicalSentence()` asigna flags).

**Solución para nuestra arquitectura**: en `parser()`, rastrear con una variable local si se ha visto un verbo en la frase actual, y cuando no se encuentre una palabra, añadir un **token marcador especial** (`UNKNOWN_WORD`) al lsBuffer. `populateLogicalSentence()` leerá ese marcador y establecerá el bit 5, igual que maneja cualquier otro token.

**Paso 1** — añadir `UNKNOWN_WORD` al enum `VOC_TYPE` en [`include/daad.h:184`](../include/daad.h#L184):
```c
enum VOC_TYPE {
    VERB,           // 0
    ADVERB,         // 1
    NOUN,           // 2
    ADJECTIVE,      // 3
    PREPOSITION,    // 4
    CONJUNCTION,    // 5
    PRONOUN,        // 6
#ifdef DAADV3
    UNKNOWN_WORD    // 7 — marcador V3: palabra no encontrada después de un verbo
#endif
};
```

**Paso 2** — en `parser()` ([L24-93](../src/daad/daad_parser_sentences.c#L24-L93)), detectar y emitir el marcador:
```c
#ifdef DAADV3
    bool verbSeen = false;  // local: ¿se ha visto verbo en la frase actual?
#endif
    // ... dentro del bucle while(*p), tras el bucle de búsqueda en vocabulario:
    if (!voc->word[0]) {
        // palabra no encontrada en vocabulario
#ifdef DAADV3
        if (ISV3 && verbSeen) {
            *lsBuffer++ = 0;            // id irrelevante
            *lsBuffer++ = UNKNOWN_WORD; // marcador: palabra no reconocida tras verbo
            *lsBuffer = 0;
        }
#endif
    } else {
        // palabra encontrada — registrar si es verbo o conjunción
#ifdef DAADV3
        if (ISV3) {
            if (voc->type == VERB)        verbSeen = true;
            if (voc->type == CONJUNCTION) verbSeen = false;  // reset entre frases
        }
#endif
        *lsBuffer++ = voc->id;
        *lsBuffer++ = voc->type;
        *lsBuffer = 0;
    }
```

**Paso 3** — en `populateLogicalSentence()` ([L102-172](../src/daad/daad_parser_sentences.c#L102-L172)), al inicio del bucle de tokens (tras las líneas de clear de flags):
```c
#ifdef DAADV3
    // V3: limpiar bits 4 y 5 al inicio de cada frase lógica
    if (ISV3) flags[fOFlags] &= ~(F53_UNRECWRD | F53_PREPFIRST);
#endif
```

Y en el cuerpo del `while (*p && *(p+1)!=CONJUNCTION)`, añadir el tratamiento del marcador:
```c
#ifdef DAADV3
        } else if (type == UNKNOWN_WORD) {
            // palabra no reconocida en vocabulario apareció tras un verbo
            flags[fOFlags] |= F53_UNRECWRD;
#endif
```

> **Por qué en `populateLogicalSentence()` y no en `parser()`**: PCDAAD limpia los bits 4 y 5 al inicio de **cada frase lógica**, no una vez por línea de entrada. Al colocar el clear en `populateLogicalSentence()` se reproduce ese comportamiento exacto: en una entrada compuesta (`COGER ESPADA Y MATAR DESCONOCIDO`) los bits se limpian y recalculan para cada sub-frase.

---

### V3-03 — Parser: bit 4 de fOFlags (preposición antes del primer sustantivo)

**Fichero**: [`src/daad/daad_parser_sentences.c`](../src/daad/daad_parser_sentences.c)

**Referencia PCDAAD** (`parser.pas:528`):
```pascal
if V3CODE then if (getFlag(FNOUN) = NO_WORD) then setFlagBit(FOBJECT_PRINT_FLAGS, 4);
```

**Dónde limpiar** — ya cubierto en V3-02 Paso 3: la limpieza de bits 4 y 5 al inicio de `populateLogicalSentence()` cubre también el bit 4.

**Dónde detectar**: en `populateLogicalSentence()` ([L138](../src/daad/daad_parser_sentences.c#L138)), en el bloque de procesamiento de PREPOSITION:

```c
    } else if (type==PREPOSITION && flags[fPrep]==NULLWORD) {   // PREP
        flags[fPrep] = id;
#ifdef DAADV3
        if (ISV3 && flags[fNoun1] == NULLWORD) flags[fOFlags] |= F53_PREPFIRST;
#endif
        ret = true;
```

---

### V3-04 — `_internal_hasat`: flags alternativas 60-91 (bit 1 de fOFlags)

**Fichero**: [`src/daad_condacts.c:580-590`](../src/daad_condacts.c#L580-L590)

**Referencia PCDAAD** (`condacts.pas:1207-1218`):
```pascal
procedure _HASAT;
var baseFlag: TFlagType;
begin
  if (getFlagBit(FOBJECT_PRINT_FLAGS, 1)) then baseFlag := 91 else baseFlag := 59;
  condactResult := getFlagBit(baseFlag - (parameter1 div 8), parameter1 mod 8);
end;
```

> **Importante**: en PCDAAD, la lógica de flags alternativas **no está protegida por `V3CODE`**. Funciona en cualquier versión de DDB si el autor pone a 1 el bit 1 de flag 53. Para MSX2DAAD, la compilación condicional `#ifdef DAADV3` cumple el mismo propósito de forma estática, pero dentro del bloque el chequeo del bit 1 es **siempre incondicional** (no hay un segundo `if DAADV3` interior).

**Código actual**:
```c
static void _internal_hasat(uint8_t value, bool negate)
{
    uint8_t bit, flagValue;
    bit = 1 << (value & 7);
    flagValue = flags[(fCOAtt+1)-(value>>3)] & bit;

    if (negate) flagValue = !flagValue;
    checkEntry = flagValue;
}
```

**Corrección** — fórmula equivalente a PCDAAD (`baseFlag` = 59 ó 91; índice baja desde baseFlag):
```c
static void _internal_hasat(uint8_t value, bool negate)
{
    uint8_t bit, flagValue;
    bit = 1 << (value & 7);
#ifdef DAADV3
    // Cuando bit 1 de fOFlags está activo: banco alternativo flags 60-91
    // baseFlag = 91 → flags[91 - (value>>3)]  (range 60-91)
    // baseFlag = 59 → flags[59 - (value>>3)]  (range standard, equivalente a fCOAtt+1)
    {
        uint8_t baseFlag = (ISV3 && (flags[fOFlags] & F53_ALTFLAGS)) ? 91 : 59;
        flagValue = flags[baseFlag - (value >> 3)] & bit;
    }
#else
    flagValue = flags[(fCOAtt+1)-(value>>3)] & bit;  // fCOAtt+1 = 59
#endif
    if (negate) flagValue = !flagValue;
    checkEntry = flagValue;
}
```

> **Verificación de equivalencia**: `baseFlag=59` → `flags[59-(value>>3)]` = `flags[(fCOAtt+1)-(value>>3)]` ya que `fCOAtt=58`. ✓
>
> **Rango alternativo**: `baseFlag=91` → `flags[91-(value>>3)]`, para value 0-7: flag 91, value 8-15: flag 90, …, value 248-255: flag 60. Cubre exactamente 32 bytes (flags 60-91).

---

### V3-05 — DOALL: bit 0 de fOFlags si no encuentra objetos

**Fichero**: [`src/daad_condacts.c:2176-2204`](../src/daad_condacts.c#L2176-L2204)

**Referencia PCDAAD** (`condacts.pas:1460,1467`):
```pascal
// al inicio de _DOALL, ANTES de buscar objetos:
if (V3CODE) then SetFlagBit(FOBJECT_PRINT_FLAGS, 0);   // SET: "aún no se encontró nada"

// cuando SE ENCUENTRA el primer objeto válido:
if V3CODE then ClearFlagBit(FOBJECT_PRINT_FLAGS, 0);   // CLEAR: "sí hay objetos"
```

**Semántica exacta (confirmada por PCDAAD)**:

| Momento | Acción sobre bit 0 |
|---------|-------------------|
| Inicio de DOALL | **SET** (señaliza "sin objetos aún") |
| Se encuentra el primer objeto válido | **CLEAR** (hay al menos un objeto) |
| DOALL termina sin objetos | bit 0 queda **SET** (nunca se limpió) |

> **⚠️ Bug en versión anterior del PRP**: la lógica estaba **invertida** — se limpiaba al inicio y se establecía al fallar. La lógica correcta (PCDAAD) es la contraria.

**En `do_DOALL()`** — **ESTABLECER** el bit al inicio (L2198):
```c
void do_DOALL() {   // locno+
#ifdef DAADV3
    if (ISV3) flags[fOFlags] |= F53_DOALLNONE;   // SET: todavía no sabemos si habrá objetos
#endif
    if (currProc->condactDOALL) errorCode(4);
    currProc->condactDOALL = ++pPROC;
    currProc->entryDOALL = currProc->entry;
    flags[fDAObjNo] = NULLWORD;
    _internal_doall();
}
```

**En `_internal_doall()`** — **LIMPIAR** el bit cuando SE ENCUENTRA un objeto (L2191-2196, justo antes de `flags[fDAObjNo] = objno`):
```c
    // objeto encontrado — actualizar flags
#ifdef DAADV3
    if (ISV3) flags[fOFlags] &= ~F53_DOALLNONE;  // CLEAR: se encontró al menos un objeto
#endif
    flags[fDAObjNo] = objno;
    flags[fNoun1] = obj->nounId;
    flags[fAdject1] = obj->adjectiveId;
    pPROC = currProc->condactDOALL;
    currProc->entry = currProc->entryDOALL;
```

> **Sin cambio cuando no hay objetos**: cuando `objno >= hdr->numObjDsc`, el bit 0 ya está SET (desde `do_DOALL()`), no hace falta establecerlo de nuevo. Sólo hay que NO limpiarlos.

---

### V3-06 — Condacto 120: `do_XMES` (mensaje con offset 16-bit en fichero externo)

**Contexto**: XMES es el condacto nativo V3 equivalente al `EXTERN <lsb> 3 <msb>` de MALUVA. Imprime un mensaje del **fichero externo de textos** (`TEXTS.XDB`) referenciado por un offset de 16 bits. El mecanismo y el fichero son exactamente los mismos que usaba MALUVA; sólo cambia el encoding en el DDB:

| Versión | Encoding en DDB | Bytes |
|---------|-----------------|------:|
| MALUVA via `EXTERN` | `<EXTERN> <lsb> 3 <msb>` | 4 |
| V3 nativo | `<XMES> <lsb> <msb>` | 3 |

La implementación reutiliza directamente `printXMES()` (`include/daad_platform_api.h:59`), la misma función que usa `do_EXTERN` case 3.

**Parámetros**: 2 bytes — LSB y MSB del offset en el fichero externo.

**`include/daad_condacts.h`** — añadir la declaración junto a las otras (tras `do_NOT_USED`):
```c
#ifdef DAADV3
void do_XMES();
void do_INDIR();
void do_SETAT();
#endif
```

**`src/daad_condacts.c`** — tabla `condactList[]` (L51):
```c
// Actual (línea 51):
{ do_NOT_USED,  1 }, { do_COPYOO, 1 }, { do_NOT_USED, 1 }, { do_COPYFO, 1 }, { do_NOT_USED, 1 },  // 120-124

// Nuevo:
#ifdef DAADV3
{ do_XMES,  1 }, { do_COPYOO, 1 }, { do_INDIR, 1 },{ do_COPYFO, 1 }, { do_SETAT, 1 },             // 120-124
#else
{ do_NOT_USED, 1 }, { do_COPYOO, 1 }, { do_NOT_USED, 1 }, { do_COPYFO, 1 }, { do_NOT_USED, 1 },   // 120-124
#endif
```

**`src/daad_condacts.c`** — tabla VERBOSE `CONDACTS[]` (L79):
```c
// Actual:
{ "NOT_USED1", 0 }, { "COPYOO", 2 }, { "NOT_USED2", 0 }, { "COPYFO", 2 }, { "NOT_USED3", 0 },

// Nuevo:
#ifdef DAADV3
{ "XMES",  2 }, { "COPYOO", 2 }, { "INDIR", 1 }, { "COPYFO", 2 }, { "SETAT", 2 },
#else
{ "NOT_USED1", 0 }, { "COPYOO", 2 }, { "NOT_USED2", 0 }, { "COPYFO", 2 }, { "NOT_USED3", 0 },
#endif
```

**Implementación de `do_XMES()`** — añadir cerca de `do_EXTERN` (L~2345), ya que es su reemplazo directo:
```c
#ifdef DAADV3
/* XMES (condacto 120): equivalente nativo a EXTERN <lsb> 3 <msb> de MALUVA.
   Imprime desde fichero externo TEXTS.XDB usando offset de 16 bits.
   Reutiliza printXMES() (daad_platform_api.h:59), igual que do_EXTERN case 3. */
void do_XMES()  // lsb msb
{
    if (!ISV3) { pPROC += 2; return; }  // ignorar en DDB V2
    uint8_t lsb = *pPROC++;
    uint8_t msb = *pPROC++;
    printXMES((uint16_t)lsb | ((uint16_t)msb << 8));
}
#endif //DAADV3
```

---

### V3-07 — Condacto 122: `do_INDIR` (indirección en 2.º parámetro)

**Contexto**: El compilador DRC genera `INDIR flagno` justo antes del condacto que usa `@` en el segundo parámetro. INDIR parchea en RAM el byte que corresponde al segundo argumento del siguiente condacto.

**Ejemplo DRC**:
```
LET 100 @12  →  <INDIR> 12 <LET> 100 0
```

**Referencia PCDAAD** (`condacts.pas:2176-2183`):
```pascal
procedure _INDIR;
begin
  if (V3CODE) then
    writeByte(CondactPTR+3, getFlag(Parameter1));
  done := true;
end;
```
`CondactPTR` en PCDAAD apunta al **byte del parámetro 1** (el número de flag) cuando el procedimiento ejecuta, equivalente a `pPROC` en nuestro código en el momento en que `do_INDIR()` es llamada.

**Mecanismo de funcionamiento — alineación de offsets**:

| Offset relativo a `CondactPTR` (PCDAAD) / `pPROC` entrada (MSX2) | Contenido |
|------|-----------|
| +0 | parámetro de INDIR: número de flag ← `pPROC` al entrar en `do_INDIR()` |
| +1 | byte de condacto siguiente (LET en el ejemplo) |
| +2 | 1.er argumento del condacto siguiente (100) |
| **+3** | **2.º argumento — se parchea con `flags[flagno]`** |

PCDAAD escribe en `CondactPTR+3`. En nuestro código, tras leer el parámetro (`uint8_t flagno = *pPROC++`), `pPROC` avanza a `+1`, por lo que el byte a parchear queda en `pPROC + 2` = `+3` desde la entrada. **La implementación `*(pPROC + 2)` es correcta** ✓.

> **Modificación en RAM**: INDIR escribe directamente en el DDB cargado en memoria. El DDB de MSX2/DOS está en RAM escribible. El parche es idempotente: INDIR siempre reescribe el valor antes de ejecutar el condacto siguiente.

**Implementación** — añadir junto a `do_SETAT` (ver V3-08):
```c
#ifdef DAADV3
/* INDIR (condacto 122): condacto interno generado por DRC para indirección en
   2.º parámetro. Escribe flags[flagno] en el 2.º byte de argumento del condacto
   siguiente, modificando el DDB en RAM. (Equivale a CondactPTR+3 en PCDAAD.) */
void do_INDIR()  // flagno
{
    if (!ISV3) { pPROC++; return; }  // ignorar en DDB V2 (saltar 1 arg)
    uint8_t flagno = *pPROC++;       // leer parámetro; pPROC avanza a +1 (próximo condacto)
    *(pPROC + 2) = flags[flagno];    // +3 desde entrada = 2.º arg del condacto siguiente
}
#endif //DAADV3
```

---

### V3-08 — Condacto 124: `do_SETAT` (set/clear/toggle de bit de atributo)

**Contexto**: SETAT es la contrapartida acción de HASAT. Donde HASAT/HASNAT comprueban un bit, SETAT lo modifica. Respeta el mismo bit 1 de fOFlags (flags alternativas 60-91) que V3-04.

**Referencia PCDAAD** (`condacts.pas:2186-2211`):
```pascal
procedure _SETAT;
var baseFlag, finalFlag: TFlagType;
    bit: byte;
begin
  if V3CODE then BEGIN
    if (getFlagBit(FOBJECT_PRINT_FLAGS, 1)) then baseFlag := 91 else baseFlag := 59;
    finalFlag := baseFlag - (Parameter1 div 8);
    bit := Parameter1 mod 8;
    Parameter2 := Parameter2 and 3;
    if (Parameter2 = 3) then Parameter2 := 2;  // valor 3 → toggle, igual que 2
    case Parameter2 of
      0: ClearFlagBit(finalFlag, bit);
      1: SetFlagBit(finalFlag, bit);
      2: ToggleFlagBit(finalFlag, bit);
    end;
  END;
  done := true;
end;
```

**Parámetros**: `value` (qué bit, mismo rango que HASAT) y `operation`:

| `operation & 3` | Acción |
|-----------------|--------|
| 0 | Borrar bit (clear) |
| 1 | Establecer bit (set) |
| 2 ó 3 | Invertir bit (toggle) — PCDAAD mapea explícitamente 3→2 antes del switch |

> La especificación V3 pide aplicar `AND 3` y tratar el resto como toggle. PCDAAD mapea explícitamente 3→2 para hacer eso. Nuestro `else` lo cubre de forma equivalente sin necesidad del mapeo explícito.

**Implementación** — añadir junto a `do_HASAT`, usando la misma fórmula `baseFlag` que V3-04:
```c
#ifdef DAADV3
/* SETAT (condacto 124): set/clear/toggle del bit de atributo indicado por value.
   Usa la misma lógica baseFlag que _internal_hasat (PCDAAD condacts.pas:2186).
   operation AND 3: 0=clear, 1=set, 2/3=toggle. */
void do_SETAT()  // value operation
{
    if (!ISV3) { pPROC += 2; return; }  // ignorar en DDB V2
    uint8_t value = getValueOrIndirection();
    uint8_t operation = *pPROC++ & 3;
    uint8_t bit = 1 << (value & 7);
    uint8_t baseFlag = (flags[fOFlags] & F53_ALTFLAGS) ? 91 : 59;  // igual que _internal_hasat
    uint8_t *flagPtr = &flags[baseFlag - (value >> 3)];

    if (operation == 0) {
        *flagPtr &= ~bit;           // clear
    } else if (operation == 1) {
        *flagPtr |= bit;            // set
    } else {
        *flagPtr ^= bit;            // toggle (operation 2 ó 3)
    }
}
#endif //DAADV3
```

---

### V3-09 — `do_PAUSE(0)` = GETKEY

**Fichero**: [`src/daad_condacts.c:2054-2061`](../src/daad_condacts.c#L2054-L2061)

**Comportamiento actual**: `PAUSE 0` espera 256/50 segundos (≈5,12 s).

**Comportamiento V3**: `PAUSE 0` espera una pulsación de tecla y la almacena en `flags[fKey1]` (flag 60) y `flags[fKey2]` (flag 61 = 0 en plataformas 8-bit). Equivalente a un INKEY bloqueante. `GETKEY` en el DSF es una macro del compilador DRC que se traduce a `PAUSE 0`.

> **Relación con `do_INKEY`**: INKEY (condacto 111) es no bloqueante — comprueba el buffer y retorna false si no hay tecla. PAUSE 0 en V3 es bloqueante — espera hasta que el jugador pulse una tecla. Ambos almacenan el resultado en `fKey1/fKey2`.

**Corrección** (usa `ISV3` para mantener el comportamiento V2 intacto cuando se carga un DDB V2):
```c
void do_PAUSE()     // value
{
    uint16_t value = getValueOrIndirection();
#ifdef DAADV3
    if (ISV3 && !value) {
        while (!checkKeyboardBuffer());
        flags[fKey1] = getKeyInBuffer();
        flags[fKey2] = 0;
        return;
    }
#endif
    if (!value) value = 256;
    setTime(0);
    while (getTime() < value);
}
```

---

### V3-10 — Bit 2: sin pronombres enclíticos para verbos≥240 (N/A)

**Referencia PCDAAD** (`parser.pas:547`, `global.pas`):
```pascal
LAST_PRONOMINAL_VERB = 239;
...
if (not LimitEnclicitPronouns) or (AWordRecord.ACode <= LAST_PRONOMINAL_VERB) then
  // aplicar pronombre enclítico
```
Cuando bit 2 de fOFlags está activo, los pronombres enclíticos sólo se aplican a verbos con código ≤ 239; verbos ≥ 240 quedan excluidos.

**Estado**: **Sin implementación**. El intérprete actual no incluye mecanismo de pronombres enclíticos (característica del español, e.g., "Tómalo" → verbo "tomar" + pronombre "lo"). Los flags `fCPNoun` (46) y `fCPAdject` (47) existen como estructura, pero no hay código de aplicación automática de pronombres. Si en el futuro se implementa la resolución de pronombres enclíticos, el bit 2 de fOFlags debe consultarse antes de aplicarlos cuando el verbo es ≥ 240 (`LAST_PRONOMINAL_VERB = 239`).

---

### EXTRA-01 — Bugfix V2: bit 6 de fOFlags (`OO_CLIST`) no leído en `_internal_listat`

**Fichero**: [`src/daad_condacts.c:1887-1908`](../src/daad_condacts.c#L1887-L1908)

**Contexto**: el bit 6 de flag 53 (máscara 64) controla el modo de listado de objetos en PCDAAD. Cuando bit 6 = 0 (por defecto), LISTOBJ imprime cada objeto en una línea separada (con `NEWLINE` como separador). Cuando bit 6 = 1, LISTOBJ usa el modo "continuo" (objetos separados por SM46 `", "` / SM47 `" y "` / SM48 `".\n"`).

**Justificación documental** — [`docs/DAAD_Manual_1991.md:2882-2886`](../docs/DAAD_Manual_1991.md#L2882-L2886):
> Flag 53 holds object print flags
>  7 - Set if any object printed as part of LISTOBJ or LISTAT
>  6 - **Set this to cause continuous object listing**. i.e. `LET 53 64` will make PAW list objects on the same line forming a valid sentence.

Confirmado también en la documentación [`docs/DAAD_Ready_Documentation_V2.md:1942`](../docs/DAAD_Ready_Documentation_V2.md#L1942) con el mismo texto para DAAD. El manual especifica claramente que bit 6 debe **activar** el modo continuo — implicando que **sin bit 6** (comportamiento por defecto) el listado es por líneas (con NEWLINE).

**Referencia PCDAAD** (`condacts.pas:332`):
```pascal
continuousListing := (getFlag(FOBJECT_PRINT_FLAGS) AND 64) <> 0;
...
if (continuousListing) then begin
    if (listed = count) then Sysmess(SM48)
    else if (listed = count - 1) then Sysmess(SM47)
    else Sysmess(SM46);
end
else _NEWLINE;
```

**Bug en MSX2DAAD**: `_internal_listat()` **siempre** usa separadores SM46/SM47 (modo continuo), ignorando el bit 6. No tiene el fallback a `NEWLINE` cuando bit 6 = 0. El bit 6 se preserva (`&= 0x7F` solo limpia bit 7), pero nunca se lee.

**Corrección** — sin `#ifdef DAADV3` (es funcionalidad V2):
```c
static void _internal_listat(uint8_t loc, bool listobj) {
    uint8_t lastFound = NULLWORD;
    uint8_t n = hdr->numObjDsc;
    bool contList = flags[fOFlags] & 64;                    // ← NUEVO: leer bit 6
    flags[fOFlags] &= (F53_LISTED ^ 255);
    for (uint8_t i=0; i<=n; i++) {
        if (i==n || objects[i].location == loc) {
            if (lastFound!=NULLWORD) {
                if (listobj && !(flags[fOFlags] & F53_LISTED)) {
                    printSystemMsg(1);
                    if (!contList) do_NEWLINE();             // ← NUEVO: newline tras SM1 si no continuo
                }
                if (flags[fOFlags] & F53_LISTED) {
                    if (contList) {                          // ← NUEVO: separadores solo en modo continuo
                        if (i<n)
                            printSystemMsg(46);
                        else
                            printSystemMsg(47);
                    } else {
                        do_NEWLINE();                        // ← NUEVO: newline entre objetos si no continuo
                    }
                }
                printObjectMsg(lastFound);
                flags[fOFlags] |= F53_LISTED;
            }
            lastFound = i;
        }
    }
}
```

> **Impacto**: bajo. Los DDBs de juegos MSX2 existentes que usan SM46/SM47/SM48 como separadores ya funcionan — el bit 6 está a 0 por defecto y ahora producirían listado por línea (que es el estándar PCDAAD). Si un juego depende del comportamiento actual (siempre inline), el autor puede establecer bit 6 con `LET 53 64` o `SETAT` para restaurar el modo continuo.

#### Tests unitarios (en `tests_condacts5.c`, no en `cond_v3.com` — es funcionalidad V2)

`do_NEWLINE()` llama `printChar('\r')` → capturado en `fake_lastCharPrinted`. Los stubs de `printSystemMsg()` **no** llaman a `printChar`. Esto permite distinguir los dos modos: si `_internal_listat` usó `do_NEWLINE()` como separador, `fake_lastCharPrinted == '\r'`; si usó SM46/SM47, `fake_lastCharPrinted` queda intacto (-1).

```c
/* LISTOBJ: bit 6 = 0 (default) → newline between objects, not SM46/SM47 */
void test_LISTOBJ_contlist_off()
{
    const char *_func = __func__;
    beforeEach();
    flags[fPlayer] = 3;
    flags[fOFlags] = 0;                        // bit 6 = 0 → newline mode
    objects[0].location = 3; objects[0].nounId = 10;
    objects[1].location = 3; objects[1].nounId = 11;

    static const char proc[] = { _LISTOBJ, 255 };
    do_action(proc);

    ASSERT_EQUAL(fake_lastCharPrinted, '\r',
        "LISTOBJ with bit6=0 must use do_NEWLINE between objects");
    SUCCEED();
}

/* LISTOBJ: bit 6 = 1 → continuous mode, SM46/SM47 separators, no NEWLINE */
void test_LISTOBJ_contlist_on()
{
    const char *_func = __func__;
    beforeEach();
    flags[fPlayer] = 3;
    flags[fOFlags] = 64;                       // bit 6 = 1 → continuous mode
    objects[0].location = 3; objects[0].nounId = 10;
    objects[1].location = 3; objects[1].nounId = 11;

    static const char proc[] = { _LISTOBJ, 255 };
    do_action(proc);

    ASSERT_EQUAL(fake_lastCharPrinted, -1,
        "LISTOBJ with bit6=1 must NOT use do_NEWLINE (uses SM separators)");
    SUCCEED();
}
```

Añadir las llamadas al `main()` de `tests_condacts5.c` junto a los tests existentes de LISTOBJ.

---

---

### EXTRA-02 — Bugfix: `referencedObject` sobreescribe bytes completos en flags[56/57]

**Fichero**: [`src/daad/daad_objects.c:53-54`](../src/daad/daad_objects.c#L53-L54)
**Descubierto al**: depurar `SETCO ERROR` en `assets/V3/TEST.DSF` (2026-04-17)

**Justificación documental** — [`docs/DAAD_Manual_1991.md:2891-2892`](../docs/DAAD_Manual_1991.md#L2891-L2892):
> Flag 56 **is 128** if the currently referenced object is a container.
> Flag 57 **is 128** if the currently referenced object is wearable

Confirmado también en [`docs/DAAD_Ready_Documentation_V2.md:1945-1946`](../docs/DAAD_Ready_Documentation_V2.md#L1945-L1946) con texto equivalente. La redacción del manual 1991 ("is 128") es categórica — **no dice** "bit 7 is set" ni "bit 7 reflects container status"; dice que el valor del flag **es 128 o no lo es**. Igualmente la tabla de símbolos HASAT ([`docs/DAAD_Manual_1991.md:1063-1064`](../docs/DAAD_Manual_1991.md#L1063-L1064)) documenta:
> WEARABLE  Flag 57 - Bit7  23      Current object is wearable
> CONTAINER Flag 56 - Bit7  31      Current object is a container

Lo que define un **único bit significativo** en cada flag (bit 7). Cualquier otro bit sería basura.

Los posibles valores son **0 ó 128**, sin más bits significativos.

**Bug**: el código usaba bit-masking que **preservaba los bits 0-6** del valor anterior del flag:
```c
// INCORRECTO — preserva bits 0-6, acumula basura de usos anteriores
flags[fCOCon] = flags[fCOCon] & 0b01111111 | (isContainer << 7);
flags[fCOWR]  = flags[fCOWR]  & 0b01111111 | (isWareable  << 7);
```

Con flags sin inicializar (ver EXTRA-03), bits 0-6 = `0x7F` → tras SETCO con objeto no-container, `flags[56] = 0x7F` en vez de 0 → `ZERO fRefObjIsContainer` falla.

**Corrección**:
```c
flags[fCOCon] = objRef->attribs.mask.isContainer << 7;  // Flag 56: 128 o 0
flags[fCOWR]  = objRef->attribs.mask.isWareable  << 7;  // Flag 57: 128 o 0
```

**Test unitario**: `unitTests/src/tests_daad_objects.c` — asserts de `flags[fCOCon]` y `flags[fCOWR]` actualizados de `(& 0x80) == 0/128` a `== 0/128` (ahora prueban byte completo):
```c
ASSERT_EQUAL(flags[fCOCon], 0,   "flags[fCOCon] must be 0 (not container)");
ASSERT_EQUAL(flags[fCOWR],  128, "flags[fCOWR] must be 128 (wearable)");
```

---

### EXTRA-03 — Bugfix: `initFlags()` no inicializaba el array `flags[]` a cero

**Fichero**: [`src/daad/daad_init.c:104-138`](../src/daad/daad_init.c#L104-L138)
**Descubierto al**: depurar `SETCO ERROR` en `assets/V3/TEST.DSF` (2026-04-17)

**Justificación documental** — [`docs/DAAD_Manual_1991.md:699-706`](../docs/DAAD_Manual_1991.md#L699-L706), sección "Initialise":
> The system initialise is carried out only once. **All flags and object positions are cleared.** The screen is cleared (although this may change on 8 bit to provide loading screen maintenance). Note that **clearing the flags has the effect that the game always starts at location zero**. This is because Player, the current location of the player, is now zero.

El manual es inequívoco: "**All flags** and object positions are cleared" durante la inicialización del sistema. No es una recomendación, es parte de la especificación del *game loop*.

**Contexto**: en C, los arrays globales están en la sección BSS y deben ser cero al arrancar. El CRT0 MSX usado (`crt0msx_msxdos_advanced.s`) **no limpia BSS**; además, `hex2bin` rellena con `0xFF` las regiones no inicializadas del `.com`. Resultado: `flags[]` arrancaba con `0xFF` en todas las posiciones — violando directamente la especificación 1991.

**Verificación** — bytes en el `.com` generado:
```
$ xxd -s 0x46C7 -l 16 obj/msx2daad.com
000046c7: ffff ffff ffff ffff ffff ffff ffff ffff  ................
```

**Bug**: `initFlags()` solo asignaba `flags[fPlayer] = 0` y algunos flags de ventana, dejando el resto a `0xFF`. Las condiciones del test DSF como `ZERO fRefObjIsContainer` fallaban porque los bits 0-6 de flags[56] partían de `0x7F`.

**Corrección** — añadir `memset(flags, 0, 256)` al inicio de `initFlags()`:
```c
void initFlags()
{
    uint8_t i;
    memset(flags, 0, 256);          // ← NUEVO: BSS no se limpia por CRT0
    flags[fPlayer] = 0;             // redundante pero explícito
    ...
}
```

> **Impacto**: correcto según la especificación DAAD (flags de usuario 64-254 deben empezar a 0). No hay juegos que dependan del estado inicial de flags sin establecerlos explícitamente, ya que el propio juego los inicializa en PRO 0.

---

### EXTRA-04 — Bugfix: DOALL sin DONE explícito solo iteraba una vez

**Fichero**: [`src/daad_condacts.c:208-211`](../src/daad_condacts.c#L208-L211)
**Descubierto al**: depurar `DOALL ERROR` en `assets/V3/TEST.DSF` (2026-04-17)

**Justificación documental** — [`docs/DAAD_Manual_1991.md:1923-1950`](../docs/DAAD_Manual_1991.md#L1923-L1950), descripción de `DOALL locno+`:
> Another powerful action which allows the implementation of an 'ALL' type command.
>
>   1 - An attempt is made to find an object at Location locno. If this is unsuccessful the DOALL is cancelled and action DONE is performed.
>   2 - The object number is converted into the LS Noun1 (and Adjective1 if present)…
>   3 - The next condact and/or entry in the table is then considered. This effectively converts a phrase of "Verb All" into "Verb object" which is then processed by the table as if the player had typed it in.
>   4 - **When an attempt is made to exit the current table, if the DOALL is still active (i.e. has not been cancelled by an action) then the attention of DAAD is returned to the DOALL as from step 1**; with the object search continuing from the next highest object number to that just considered.

El **paso 4** del manual describe exactamente el comportamiento que faltaba: al **intentar salir de la tabla actual** (nuestro `verb==0` → `_popPROC()`), si DOALL sigue activo, hay que volver al DOALL (equivalente a llamar `do_DONE()` internamente, que a su vez llama `_internal_doall()`). Confirmado también en [`docs/DAAD_Ready_Documentation_V2.md:1431-1434`](../docs/DAAD_Ready_Documentation_V2.md#L1431-L1434) con texto idéntico.

**Contexto**: el mecanismo DOALL avanza al siguiente objeto en dos situaciones:
1. DONE se llama explícitamente dentro del cuerpo del bucle (`do_DONE()` → `_internal_doall()`).
2. La tabla de proceso se agota sin que ninguna entrada coincida → DONE implícito (paso 4 del manual).

**Bug**: cuando la tabla de proceso se agotaba (se llegaba al final con `verb==0`), el código llamaba directamente `_popPROC()` sin verificar si DOALL estaba activo. Con un PRO de cuerpo simple sin DONE explícito:
```
/PRO 6
> _ _ DOALL 1
      PLUS 100 1      ; ← sin DONE
```
Solo se ejecutaba una iteración (el primer objeto) y luego se salía del proceso. El test `assets/V3/TEST.DSF` usa exactamente este patrón y espera que `flag[100] == 4` tras iterar 4 objetos.

**Corrección** — en la rama `verb==0` del bucle de búsqueda de entradas:
```c
// ANTES:
if (currProc->entry->verb==0) {
    _popPROC();
    break;
}

// DESPUÉS: DONE implícito si DOALL está activo
if (currProc->entry->verb==0) {
    if (currProc->condactDOALL) do_DONE();  // ← avanzar al siguiente objeto
    else _popPROC();
    break;
}
```

`do_DONE()` con `condactDOALL != NULL` llama `_internal_doall()`:
- Si hay más objetos: reinicia `pPROC` y `entry` al cuerpo del bucle → siguiente iteración.
- Si no hay más objetos: limpia `condactDOALL`, llama `do_DONE()` de nuevo (ahora `condactDOALL == NULL`) → `isDone=true`, `checkEntry=false` → `_popPROC()` en la siguiente vuelta.

> **Compatibilidad con DONE explícito**: el camino con DONE explícito (`do_DONE()` → `_internal_doall()` → reinicio del bucle) no pasa por la rama `verb==0` mientras haya más objetos — sigue funcionando igual que antes.

---

## 5. Plan de implementación

### Fase 1 — Infraestructura y definiciones (sin riesgo)

- [x] **V3-01a**: Añadir macro `ISV3` en [`include/daad.h`](../include/daad.h) (~L82, junto a `MACHINE_*`)
- [x] **V3-01b**: Añadir `UNKNOWN_WORD = 7` al enum `VOC_TYPE` bajo `#ifdef DAADV3` ([`include/daad.h:184`](../include/daad.h#L184))
- [x] **V3-01c**: Añadir constantes `HAS_*` y `F53_*` en [`include/daad.h`](../include/daad.h) bajo `#ifdef DAADV3`
- [x] Añadir declaraciones `do_XMES()`, `do_INDIR()`, `do_SETAT()` en [`include/daad_condacts.h`](../include/daad_condacts.h) bajo `#ifdef DAADV3`
- [x] Actualizar `condactList[]` (L51) y tabla VERBOSE `CONDACTS[]` (L79) en [`src/daad_condacts.c`](../src/daad_condacts.c)
- [x] Añadir `-DDAADV3` al `CXXFLAGS` por defecto en el [`Makefile`](../Makefile)

### Fase 2 — Condactos nuevos (bajo riesgo)

- [x] **V3-06**: Implementar `do_XMES()` en [`src/daad_condacts.c`](../src/daad_condacts.c) (zona de mensajes)
- [x] **V3-07**: Implementar `do_INDIR()` en [`src/daad_condacts.c`](../src/daad_condacts.c)
- [x] **V3-08**: Implementar `do_SETAT()` en [`src/daad_condacts.c`](../src/daad_condacts.c) (zona de atributos)

### Fase 3 — Modificaciones de condactos existentes

- [x] **V3-04**: Modificar `_internal_hasat()` (L581-589) con `baseFlag` e `ISV3`
- [x] **V3-05**: Modificar `do_DOALL()` (L2198) y `_internal_doall()` (L~2191) para bit 0 de fOFlags con `ISV3`
- [x] **V3-09**: Modificar `do_PAUSE()` (L2054-2061) con `ISV3` para comportamiento GETKEY
- [x] **EXTRA**: Corregir `_internal_listat()` (L1887-1908) para leer bit 6 de fOFlags (bugfix V2, sin `#ifdef DAADV3`)

### Fase 4 — Modificaciones del parser

- [x] **V3-02**: Modificar `parser()` (L24-93) para emitir token `UNKNOWN_WORD` con `ISV3`
- [x] **V3-03**: Modificar `populateLogicalSentence()` (L102-172) para bits 4 y 5 con `ISV3`

### Fase 5 — Verificación

- [x] Compilar con y sin `-DDAADV3` — ambos deben compilar sin warnings
- [x] Compilar con `-DDAADV3` + DDB V2 → comportamiento idéntico al actual (ISV3=false)
- [ ] Compilar con `-DDAADV3` + DDB V3 → todas las rutas nuevas activas (ISV3=true)
- [ ] Verificar que los tests existentes (PRP008/PRP009/PRP010) pasan con `DAADV3` activo
- [ ] Probar en openMSX: SETAT con operaciones 0/1/2/3, INDIR con LET/PLACE, PAUSE 0 (espera tecla)
- [ ] Probar XMES con offset de mensajes generado por DRC

---

### Fase 6 — Tests unitarios V3

#### Relación con los tests existentes

Los tests `condact1–6.com` se compilan **con** `-DDAADV3` (añadido al `CXXFLAGS` base de `unitTests/Makefile` L12). Verifican que el comportamiento V2 se preserva cuando el código V3 está compilado: `ISV3 = isV3 = false` porque `beforeEach()` establece `hdr->version = 2` e `isV3 = false`.

Para ello, en `condacts_stubs.c` `beforeEach()` se añade `hdr->version = 2;` tras el `memset(hdr, 0, ...)` existente — una línea.

#### Nuevo fichero: `tests_condacts_v3.c` → `cond_v3.com`

Un único fichero nuevo cubre todos los condactos nuevos y modificados en V3. Se compila con `-DDAADV3` y, al igual que los demás, usa `condacts_stubs.c/h` como base de stubs.

**Condactos a testear:**

| Condacto | Tests |
|----------|-------|
| `SETAT` (124) | operation=0 clear, 1=set, >1=toggle; con y sin `F_ALTFLAGS`; rango estándar vs alternativo |
| `INDIR` (122) | parchea correctamente el 2.º arg del condacto siguiente; verificar byte en buffer |
| `XMES` (120) | llama a `printXMES()` con el offset correcto (lsb + msb×256) |
| `HASAT`/`HASNAT` (58/59) | con `F_ALTFLAGS` activo: accede a flags 60-91 en lugar del rango estándar |
| `PAUSE 0` (35) | en DDB V3: espera tecla y almacena en `fKey1`; en DDB V2: comportamiento original (no testable con getTime=0, pero verificable que NO lee teclado) |
| `DOALL` bit 0 | `F_DOALLNONE` SET al inicio; CLEAR cuando objeto encontrado; queda SET si no hay objetos |

> `PAUSE 0` V2 (espera ~5s) es no testable en unit tests — `getTime()` siempre devuelve 0 (limitación conocida, igual que en PRP008). La verificación V3 sí es testable porque es keypress-based.

#### Análisis de stubs: cambios mínimos en `condacts_stubs.c/h`

**No se necesita ningún fichero de stubs nuevo.** Los cambios son mínimos:

**`unitTests/src/condacts_stubs.c`** — 3 líneas:
```c
// Añadir global (junto a los otros fake_*):
uint16_t fake_lastXmesPrinted;

// Actualizar stub (capturar el offset):
void printXMES(uint16_t address) { fake_lastXmesPrinted = address; }

// En beforeEach() (junto al reset de otros fakes):
fake_lastXmesPrinted = 0;
```

**`unitTests/src/condacts_stubs.h`** — 1 línea:
```c
extern uint16_t fake_lastXmesPrinted;
```

Los stubs de teclado (`checkKeyboardBuffer`, `getKeyInBuffer`, `fake_keyPressed`) ya son suficientes para `PAUSE 0` V3.

> **Nota sobre INDIR**: el buffer `proc[]` pasado a `do_action()` debe ser **no-const** (`char proc[]`, no `static const char proc[]`) porque INDIR escribe en él. Los demás condact tests pueden seguir usando const.

#### Cambio en `unitTests/Makefile`

Añadir `-DDAADV3` al `CXXFLAGS` base (L12) para que **todos** los tests lo incluyan:

```makefile
CXXFLAGS := -DUNIT_TESTS -DLANG_ES -DMSXDOS1 -DMSX2 -DDAADV3
```

Añadir `cond_v3.com` como variable **separada** (`V3_PROGRAMS`) — no en `CONDACTS_PROGRAMS`, porque `cond_v3.com` no encaja con el patrón `condact%.com` de la regla genérica:

```makefile
V3_PROGRAMS = cond_v3.com
PROGRAMS = dos.com $(CONDACTS_PROGRAMS) $(V3_PROGRAMS) $(DAAD_PROGRAMS)

$(OBJDIR)cond_v3.com: $(CONDACTS_BASE_LIBS) $(SRCDIR)tests_condacts_v3.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $^ -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
	@cp $@ dsk/
```

La distinción V2/V3 se hace mediante `hdr->version`:
- Tests existentes (`condact1–6`): `beforeEach()` establece `hdr->version = 2` e `isV3 = false` → `ISV3 = false` → rutas V2
- Tests nuevos (`cond_v3`): `setV3()` establece `hdr->version = 3` e `isV3 = true` → `ISV3 = true` → rutas V3

---

## 6. Tabla de impacto por fichero

### `include/daad.h`
| Línea | Cambio |
|-------|--------|
| ~82 | `extern bool isV3; #define ISV3 isV3` bajo `#ifdef DAADV3` |
| ~52 | Añadir bloque `#ifdef DAADV3` con 5 constantes `HAS_*` y 5 constantes `F53_*` |
| ~184 | Añadir `UNKNOWN_WORD = 7` al enum `VOC_TYPE` bajo `#ifdef DAADV3` |

### `include/daad_condacts.h`
| Línea | Cambio |
|-------|--------|
| ~141 | Añadir `#ifdef DAADV3 void do_XMES(); void do_INDIR(); void do_SETAT(); #endif` |

### `src/daad/daad_objects.c`
| Línea | Cambio |
|-------|--------|
| 53-54 | `referencedObject()`: escribir byte completo en flags[56/57] sin bit-masking (EXTRA-02) |

### `src/daad/daad_global_vars.c`
| Línea | Cambio |
|-------|--------|
| ~32 | Añadir `#ifdef DAADV3 bool isV3; #endif` junto a los demás globales |

### `src/daad/daad_init.c`
| Línea | Cambio |
|-------|--------|
| ~62 | Inicializar `isV3 = (hdr->version == 3)` antes del chequeo de versión; sustituir `hdr->version != 3` por `!isV3` en ese chequeo |
| 107 | `initFlags()`: añadir `memset(flags, 0, 256)` al inicio (EXTRA-03) |

### `src/daad_condacts.c`
| Línea | Cambio |
|-------|--------|
| 51 | `condactList[]`: slots 120/122/124 con guards `#ifdef DAADV3` → do_XMES/do_INDIR/do_SETAT |
| 79 | `CONDACTS[]` VERBOSE: actualizar nombres/args de slots 120/122/124 |
| ~585 | `_internal_hasat()`: `baseFlag` con `ISV3 &&` (V3-04) |
| ~598 | Nueva función `do_SETAT()` con `ISV3` guard y `baseFlag` (V3-08) |
| ~900 | No tocar `do_PLACE()` — INDIR parchea el DDB antes de que se ejecute |
| ~1887 | `_internal_listat()`: leer bit 6 de fOFlags para modo continuo vs newline (EXTRA bugfix V2, sin `#ifdef DAADV3`) |
| ~2345 | Nueva función `do_XMES()` junto a `do_EXTERN` — usa `printXMES()` del API de plataforma (V3-06) |
| ~2056 | `do_PAUSE()`: rama `#ifdef DAADV3 if (ISV3 && !value)` (V3-09) |
| ~2191 | `_internal_doall()`: `if (ISV3) flags &= ~F53_DOALLNONE` cuando objeto encontrado (V3-05) |
| ~2199 | `do_DOALL()`: `if (ISV3) flags |= F53_DOALLNONE` al iniciar (V3-05) |
| ~2204 | Nueva función `do_INDIR()` con `if (!ISV3) skip` (V3-07) |
| 208-211 | `processPROC()`: rama `verb==0` con DONE implícito cuando `condactDOALL` activo (EXTRA-04) |

### `src/daad/daad_parser_sentences.c`
| Línea | Cambio |
|-------|--------|
| ~26 | `parser()`: `#ifdef DAADV3 bool verbSeen = false; #endif` |
| ~64-76 | `parser()`: emitir token `UNKNOWN_WORD` cuando `ISV3 && verbSeen` (V3-02) |
| ~111 | `populateLogicalSentence()`: `if (ISV3) flags &= ~(F53_UNRECWRD|F53_PREPFIRST)` (V3-02/03) |
| ~118 | `populateLogicalSentence()`: caso `UNKNOWN_WORD` → `flags |= F53_UNRECWRD` (V3-02) |
| ~138 | `populateLogicalSentence()`: `if (ISV3 && fNoun1==NULLWORD) flags |= F53_PREPFIRST` (V3-03) |

### `unitTests/Makefile`
| Línea | Cambio |
|-------|--------|
| 12 | Añadir `-DDAADV3` al `CXXFLAGS` base |
| ~39 | Nueva variable `V3_PROGRAMS = cond_v3.com` (separada de `CONDACTS_PROGRAMS` para evitar conflicto con patrón `condact%.com`) |
| ~40 | Añadir `$(V3_PROGRAMS)` a `PROGRAMS` |
| ~65 | Nueva regla explícita para `cond_v3.com` |

### `unitTests/src/condacts_stubs.c`
| Línea | Cambio |
|-------|--------|
| ~20 | Añadir `uint16_t fake_lastXmesPrinted;` |
| ~104 | Actualizar `printXMES()` para capturar: `{ fake_lastXmesPrinted = address; }` |
| ~153 | Añadir `hdr->version = 2;` en `beforeEach()` (tras el `memset(hdr, 0, ...)`) |
| ~165 | Añadir `fake_lastXmesPrinted = 0;` en `beforeEach()` |

### `unitTests/src/condacts_stubs.h`
| Línea | Cambio |
|-------|--------|
| (extern) | Declarar `extern uint16_t fake_lastXmesPrinted;` |

### `unitTests/src/tests_daad_objects.c`
| Línea | Cambio |
|-------|--------|
| 154-155 | `test_referencedObject_sets_all_flags()`: asserts de `fCOCon`/`fCOWR` — de `& 0x80` a `ASSERT_EQUAL` byte completo (EXTRA-02) |

### `unitTests/src/tests_condacts5.c`
| Línea | Cambio |
|-------|--------|
| (tests) | Añadir `test_LISTOBJ_contlist_off()` y `test_LISTOBJ_contlist_on()` (EXTRA-01 bugfix V2 bit 6) |
| (main) | Añadir llamadas a los 2 tests nuevos junto a los LISTOBJ existentes |

### `unitTests/src/tests_condacts_v3.c` *(nuevo)*
Fichero nuevo con tests para SETAT, INDIR, XMES, HASAT+F_ALTFLAGS, PAUSE 0 V3, DOALL bit 0.

---

## 7. Riesgos y notas

| Riesgo | Descripción | Mitigación |
|--------|-------------|------------|
| DDB V2 con `-DDAADV3` | `do_XMES`/`do_INDIR`/`do_SETAT` apuntan a funciones reales | Cada función comprueba `if (!ISV3) { skip args; return; }` como salvaguarda |
| INDIR parchea el DDB en RAM | Escritura directa sobre el DDB cargado | DDB de MSX2/DOS está en RAM escribible; parcheo idempotente (INDIR siempre reescribe antes de usar) |
| `_internal_hasat` flags alternativas con valores >31 | Si `value>>3 > 31`, `flags[baseFlag - n]` con n>31 → fuera rango 60-91 | Responsabilidad del DSF del juego; mismo comportamiento que PCDAAD (sin bounds check) |
| `do_SETAT` modo alternativo escribe flags 60-63 | fKey1/fKey2 (60/61), fScMode (62), fCurWin (63) accesibles | Ídem anterior; autor debe evitar values que afecten flags de sistema |
| `UNKNOWN_WORD` en lsBuffer en modo V2 | Si `ISV3` es falso, el marcador nunca se emite | Condicionado por `if (ISV3 && verbSeen)` en `parser()` |
| enum `VOC_TYPE` con valor 7 | Vocabularios existentes nunca generan tipo 7; sólo `parser()` lo emite | Marcador sintético, nunca en vocabulario real |

---

## 8. Referencias

- [`docs/DAAD_Manual_1991.md`](../docs/DAAD_Manual_1991.md) — **Manual original DAAD 1991** (Tim Gilberts, Infinite Imaginations): especificación canónica. Referencias usadas:
  - `L699-706` — Sección "Initialise": "All flags and object positions are cleared" (justifica EXTRA-03)
  - `L1063-1064` — Tabla símbolos HASAT: WEARABLE/CONTAINER en bit 7 de flags 56/57 (justifica EXTRA-02)
  - `L1923-1950` — DOALL: 4 pasos del mecanismo, paso 4 "return to DOALL as from step 1" (justifica EXTRA-04)
  - `L2882-2886` — Flag 53 bit 6: "Set this to cause continuous object listing" (justifica EXTRA-01)
  - `L2891-2892` — Flag 56/57: "is 128 if… container/wearable" (justifica EXTRA-02)
- [`docs/DAAD_Ready_Documentation_V2.md`](../docs/DAAD_Ready_Documentation_V2.md) — Manual DAAD Ready V2: confirmación paralela de `L1431-1434` (DOALL paso 4), `L1942` (flag 53 bit 6), `L1945-1946` (flags 56/57 = 128)
- [`assets/V3/DAAD_V3.txt`](../assets/V3/DAAD_V3.txt) — Especificación DAAD V3 original (Tim Gilberts)
- [`assets/V3/DAAD_V3_full.md`](../assets/V3/DAAD_V3_full.md) — Especificación expandida con tablas HASAT, bit 6, encoding XMES/INDIR
- [PCDAAD — `condacts.pas`](https://github.com/Utodev/PCDAAD) — Implementación de referencia: `_INDIR` (L2176), `_XMES` (L373), `_SETAT` (L2186), `_HASAT` (L1207), `_DOALL` (L1444), parser bits 4/5 (L495-572)
- [`src/daad_condacts.c`](../src/daad_condacts.c) — Implementación de condactos (fichero principal)
- [`include/daad.h`](../include/daad.h) — Definiciones de constantes, structs, flags del sistema; `DDB_Header.version` (L90)
- [`include/daad_condacts.h`](../include/daad_condacts.h) — Declaraciones de funciones de condactos
- [`src/daad/daad_parser_sentences.c`](../src/daad/daad_parser_sentences.c) — Parser y lógica de frases
- [DAAD Condacts Reference](.agents/skills/daad-system/references/condacts.md) — Tabla completa de condactos (0-127)
- [DAAD Flags & Objects](.agents/skills/daad-system/references/flags-and-objects.md) — Flags del sistema y estructura de objetos
- [MALUVA Extensions](.agents/skills/daad-system/references/maluva-extensions.md) — Contexto de XMES/XUNDONE en MALUVA
