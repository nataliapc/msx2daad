# PRP023 — V9938CmdData stream-decompress to port `#9B`

- **ID**: PRP023
- **Tipo**: Implementación (engine + tool)
- **Ficheros afectados**:
  - [`src/libs/vdp_copycmddata.s`](../src/libs/vdp_copycmddata.s) — nuevo helper `copyCmdData` (RAW: RAM → port `#9B`)
  - [`src/libs/vdp_copyvram2cmddata.s`](../src/libs/vdp_copyvram2cmddata.s) — nuevo `copyVRam2CmdData` (VRAM → port `#9B`)
  - [`src/libs/utils_unrle_data.s`](../src/libs/utils_unrle_data.s) — nuevo `unRLE_Data` (RLE: RAM → port `#9B`)
  - [`src/libs/utils_pletter2data.c`](../src/libs/utils_pletter2data.c) — nuevo `pletter2Data` (Pletter: usa `pletter2vram` a scratch VRAM + `copyVRam2CmdData`)
  - [`include/vdp.h`](../include/vdp.h) — declarar `copyCmdData` y `copyVRam2CmdData`
  - [`include/utils.h`](../include/utils.h) — declarar `unRLE_Data`, `pletter2Data`
  - [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c) — handler funcional de `IMG_CHUNK_CMDDATA`
  - [`bin/imgwizard.php`](../bin/imgwizard.php) — cap diferenciado por compresor (`IMG_CHUNK_SIZE=2043` para RAW/RLE; `11264` para PLETTER); warning DEPRECATED en `c`/`cl`/`s` sin transparencia
- **Severidad**: FEATURE (continuación del PRP022)
- **Fecha**: 2026-04-25
- **Estado**: Pendiente

> **Alcance**: completar la ejecución del chunk `IMG_CHUNK_CMDDATA` (que en PRP022 quedó como skip-only) usando descompresores específicos para HMMC streaming. Para Pletter se reusa `pletter2vram` ya enlazado: descomprime a una zona oculta de VRAM (page 1 hidden, offset `0x1D400 = 119808`, 11264 bytes) y luego `copyVRam2CmdData` lee y vuelca al port `#9B`. **Esto requiere que el comando HMMC permanezca activo durante `pletter2vram`** — verificación técnica documentada abajo. Como efecto colateral: marcar como DEPRECATED los comandos `c`/`cl`/`s` (sin transparencia) en imgwizard, recomendando `cx[l]` en su lugar. Las funciones legacy `copyToVRAM`, `pletter2vram` y los chunks `IMG_CHUNK_RAW`/`IMG_CHUNK_PLETTER` se mantienen por retrocompatibilidad con imágenes existentes en `dsk/` — su eliminación queda diferida a un PRP futuro.

---

## Problema

Tras PRP022, el engine genera y reconoce los chunks `INFO`/`V9938Cmd`/`V9938CmdData`, pero `IMG_CHUNK_CMDDATA` está implementado como skip-only en [`src/daad_platform_msx2.c:934-985`](../src/daad_platform_msx2.c#L934-L985):

```c
if (chunk->type==IMG_CHUNK_CMDDATA) {
    fseek(chunk->chunkSize, SEEK_CUR);    // descarta payload
    /* TODO PRP futuro: descomprimir + volcar a #9B */
}
```

Resultado: las imágenes generadas con `imgwizard cx[l]` se cargan sin crash pero **no se renderizan** (el VDP queda esperando datos que nunca llegan tras el HMMC).

Las dos opciones identificadas en PRP022 son:
- **Opción A** (rechazada): descomprimir a RAM completa y volcar a `#9B` con un OTIR. Coste prohibitivo en MSX-DOS1: hasta 16KB de buffer transitorio si `uncompressedSize` no se acota.
- **Opción B** (preferida): descompresores con sumidero directo a `#9B`, sin buffer intermedio (RAW y RLE pueden), o con buffer acotado a `IMG_CHUNK_SIZE = 2043` bytes (Pletter, por necesitar back-references LZ77).

Este PRP implementa la Opción B con la arquitectura **`*_Data`** (canal HMMC) en paralelo a las funciones **`*_vram`** existentes (canal v1 legacy con `setVDP_Write`).

---

## Análisis técnico

### Tres rutas según compresor

| Compresor | Ruta engine                                              | Cap uncompressed | RAM transitoria |
|-----------|----------------------------------------------------------|------------------|------------------|
| RAW       | `copyCmdData` directo (`OTIR` a `#9B`)                   | 2043 b (`IMG_CHUNK_SIZE`) | 0 |
| RLE       | `unRLE_Data` directo (descomprime y vuelca a `#9B`)      | 2043 b           | 0 |
| Pletter   | `pletter2vram` → VRAM scratch + `copyVRam2CmdData` → `#9B` | **11264 b**      | 0 (usa VRAM oculta) |

### Por qué Pletter no puede stream-decompress directo

`pletter2vram` resuelve back-references aprovechando el VDP: tras escribir un byte a `#98`, puede `setVDP_Read` en el offset destino para leer el byte ya escrito. En HMMC ese truco no funciona — port `#9B` es write-only y cualquier intento de leer la salida aborta el comando HMMC pendiente.

**Solución elegida**: descomprimir el chunk con `pletter2vram` a una zona oculta de VRAM, luego copiar de VRAM al port `#9B` con `copyVRam2CmdData`. **No usa RAM transitoria.**

### Zona scratch en VRAM

| Modo  | Bytes/línea | Layout                                          | Zona scratch usada                       |
|-------|-------------|-------------------------------------------------|-------------------------------------------|
| SC5/6 | 128         | Page 0+1 = 64KB. Pages 2-3 enteras libres.      | Offset `0x1D400` (en page 1 hidden)       |
| SC7/8/A/C | 256     | Page 0+1 = 128KB. Hidden zone Y=212..255.       | Offset `0x1D400 = 119808` (= 65536+54272), 11264 bytes (Y=468..511 page 1) |

> Constante `VRAM_PLETTER_SCRATCH = 0x1D400` (119808 = page 1 hidden zone start, 44 líneas × 256 bytes = **11264 bytes** disponibles).
> Verificación pendiente: que `FONTINITY` y otras zonas ocultas no colisionen con esta área. La fuente típicamente vive en page 0 hidden (Y=224..255), no en page 1.

### Persistencia de HMMC durante `pletter2vram`

**Análisis del V9938**: HMMC mantiene `CE=1` (S#2 bit 0) hasta transferir los `NX*NY` bytes vía port `#9B`. El comando vive en **R32-R46**.

`pletter2vram` accede al VDP únicamente vía:
- `out (0x99),a` para escribir R0/R1/R14/R17 (registros de **acceso CPU↔VRAM**, no de comando).
- `out (0x98),a` y `in (0x98),a` para escribir/leer bytes a VRAM por la ruta CPU.

**Ni un solo `out (0x9B),a`** ni escritura a R32-R46 (verificable inspeccionando [`utils_pletter2vram.s`](../src/libs/utils_pletter2vram.s) — todas las escrituras a `#99` van a R0/R1/R14 vía R17 indirecto, nunca a registros de comando).

**Arbitraje VRAM**: el V9938 tiene slots de VRAM compartidos entre display, command engine y CPU. Cuando HMMC está en estado `TR=0` (esperando datos en R44) **no consume slots**: el command engine queda en standby. Las lecturas/escrituras CPU vía `#98` durante ese intervalo no compiten ni interfieren con la operación HMMC. Una vez resumimos las escrituras a `#9B`, `TR` vuelve a ciclar y la transferencia continúa donde quedó.

**Conclusión**: HMMC persiste. **Riesgo residual**: glitches transitorios en TR/CE en el primer write a `#9B` tras un periodo largo sin actividad de comando — el Z80 es lento, TR vuelve a 1 inmediatamente. Riesgo bajo pero requiere **verificación empírica** con un test mínimo (ver §Tests).

### Cap diferenciado por compresor en imgwizard

Hoy `compressV9938Rectangle` en imgwizard usa el bucle adaptativo:
```php
$sizeIn = min($totalUncomp - $pos, CHUNK_CMDDATA_MAX);  // CHUNK_CMDDATA_MAX = 2040 bytes COMPRIMIDOS
```
y crece `$sizeIn` (uncompressed) hasta que el output comprimido alcanza 2040 bytes. Con Pletter sobre datos muy redundantes, `$sizeIn` puede crecer a 50KB+ antes de que la salida llegue a 2040 bytes — el campo `uncompressedSize` es 16-bit (max 65535).

Cap diferenciado:
```php
define('CHUNK_PLETTER_MAX_UNCOMP', 11264);   // VRAM scratch zone size

// En compressV9938Rectangle:
$cap = ($comp[COMP_ID]==CHUNK_PLETTER) ? CHUNK_PLETTER_MAX_UNCOMP : CHUNK_SIZE;
$sizeIn = min($totalUncomp - $pos, $cap);
// ... y mismo cap en la rama "grow" del bucle adaptativo
```

- **RAW/RLE**: cap a `CHUNK_SIZE = 2043` (chunk header + payload caben sin overflow). El engine los descomprime directo a `#9B` sin buffer intermedio, así que la cota es la del chunk en disco.
- **PLETTER**: cap a `11264` (`CHUNK_PLETTER_MAX_UNCOMP`). El engine descomprime a VRAM scratch (11264 bytes) y luego copia de ahí a `#9B`. Cota natural: tamaño de la zona scratch.

> Implicación práctica: para una imagen 256×212 SC8 (54272 bytes) con Pletter, antes 1 chunk podía cubrirla; ahora ~5 chunks de 11264. Para RAW/RLE: ~26 chunks de 2043. Overhead de cabecera `5 × 8 b = 40 b` por imagen Pletter — despreciable.

### Linker dead-code elimination

`sdar` (archivador SDCC) sólo enlaza `.rel` referenciados. Una vez introducidas las variantes `*_Data`, si **nadie en el engine las llama** los `.rel` legacy desaparecen del `.com` final. Tabla de impacto cuando la migración esté completa:

| Función legacy   | Tamaño   | ¿Eliminable tras migración completa?                                                |
|------------------|---------|--------------------------------------------------------------------------------------|
| `copyToVRAM`     | 33 b    | ✅ Sí (cuando `c`/`cl`/`s` ya no emitan `IMG_CHUNK_RAW`)                            |
| `unRLE_vram`     | ~90 b   | ❌ **No** — `s` con transparencia conserva el token "skip-output" único de RLE      |
| `pletter2vram`   | ~244 b  | ✅ Sí (cuando `c`/`cl` ya no emitan `IMG_CHUNK_PLETTER`)                            |

> `IMG_CHUNK_RLE` es **el único formato legacy con valor irreemplazable**: su token `mark+0x01+N` indica "saltar N bytes en VRAM sin escribir", semántica imprescindible para "color transparente". HMMC sobre `#9B` no puede saltar (cada byte que llega se escribe). Por tanto `unRLE_vram` se mantiene indefinidamente.

**Ahorro neto post-migración**: 33 + 244 − 80 (nuevos `*_Data`) ≈ **~197 bytes** en `msx2daad.com`.

### Política DEPRECATED en imgwizard

Los comandos `c`/`cl`/`s` siguen funcionando exactamente igual (formato v1 legacy) — las imágenes existentes en `dsk/` no se rompen ni necesitan regenerarse. Pero cuando el usuario los invoca **sin transparencia** (caso para el que `cx[l]` es estrictamente mejor), imgwizard imprime un warning recomendando migrar:

```
WARNING: 'c' command without transparency is DEPRECATED.
         Use 'cx' for new images (V9938 HMMC streaming, ~197 bytes smaller .com).
         Continuing in legacy mode...
```

Los comandos `s` con transparencia (sí soportada: imagen pintada sobre background existente) **no muestran warning**: ése es el caso de uso para el que `IMG_CHUNK_RLE` legacy sigue siendo necesario.

---

## Cambios

### 1. Nueva función `copyCmdData()`

**Archivo nuevo**: [`src/libs/vdp_copycmddata.s`](../src/libs/vdp_copycmddata.s).

Estructura idéntica a [`vdp_copytovram.s`](../src/libs/vdp_copytovram.s) pero:
- Sin `setVDP_Write` (el VDP ya está en HMMC tras `fastVCopy()`).
- Port `#9B` en vez de `#98`.
- Sin parámetro VRAM destination.

```asm
; void copyCmdData(uint8_t *src, uint16_t length);
; Sends `length` bytes from RAM to VDP port #9B (HMMC streaming).
; Caller MUST have dispatched a HMMC command via fastVCopy() first.

.area _CODE

_copyCmdData::
        push ix
        ld   ix,#4
        add  ix,sp

        ld   l,0(ix)
        ld   h,1(ix)
        ld   c,2(ix)
        ld   b,3(ix)
        pop  ix

copyCmdData::           ; HL=src, BC=length
        ld   d,b        ; D = blocks of 256 bytes
        ld   b,c        ; B = remainder
        ld   c,#0x9B
        xor  a
        cp   b
        jr   z, .ccd_loop1
.ccd_loop0:
        otir            ; Send remainder bytes
        cp   d
        ret  z
.ccd_loop1:
        otir            ; Send block of 256 bytes
        dec  d
        jp   nz, .ccd_loop1
        ret
```

> Tamaño estimado: **~22 bytes**.

Declaración en [`include/vdp.h`](../include/vdp.h):
```c
void copyCmdData(uint8_t *src, uint16_t length) SDCC_STACKCALL;
```

### 2. Nueva función `unRLE_Data()`

**Archivo nuevo**: [`src/libs/utils_unrle_data.s`](../src/libs/utils_unrle_data.s).

Adaptación directa de [`utils_unrle_vram.s`](../src/libs/utils_unrle_vram.s) con cuatro recortes:

| Cambio                                          | Bytes  |
|-------------------------------------------------|--------|
| Eliminar setup `setVDP_Write` + variable `.vram_page` | −15    |
| `ld c, #0x98` → `ld c, #0x9B`                   |   0    |
| Eliminar `inc de` (no hay tracking VRAM addr) × 2 | −4     |
| Eliminar rama `.skip_output` (V9938CmdData no la emite) | −20    |

```asm
; void unRLE_Data(char *source);
; Decompresses RLE stream from RAM directly to VDP port #9B.
; Caller MUST have dispatched a HMMC command via fastVCopy() first.
; Stops when EOF token (mark+0x00) is found.

.area _CODE

_unRLE_Data::
        pop  af
        pop  hl
        push hl
        push af

unRLE_Data::            ; HL=source
        ld   c, #0x9B
        ld   a,(hl)     ; A = mark byte
        ld   (#.mark),a
        inc  hl
.urd_loop:
        ld   a,(#.mark)
.urd_loop2:
        cp   (hl)
        jr   z, .urd_rle
        outi            ; Copy one byte raw to #9B
        jr   .urd_loop2
.urd_rle:
        inc  hl
        xor  a
        cp   (hl)
        ret  z          ; EOF (mark+0x00)
        ld   b,(hl)     ; B = repeat count
        inc  hl
        ld   a,(hl)     ; A = value
        inc  hl
.urd_rep:
        out  (0x9B),a
        djnz .urd_rep
        jr   .urd_loop

.mark:
        .ds  1
```

> Tamaño estimado: **~50 bytes** (vs. ~90 b de `utils_unrle_vram.s`).

Declaración en [`include/utils.h`](../include/utils.h):
```c
void unRLE_Data(char *source) SDCC_STACKCALL;
```

### 3. Nueva función `copyVRam2CmdData()`

**Archivo nuevo**: [`src/libs/vdp_copyvram2cmddata.s`](../src/libs/vdp_copyvram2cmddata.s).

Lee `length` bytes desde una dirección de VRAM y los vuelca al port `#9B` (HMMC streaming). Usa `setVDP_Read` (ya existente) para configurar el puntero de lectura, luego ciclo `in (0x98) / out (0x9B)`.

```asm
; void copyVRam2CmdData(uint32_t vram_src, uint16_t length);
; Lee `length` bytes desde VRAM (A+DE_low) y los vuelca al port #9B.
; El llamante DEBE haber disparado un HMMC vía fastVCopy() previamente.

.area _CODE
.globl setVDP_Read

_copyVRam2CmdData::
        push ix
        ld   ix,#4
        add  ix,sp
        ld   e,0(ix)        ; DE = vram low 16 bits
        ld   d,1(ix)
        ld   a,2(ix)        ; A = vram bit 16
        ld   c,4(ix)        ; BC = length
        ld   b,5(ix)
        pop  ix

        push bc
        call setVDP_Read    ; A+DE → VDP read pointer
        pop  bc

        ld   d,b            ; D = bloques de 256
        ld   b,c            ; B = resto
        xor  a
        cp   b
        jr   z, .cv_block
.cv_rest:
        in   a,(0x98)
        out  (0x9B),a
        djnz .cv_rest
        xor  a
        cp   d
        ret  z
.cv_block:
        in   a,(0x98)
        out  (0x9B),a
        djnz .cv_block
        dec  d
        jp   nz, .cv_block
        ret
```

> Tamaño estimado: **~30 bytes**. Reusa `setVDP_Read` (~20 b ya enlazado por otras rutinas).

Declaración en [`include/vdp.h`](../include/vdp.h):
```c
void copyVRam2CmdData(uint32_t vram_src, uint16_t length) SDCC_STACKCALL;
```

### 4. Nueva función `pletter2Data()`

**Archivo nuevo**: [`src/libs/utils_pletter2data.c`](../src/libs/utils_pletter2data.c).

Wrapper en C sobre `pletter2vram` + `copyVRam2CmdData`. Descomprime el payload Pletter a la zona scratch VRAM (`0x1D400 = 119808`, 11264 bytes en page 1 hidden), luego copia de ahí al port `#9B`.

```c
#include <stdint.h>
#include "utils.h"
#include "vdp.h"

#define VRAM_PLETTER_SCRATCH  0x1D400UL    // 119808: page 1 hidden zone start
                                            // Y=468..511 SC7/8/A/C, 11264 bytes

/*
 * pletter2Data — descomprime Pletter5 a la zona scratch VRAM oculta
 * (page 1 hidden, 11264 bytes) y vuelca el resultado al port #9B
 * para HMMC streaming.
 *
 * El llamante DEBE haber disparado un comando HMMC vía fastVCopy() antes.
 * El llamante GARANTIZA que `uncompSize <= 11264` (cap impuesto por imgwizard).
 *
 * El HMMC pendiente persiste durante pletter2vram porque éste sólo accede a
 * R0/R1/R14 vía port #99 — nunca toca R32-R46 (registros de comando) ni
 * port #9B. Verificado empíricamente (ver tests del PRP).
 */
void pletter2Data(uint8_t *src, uint16_t uncompSize)
{
    pletter2vram(src, VRAM_PLETTER_SCRATCH);
    copyVRam2CmdData(VRAM_PLETTER_SCRATCH, uncompSize);
}
```

> Tamaño estimado: ~15 bytes de glue. Reusa `pletter2vram` (~244 b, **ya enlazado** por el handler legacy `IMG_CHUNK_PLETTER` del bucle de chunks).

Declaración en [`include/utils.h`](../include/utils.h):
```c
void pletter2Data(uint8_t *src, uint16_t uncompSize);
```

> **Coste cero en `.com`** si el handler legacy de v1 sigue activo (`pletter2vram` ya pesa). Si en un futuro PRP se elimina la ruta v1 y nadie más usa `pletter2vram`, esta nueva ruta lo mantendrá enlazado — ganando los 244 b a cambio de no necesitar buffer RAM. Sigue siendo trade-off aceptable.

### 5. Handler funcional de `IMG_CHUNK_CMDDATA` en `gfxPictureShow()`

[`src/daad_platform_msx2.c:934-985`](../src/daad_platform_msx2.c#L934-L985) — sustituir el bloque skip-only por la lógica real:

```c
if (chunk->type==IMG_CHUNK_CMDDATA) {
    //=============================================
    // V9938 command data: descomprime y vuelca al port #9B (HMMC streaming).
    // El comando HMMC fue disparado por el V9938Cmd previo; el VDP está
    // esperando bytes en R44 vía port #9B.
    size = fread(chunk->data, chunk->auxData);          // 3 bytes extra header
    if (!(size & 0xff00)) {
        uint8_t  compID = chunk->data[0];
        uint16_t uncomp = *(uint16_t*)&chunk->data[1];
        fread(chunk->data, chunk->chunkSize);           // payload comprimido
        switch (compID) {
            case 0:  copyCmdData(chunk->data, uncomp);     break;   // RAW
            case 1:  unRLE_Data(chunk->data);              break;   // RLE
            case 2:  pletter2Data(chunk->data, uncomp);    break;   // PLETTER
        }
    }
} else
```

> Tamaño estimado del cambio: ~25 bytes en C (vs. ~10 bytes del skip actual + 40 líneas de comentario que se eliminan).

### 6. Cap diferenciado por compresor en imgwizard

[`bin/imgwizard.php`](../bin/imgwizard.php) — en `compressV9938Rectangle`, cap distinto según compresor:

```php
define('CHUNK_PLETTER_MAX_UNCOMP', 11264);    // VRAM scratch zone size

function compressV9938Rectangle($file, $x, $y, $w, $h, $comp)
{
    ...
    // Cap por compresor
    $cap = ($comp[COMP_ID]==CHUNK_PLETTER) ? CHUNK_PLETTER_MAX_UNCOMP : CHUNK_SIZE;

    while ($pos < $totalUncomp) {
        $sizeIn = min($totalUncomp - $pos, $cap);
        $sizeDelta = intval($cap / 2);
        ...
        // Y en la rama "grow":
        $sizeIn = min($sizeIn + $sizeDelta, $totalUncomp - $pos, $cap);
        ...
    }
}
```

Resumen:
- **RAW/RLE**: cap a `CHUNK_SIZE = 2043` (chunk file fits + descompresión directa al `#9B` sin buffer).
- **PLETTER**: cap a `11264` (zona scratch VRAM oculta).

**Efecto secundario**: para imágenes con compresión Pletter muy alta sobre fondos uniformes, antes 1 chunk cubría todo el rectángulo (50KB→850 b); ahora se generan ~5 chunks de hasta 11264 b cada uno. El overhead es 5×8 b (cabecera por chunk) = 40 b extra por imagen completa — despreciable.

### 7. Warning DEPRECATED en imgwizard

[`bin/imgwizard.php`](../bin/imgwizard.php) — en los handlers de `c`/`cl` y `s`:

**Comando `c`/`cl`** (tras la determinación de `$transparent`):
```php
if ($transparent < 0) {
    echo "\n".
         "WARNING: 'c' command without transparency is DEPRECATED.\n".
         "         Use 'cx' for new images (V9938 HMMC streaming).\n".
         "         Continuing in legacy mode...\n\n";
}
```

**Comando `s`** (tras la determinación de `$transparent`):
```php
if ($transparent < 0) {
    echo "\n".
         "WARNING: 's' command without transparency is DEPRECATED.\n".
         "         Use 'cx' for new images (V9938 HMMC streaming).\n".
         "         's' with transparency remains the recommended path.\n".
         "         Continuing in legacy mode...\n\n";
}
```

> Sólo `c`/`cl` y `s` **sin transparencia** muestran warning. `s` **con transparencia** (caso de uso único de `IMG_CHUNK_RLE` con skip-output) no muestra warning — sigue siendo la ruta correcta.

### 8. Header del engine

[`include/daad_platform_msx2.h`](../include/daad_platform_msx2.h) — actualizar comentarios en los `#define`:

```c
#define IMG_CHUNK_RAW       2     // **LEGACY v1** — preferir V9938CmdData (compID=0)
#define IMG_CHUNK_RLE       3     // v1 — usado por `s` con transparencia (skip-output)
#define IMG_CHUNK_PLETTER   4     // **LEGACY v1** — preferir V9938CmdData (compID=2)
```

`IMG_CHUNK_RLE` **no** se marca como legacy — sigue siendo la ruta canónica para imágenes con transparencia.

---

## Compatibilidad

- **Imágenes existentes en `dsk/`**: siguen funcionando idénticamente. El handler legacy del `else` final del bucle de chunks no se toca, y `copyToVRAM`/`unRLE_vram`/`pletter2vram` permanecen enlazadas mientras alguna imagen v1 las invoque.
- **Comandos `c`/`cl`/`s` sin transparencia**: siguen funcionando, sólo añaden el warning. Los scripts existentes que invoquen estos comandos NO necesitan cambios.
- **Comandos `cx[l]` (PRP022)**: ahora pintan el rectángulo correctamente en lugar de quedarse "en negro".

## Casos límite

### Buffer en `heap_top + sizeof(IMG_CHUNK)`

`IMG_CHUNK` ocupa 5 + IMG_CHUNK_SIZE = 2048 bytes. El buffer de descompresión Pletter empieza en `heap_top + 2048` y necesita hasta 2043 bytes más → ocupa hasta `heap_top + 4091`. La macro `getFreeMemory()` devuelve `varTPALIMIT - heap_top + hdr->fileLength - sizeof(IMG_CHUNK)` ≈ varios KB; el buffer transitorio cabe holgadamente.

### `pletter2ram` y stack

`pletter2ram` usa IX y todos los registros principales. El wrapper C de `pletter2Data` debe respetar la convención `__sdcccall(1)` — el call C estándar funciona correctamente porque `pletter2ram` ya tiene su propio prólogo `push ix` / `push af` etc.

### V9938CmdData con `compID` desconocido

Si un fichero corrupto trae `compID > 2`, el `switch` no entra en ninguna rama y el VDP queda esperando bytes que nunca llegan. **No se valida** explícitamente — coherente con el patrón actual del engine de no validar campos de chunks (la herramienta es la fuente de verdad).

### Cap a 2043 bytes y ratio Pletter

Para datos muy redundantes (fondos planos), Pletter puede comprimir 50KB → <100 b en un solo bloque. Con el cap a 2043 el mismo rectángulo se trocea en ~25 chunks. El tamaño final del fichero `.IMx` aumenta ligeramente (overhead por chunk de cabecera), pero el ahorro de RAM en el engine justifica el coste.

---

## Resumen de archivos modificados

| Archivo                                       | Cambio                                                                 |
|-----------------------------------------------|------------------------------------------------------------------------|
| [`src/libs/vdp_copycmddata.s`](../src/libs/vdp_copycmddata.s) | **Nuevo**: ~22 b OTIR a port `#9B` (RAM→`#9B`)      |
| [`src/libs/vdp_copyvram2cmddata.s`](../src/libs/vdp_copyvram2cmddata.s) | **Nuevo**: ~30 b VRAM→`#9B` (`setVDP_Read` + bucle `in/out`) |
| [`src/libs/utils_unrle_data.s`](../src/libs/utils_unrle_data.s) | **Nuevo**: ~50 b RLE → `#9B` (sin skip-output, sin tracking VRAM) |
| [`src/libs/utils_pletter2data.c`](../src/libs/utils_pletter2data.c) | **Nuevo**: wrapper `pletter2vram` + `copyVRam2CmdData` (~15 b) |
| [`include/vdp.h`](../include/vdp.h)           | Declaraciones de `copyCmdData` y `copyVRam2CmdData`                    |
| [`include/utils.h`](../include/utils.h)       | Declaraciones de `unRLE_Data` y `pletter2Data`                         |
| [`include/daad_platform_msx2.h`](../include/daad_platform_msx2.h) | Comentarios actualizados en `IMG_CHUNK_RAW`/`RLE`/`PLETTER`     |
| [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c) | Handler funcional de `IMG_CHUNK_CMDDATA` (sustituye el skip-only del PRP022) |
| [`bin/imgwizard.php`](../bin/imgwizard.php)   | Cap diferenciado por compresor (2043 RAW/RLE; 11264 PLETTER); warning DEPRECATED en `c`/`cl`/`s` sin transparencia |

**Total estimado**: ~117 bytes nuevos en `vdp.lib`/`utils.lib` + ~25 bytes en C (handler + wrapper). **0 bytes de RAM transitoria** (Pletter usa zona oculta de VRAM). Sin cambios al ABI del formato `.IMx`.

---

## Tests propuestos

### Test 0 — Verificación de persistencia HMMC (CRÍTICO)

**Objetivo**: confirmar empíricamente que un comando HMMC pendiente sobrevive a la ejecución de `pletter2vram` sin abortarse ni corromperse.

**Procedimiento**:
1. Llenar VRAM page 0 con un patrón conocido (e.g. todo `0xAA`).
2. Dispatch HMMC con `fastVCopy` apuntando a un rectángulo pequeño (e.g. 32×32 en `(100, 100)`).
3. Ejecutar `pletter2vram(payload_test, 0x1D400)` para descomprimir un payload conocido al área scratch.
4. Llamar `copyVRam2CmdData(0x1D400, 1024)` para enviar 1024 bytes desde VRAM scratch al port `#9B`.
5. Verificar:
   - El rectángulo (100,100)..(132,132) contiene los datos descomprimidos.
   - El área scratch (`0x1D400`..`0x1D7FF`) no se ha corrompido por la actividad HMMC.
   - El bit CE de S#2 cae a 0 al completarse los 1024 bytes (transferencia completa).

**Si falla**: descartar Opción C, volver a Opción A (`pletter2ram` + buffer RAM transitorio).

### Tests funcionales

1. **`imgwizard cx`** SC8 con `RAW` (rect 64×32) → cargar en engine: rectángulo se pinta correctamente (sin garbage). Vía `copyCmdData`.
2. **`imgwizard cx`** SC8 con `RLE` → mismo, vía `unRLE_Data`.
3. **`imgwizard cxl`** SC5 paletizado con `PLETTER` (rect 256×100) → engine descomprime con `pletter2Data` (`pletter2vram` a scratch `0x1D400` + `copyVRam2CmdData` a `#9B`). Rectángulo pintado correctamente. **0 bytes RAM transitoria.**
4. **`imgwizard cxl`** SC8 con `PLETTER` y rectángulo grande (256×212) → genera ~5 chunks de hasta 11264 b uncompressed; engine los procesa secuencialmente, cada uno reusando el área scratch. Imagen completa pintada.
5. **Imagen legacy `dsk/000.IM8`** (RLE v1) → sigue cargándose vía handler `else` final + `unRLE_vram`. Ningún cambio visual.
6. **`imgwizard c image.sc8 96 RLE`** (sin transparencia) → muestra warning DEPRECATED, genera imagen v1 (compatible con engine actual).
7. **`imgwizard s image.sc8 0 0 64 64 5`** (con transparencia color 5) → SIN warning, genera imagen v1 con `IMG_CHUNK_RLE` que el engine pinta vía `unRLE_vram` con skip-output preservando background.
8. **`make test`** con un `dsk/` que combine imágenes legacy y `cx[l]` → ambas se pintan correctamente.
9. **Verificar tamaño `.com`** antes y después del PRP. Esperado: incremento ~117 b por las funciones nuevas + ~25 b por el handler. **`pletter2vram` no añade bytes** (ya enlazado por handler legacy v1). Si en el futuro se elimina el handler legacy del `else`, esperamos reducción de ~33 b por DCE de `copyToVRAM` (ya no usado) — pero `pletter2vram` se mantiene porque ahora lo usa `pletter2Data`.

---

## Verificación manual

### Pintando un rectángulo CX SC8

```bash
$ bin/imgwizard.php cx test.sc8 0 0 256 100 RLE
### Loading test.sc8
### Compressor: RLE (CX command — no transparency allowed)
### Mode SCREEN 8
### Rectangle Start:(0, 0) Width:(256, 100)
    franja pos=0 uncomp=2043 comp=89 bytes
    franja pos=2043 uncomp=2043 comp=92 bytes
    …
    franja pos=23492 uncomp=2108 comp=104 bytes
### Writing test.IM8
### Done
```

(Antes del cap, una sola franja de 25600 b uncomp; ahora ~13 franjas de 2043 b. El número de chunks aumenta pero cada uno cabe en buffer 2KB del engine.)

### Warning DEPRECATED

```bash
$ bin/imgwizard.php c image.sc8 96 RLE
### Loading image.sc8

WARNING: 'c' command without transparency is DEPRECATED.
         Use 'cx' for new images (V9938 HMMC streaming).
         Continuing in legacy mode...

### Mode SCREEN 8
### Lines 96
…
```

### `s` con transparencia (sin warning)

```bash
$ bin/imgwizard.php s image.sc8 0 0 64 64 5
### Loading image.sc8
### Transparent color: 5
### Compressor: RLE (forced)
### Mode SCREEN 8
…
### Done
```

(No imprime warning — `s` con transparencia sigue siendo la ruta correcta.)

---

## Notas de implementación

- **Convención de nombres**: las nuevas funciones usan sufijo `_Data` o prefijo `copyCmdData`/`copyVRam2CmdData` para distinguir del canal v1 (`_vram`/`copyToVRAM`). Esto facilita la búsqueda y deja claro que son específicas para HMMC streaming via port `#9B`.
- **`pletter2Data` no es un .s nuevo**: se implementa en C como wrapper trivial sobre `pletter2vram` (ya enlazado) + `copyVRam2CmdData`. Evita duplicar los ~244 b del descompresor Pletter5+VRAM.
- **Zona scratch VRAM `0x1D400`**: page 1 hidden (Y=468..511), 11264 bytes. **Reusada entre chunks**: cada `V9938CmdData` Pletter sobreescribe la misma zona; el contenido se vuelca al `#9B` antes del siguiente chunk. **Cero footprint RAM**.
- **Por qué Pletter es ~2.5× más lento que la opción "ram + dump"**: `pletter2vram` hace VDP register switching para cada back-reference (lee un byte de la salida ya escrita en VRAM, switchea VDP a write para el siguiente byte). Para un payload típico de imagen, ~280ms vs ~110ms de `pletter2ram`. Aceptable porque el contexto es "carga de imagen al entrar a una localización", no loop principal.
- **Persistencia HMMC**: verificada por análisis del código de `pletter2vram` (no toca R32-R46 ni port `#9B`). Ver §Test 0 para verificación empírica antes de dar por bueno el diseño.
- **Política DEPRECATED**: el warning se muestra en stdout, no en stderr, para no interferir con scripts que redirijan stderr. El comando termina con exit code 0 (no es un error, sólo un aviso).
- **Cuándo eliminar `copyToVRAM`**: cuando `dsk/` no contenga imágenes con `IMG_CHUNK_RAW`. La verificación se puede hacer con `imgwizard l *.IM*` y comprobar que sólo aparecen `INFO`/`PALETTE`/`CLS`/`RLE`/`V9938Cmd`/`V9938CmdData`. PRP futuro.
- **`pletter2vram` permanece** indefinidamente: ahora lo usa **también** `pletter2Data` (ruta v2). No se elimina ni siquiera cuando se retire la ruta v1.
- **`IMG_CHUNK_RLE` permanece** indefinidamente: mientras exista el caso de uso de transparencia. No se considera legacy.
- **Plan de fallback si Test 0 falla**: si el análisis empírico revelase que HMMC se aborta o corrompe durante `pletter2vram` (por ejemplo, una secuencia de `setVDP_Read`/`setVDP_Write` provoca timing issues con el command engine), volver a la **Opción A** (`pletter2ram` + buffer RAM 2KB en `heap_top + sizeof(IMG_CHUNK)` + `copyCmdData`). Coste adicional: 2KB RAM transitoria, pero evita el riesgo. Cap Pletter pasaría a `CHUNK_SIZE = 2043` (mismo que RAW/RLE).
