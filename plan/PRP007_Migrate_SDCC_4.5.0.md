# PRP007 — Migración de SDCC 4.1.0 a SDCC 4.5.0

- **ID**: PRP007
- **Tipo**: Toolchain upgrade
- **Ficheros afectados**: [Makefile](../Makefile), [include/](../include/), [src/libs/dos_*.c](../src/libs/), [src/daad_platform_msx2.c](../src/daad_platform_msx2.c), [src/libs/*.s](../src/libs/)
- **Severidad**: Migración planificada — cambio de convención de llamada por defecto rompe funciones *naked* si no se trata
- **Fecha**: 2026-04-13 / 2026-04-14
- **Estado**: Completado ✅

> **Nota**: El plan inicial apuntaba a SDCC 4.2.0 como primer paso. Se descartó porque 4.2.0 presentaba un bug de runtime no relacionado con calling convention (cuelgue en `initDAAD`). Se saltó directamente a **SDCC 4.5.0**, que resuelve los bugs de 4.1.0 y no presenta el problema de 4.2.0.

---

## 1. Resumen

SDCC 4.2.0 introdujo un cambio mayor: la convención de llamada por defecto del puerto Z80 pasó de `__sdcccall(0)` (parámetros siempre por *stack*) a `__sdcccall(1)` (parámetros en registros). SDCC 4.5.0 mantiene esa misma convención.

El salto 4.1.0 → 4.5.0 requirió:

1. **Funciones C regulares**: el compilador las recompila automáticamente con la nueva convención. Sin acción manual.
2. **Funciones `__naked` con bloque `__asm`** que leían parámetros del *stack* mediante `ix+n`, `pop`/`push`, o similar: **reescritas** para usar sdcccall(1) nativamente, o marcadas con `SDCC_STACKCALL` donde la estructura asm lo impedía.
3. **Eliminación de MSX-DOS2**: todos los `#ifdef MSXDOS2` eliminados. Solo se mantiene la API FCB/MSX-DOS1, reduciendo el alcance de la reescritura a la mitad.
4. **Eliminación del workaround de `populateLogicalSentence`**: el bug de biblioteca de SDCC 4.1.0 (Bug #3569) está resuelto en 4.5.0; la función se reintegró en [src/daad/daad_parser_sentences.c](../src/daad/daad_parser_sentences.c) y se borró el fichero `.c` independiente.

## 2. Convenciones de llamada relevantes

### 2.1. `__sdcccall(0)` (default en 4.1.0)

| Tamaño  | Parámetros | Retorno |
|---------|-------|-------|
| 8 bits  | Stack | L     |
| 16 bits | Stack | HL    |
| 32 bits | Stack | DE-HL |

### 2.2. `__sdcccall(1)` (default en 4.2.0+, confirmado en 4.5.0)

| Tamaño  | Parámetros (1er/2º) | Retorno |
|---------|-------   |-------|
| 8 bits  | A        | A     |
| 16 bits | HL       | DE    |
| 32 bits | HL-DE    | HL-DE |
| 8+8     | A+L      | —     |
| 8+16    | A+DE     | —     |
| 16+8    | HL+stack | —     |
| 16+16   | HL+DE    | —     |

> **Importante (verificado en 4.5.0)**: el retorno 16-bit es **DE** (D=high, E=low), no HL como recogen algunos documentos. Confirmado leyendo el `.asm` generado por el compilador.

Layout de `uint32_t` en sdcccall(1): E=bits7-0, D=bits15-8, L=bits23-16, H=bits31-24.

### 2.3. `__z88dk_fastcall` (inalterada entre versiones)

| Tamaño  | Parámetros | Retorno |
|---------|-------|-------|
| 8 bits  | L     | L     |
| 16 bits | HL    | HL    |
| 32 bits | DE-HL | DE-HL |

Solo admite **un** parámetro.

## 3. Estrategia adoptada

Se eligió **reescritura completa en sdcccall(1)** en lugar de "pin ABI con SDCC_STACKCALL". Razones:

- La eliminación de DOS2 redujo las funciones a reescribir de ~22 a ~12.
- El upstream `sdcc_msxdos` ya tenía las funciones reescritas como referencia.
- Código resultante más compacto y eficiente (~20 T-states menos por llamada típica).

`SDCC_STACKCALL` (= `__sdcccall(0)`) se reservó para las pocas funciones de utilidad cuyo ensamblador usa `push ix / ld ix,#4 / add ix,sp` y cuya reescritura no era prioritaria: `initializeMapper`, `setMapperPage2`, `restoreMapperPage2`, `unRLE`, `unRLE_vram`, `pletter2vram`, `dzx7vram`, `copyToVRAM`, `setColor`, `bitBlt`, `fastVCopy`.

### Macro de compatibilidad

Nueva cabecera [include/sdcc_compat.h](../include/sdcc_compat.h):

```c
#define SDCC_STACKCALL __sdcccall(0)
```

Incluida desde [include/dos.h](../include/dos.h), [include/vdp.h](../include/vdp.h), [include/utils.h](../include/utils.h) y [include/daad_platform_api.h](../include/daad_platform_api.h).

## 4. Inventario de cambios

### 4.1. Makefile

- `SDCC_VER 4.1.0 → 4.5.0`
- `HEX2BIN` ahora usa Docker (igual que el compilador)
- Eliminada la referencia a `daad_populateLogicalSentence.rel` del build

### 4.2. Funciones DOS eliminadas (DOS2-only o sin callers)

Los ficheros siguientes fueron **borrados**:

| Fichero | Motivo |
|---|---|
| `dos_exit.c` | Solo útil con DOS2; DOS1 redirige a `exit0` |
| `dos_explain.c` | DOS2 only |
| `dos_get_current_directory.c` | DOS2 only |
| `dos_get_current_drive.c` | DOS2 only |
| `dos_get_drive_params.c` | DOS2 only |
| `dos_get_env.c` | DOS2 only |
| `dos_parse_pathname.c` | DOS2 only |
| `dos_rw_abs_sector_drv.c` | DOS2 only |
| `dos_set_transfer_address.c` | DOS2 only |

También se actualizaron las declaraciones en [include/dos.h](../include/dos.h) eliminando estos prototipos.

### 4.3. Funciones DOS reescritas en sdcccall(1)

Todas las funciones que siguen se reescribieron eliminando el bloque `push ix / ld ix,#4 / add ix,sp` y leyendo los parámetros directamente de los registros. Se eliminaron todos los `#ifdef MSXDOS2`.

| Fichero | Firma nueva | Convención |
|---|---|---|
| `dos_fopen.c` | `uint16_t fopen(char *fn)` | sdcccall(1): fn→HL |
| `dos_fcreate.c` | `uint16_t fcreate(char *fn)` | sdcccall(1): fn→HL |
| `dos_fclose.c` | `uint8_t fclose()` | sin params |
| `dos_fread.c` | `uint16_t fread(char *buf, uint16_t size)` | buf→HL, size→DE |
| `dos_fwrite.c` | `uint16_t fwrite(char *buf, uint16_t size)` | buf→HL, size→DE |
| `dos_fgets.c` | `char* fgets(char *str, uint16_t size)` | str→HL, size→DE |
| `dos_fputs.c` | `uint16_t fputs(char *str)` | str→HL |
| `dos_fseek.c` | `uint8_t fseek(uint32_t offset, char origin)` | convertida a C regular (no naked) |
| `dos_fileexists.c` | `uint8_t fileexists(char *fn)` | fn→HL |
| `dos_filesize.c` | `uint32_t filesize(char *fn)` | fn→HL |
| `dos_findfirst.c` | `uint8_t findfirst(char *fn, uint8_t attr)` | fn→HL, attr→A (pero SDCC_STACKCALL por estructura asm) |
| `dos_findnext.c` | `uint8_t findnext()` | sin params |
| `dos_remove.c` | `uint8_t remove(char *fn)` | fn→HL |
| `dos_dosver.c` | `uint8_t dosver()` | usa `call 0xF37D` (BDOS directo) |
| `dos_kbhit.c` | `uint8_t kbhit()` | retorno A directamente del DOSCALL |
| `dos_putchar.c` | `void putchar(char c)` | c→A (sdcccall(1)) |
| `dos_getchar.c` | `char getchar()` | retorno A |
| `dos_exit0.c` | `void exit0()` | sin params |
| `dos_get_screen_size.c` | `uint16_t get_screen_size()` | sin params |

### 4.4. Funciones de plataforma (`daad_platform_msx2.c`)

| Función | Cambio |
|---|---|
| `loadFile` | Llamadas a `fopen`/`fread`/`fclose` con nuevas firmas |
| `printXMES` | Llamadas actualizadas |
| `gfxPictureShow` | Llamadas actualizadas |
| `sfxWriteRegister(uint8_t reg, uint8_t val)` | reg→A, val→L (sdcccall(1)); eliminado `pop af/pop bc` |
| `sfxTone(uint8_t tone, uint8_t duration)` | tone→A (índice tabla), duration→L (contador espera) |
| `getTime()` | Simplificado a `return varJIFFY` (función C regular, elimina `__naked`) |

### 4.5. Ficheros ensamblador (`src/libs/*.s`)

Actualizados para sdcccall(1) en los puntos de entrada C (`_foo::`):

| Fichero | Cambio |
|---|---|
| `vdp_setvdp_read.s` | `_setVDP_Write::` y `_setVDP_Read::`: `ld a,e` + `ex de,hl` para conversión correcta del uint32_t |
| `vdp_clearscreen5.s` | `push hl/push de` antes de `.clearsc5_ldir`, `pop de/pop hl` después, para preservar params |
| `vdp_clearscreen7.s` | Ídem para SC7 |
| `vdp_setborder.s` | `uint8_t border→A`: sin cambio de lógica |
| `vdp_setcolorpal.s` | `colIndex→A`, `grb→DE`: correcto en sdcccall(1) |
| `vdp_setregvdp.s` | `reg→A→ld c,a`, `value→L→ld b,l`: correcto |
| `vdp_setpalette.s` | `char *paletteAddress→HL`: correcto |
| `vdp_setvpage.s` | `uint8_t page→A→sla a`: correcto |
| `utils_pletter2ram.s` | Eliminado bloque `push ix / ld ix,#4`; HL=datain, DE=dataout ya en registros |

### 4.6. Workaround `populateLogicalSentence` eliminado

- [src/daad_populateLogicalSentence.c](../src/daad_populateLogicalSentence.c) **borrado**.
- Función `populateLogicalSentence` reintegrada en [src/daad/daad_parser_sentences.c](../src/daad/daad_parser_sentences.c).
- El bug SDCC #3569 (fallo de enlace de librería) está resuelto en 4.5.0.

### 4.7. Condacts (`daad_condacts.c`)

Actualizadas todas las llamadas a funciones de fichero (`fopen`, `fclose`, `fread`, `fwrite`, `fcreate`) para usar las nuevas firmas sin el parámetro `fp`.

## 5. Bugs encontrados y corregidos durante la revisión

### Bug 1 — `sfxTone`: parámetros intercambiados

**Fichero**: [src/daad_platform_msx2.c](../src/daad_platform_msx2.c)

La firma original `sfxTone(uint8_t duration, uint8_t tone)` con sdcccall(0) asignaba `duration` al primer argumento del caller. Al migrar, el ensamblador interno usaba A como índice de tabla (= tono) y L como contador de espera (= duración), que coincide con la nueva convención sdcccall(1) **si la firma es `sfxTone(uint8_t tone, uint8_t duration)`**. La firma se corrigió para que los nombres matcheen el uso real.

Call site: `sfxTone(getValueOrIndirection(), *pPROC++)` — comportamiento idéntico al original.

### Bug 2 — `kbhit`: siempre retornaba 0

**Fichero**: [src/libs/dos_kbhit.c](../src/libs/dos_kbhit.c)

Código anterior: tras el `DOSCALL`, hacía `xor a` (para poner H=0) destruyendo el resultado en A. La función retornaba siempre 0. El tipo de retorno era `int` cuando debería ser `uint8_t`.

Corrección: eliminado el `xor a`; retorno tipo `uint8_t`; A del DOSCALL se propaga directamente.

### Bug 3 — `write_byte` en `utils_dzx7bvram.c`: registro incorrecto

**Fichero**: [src/libs/utils_dzx7bvram.c](../src/libs/utils_dzx7bvram.c)

```c
void write_byte(int value) __naked {
    __asm
        out (0x98), a   // ← incorrecto: int (16-bit) va en HL; byte bajo = L
    __endasm;
}
```

En sdcccall(1) el primer parámetro `int` (16 bits) va en HL; el byte bajo es L. Corregido a `ld a,l ; out (0x98), a`.

### Bug 4 — `vdp_setvdp_read.s`: conversión de uint32_t incorrecta

**Fichero**: [src/libs/vdp_setvdp_read.s](../src/libs/vdp_setvdp_read.s)

Código anterior usaba `ld a, l` para extraer el byte alto de la dirección VRAM de la convención sdcccall(1). El layout correcto de `uint32_t` en sdcccall(1) es E=bits7-0, D=bits15-8, L=bits23-16, H=bits31-24. La convención interna de `setVDP_Write::` espera A=bits7-0, y tras la rotación necesita D=bits31-24, E=bits23-16.

`ld a, l` daba A=bits23-16 (incorrecto); D y E quedaban sin reordenar.

Corrección: `ld a, e` + `ex de, hl` (E→A=bits7-0; DE pasa a ser H,L = bits31-24, bits23-16).

### Bug 5 — `vdp_clearscreen5.s` / `vdp_clearscreen7.s`: HL/DE destruidos antes de uso

**Ficheros**: [src/libs/vdp_clearscreen5.s](../src/libs/vdp_clearscreen5.s), [src/libs/vdp_clearscreen7.s](../src/libs/vdp_clearscreen7.s)

`_clearSC5lines::` recibe `startline→HL`, `numlines→DE` pero llamaba a `.clearsc5_ldir` (que hace `ldir` destruyendo HL y DE) antes de guardar los valores con `ld (BLT_DY), hl` / `ld (BLT_NY), de`.

Corrección: `push hl / push de` antes del `call .clearsc5_ldir`, `pop de / pop hl` después.

## 6. Estado final

### 6.1. Lo completado

- [x] **SDCC 4.5.0**: `Makefile` actualizado, imagen Docker `nataliapc/sdcc:4.5.0` en uso.
- [x] **Macro `SDCC_STACKCALL`**: creada en [include/sdcc_compat.h](../include/sdcc_compat.h), incluida en todas las cabeceras relevantes.
- [x] **DOS2 eliminado**: 9 ficheros borrados, `#ifdef MSXDOS2` eliminados del resto.
- [x] **Funciones DOS reescritas en sdcccall(1)**: ~19 ficheros.
- [x] **Funciones de plataforma actualizadas**: `loadFile`, `printXMES`, `gfxPictureShow`, `sfxWriteRegister`, `sfxTone`, `getTime`.
- [x] **Ficheros asm actualizados**: 10 ficheros `.s` revisados y corregidos donde era necesario.
- [x] **Workaround `populateLogicalSentence`**: eliminado; función reintegrada en la librería.
- [x] **5 bugs encontrados y corregidos** durante la revisión (ver §5).
- [x] **Condacts actualizados** con las nuevas firmas de las funciones de fichero.

### 6.2. Tamaño del binario

| Versión | Bytes `.com` |
|---|---|
| SDCC 4.1.0 (baseline) | 19775 |
| SDCC 4.5.0 (este PRP) | 18260 (-1515 bytes, –7.7%) |

### 6.3. Verificación pendiente

- [x] Build limpio: `make clean && make` sin errores ni warnings nuevos.
- [x] Test suite: 212 OK / 50 TODO / 0 FAIL — sin regresiones.
- [x] Smoke en openMSX: arrancar `dsk/` y llegar al primer prompt "¿Qué quieres hacer?".
- [x] Test parser: `COGER X` / `IR SUR` con respuesta coherente (ejercita `populateLogicalSentence` en su nueva ubicación).

## 7. Referencias

- [SDCC 4.2.0 Release Notes](https://sdcc.sourceforge.net/ChangeLog.txt) — cambio de convención por defecto a sdcccall(1).
- [SDCC Bug #3166 — lospre con static vars](https://sourceforge.net/p/sdcc/bugs/3166/) — resuelto en 4.x.
- [SDCC Bug #3569 — library link fallido](https://sourceforge.net/p/sdcc/bugs/3569/) — resuelto en 4.5.0; permite eliminar el workaround.
- [REPORT_DOS.md](../REPORT_DOS.md) — diferencias entre nuestro `src/libs/dos_*` y el repo upstream `sdcc_msxdos`; referencia para la reescritura sdcccall(1).
- [include/sdcc_compat.h](../include/sdcc_compat.h) — macro `SDCC_STACKCALL`.
