# PRP025 — `dzx0vram` y `dzx0Data`: descompresor ZX0 a VRAM/HMMC

## Contexto

PRP022 introdujo el formato V9938CmdData con tres compresores: RAW (0), RLE (1), Pletter5 (2). PRP023 implementó los descompresores HMMC streaming en runtime (`copyCmdData`, `unRLE_Data`, `pletter2Data`). PRP024 reutiliza el mismo pipeline para LMMC streaming (transparencia).

ZX0 (Einar Saukas) suele lograr 5-15% mejor compresión que Pletter5 sobre los mismos datos a coste de mayor tiempo de descompresión. Añadirlo como cuarto compresor del formato extiende el ratio de compresión disponible para autores de aventuras DAAD sin afectar la pipeline de chunks ya existente.

El proyecto ya tiene:
- [src/libs/utils_dzx0.asm](src/libs/utils_dzx0.asm) — decoder ZX0 estándar RAM→RAM (68 bytes, by Einar Saukas & Urusergi).
- [src/libs/utils_dzx0data.c](src/libs/utils_dzx0data.c) — wrapper que llama a `dzx0vram(src, VRAM_PLETTER_SCRATCH)` y luego `copyVRam2CmdData()` para streaming HMMC/LMMC. **Asume que `dzx0vram` existe.**
- [include/utils.h](include/utils.h:31) — declara `void dzx0vram(unsigned char* source, uint32_t vram_offset) SDCC_STACKCALL;`
- [bin/zx0](bin/zx0) y `bin/zx0.exe` — compresor de host (ya disponibles).
- [src/libs/utils_pletter2vram.s](src/libs/utils_pletter2vram.s) — implementación de referencia del patrón "decompress directo a VRAM con read-modify-write por byte", adaptada para V9938 128KB.

**Falta** implementar `utils_dzx0vram.s` y cablear el compresor ZX0 en el resto del pipeline (`imgwizard.php`, runtime dispatcher).

## Objetivo

1. Implementar `src/libs/utils_dzx0vram.s` — decoder ZX0 con destino directo a VRAM, análogo a `pletter2vram.s`.
2. Añadir el compresor ZX0 al `imgwizard.php` (`compressors[]`, command `cx[l]` acepta `ZX0`).
3. Añadir `case 3: dzx0Data(...)` al handler `IMG_CHUNK_CMDDATA` en `src/daad_platform_msx2.c`.
4. Renombrar conceptualmente `VRAM_PLETTER_SCRATCH` → `VRAM_DECOMP_SCRATCH` (la zona scratch ya no es exclusiva de Pletter; opcional pero recomendado).
5. Tests manuales: comprimir con ZX0 vs PLETTER y verificar ratio + render correcto en openMSX.

## Diseño técnico

### 1. `src/libs/utils_dzx0vram.s` — implementación

#### 1.1 Estrategia general

Tomar el algoritmo estándar de ZX0 (`utils_dzx0.asm`, 68 bytes, dos `ldir` por bucle) y reemplazar **ambos `ldir`** por equivalentes que escriben/leen VRAM vía port #98 con paginación R#14:

| ZX0 estándar | dzx0vram |
|--------------|----------|
| `ldir` literales (HL→DE en RAM) | bucle `ld a,(hl); inc hl; out (#98),a; inc de` (VDP en modo escritura) |
| `ldir` back-ref (HL=DE-offset → DE) | bucle: por byte, configurar VDP read en VRAM(DE-offset) → IN A,(#98); configurar VDP write en VRAM(DE) → OUT (#98),A |

La paginación 17-bit se gestiona con una variable estática `(#page)` (igual patrón que `pletter2vram` usa: `(#page)` guarda el bit 16, los bits 14-15 se calculan de los bits altos de D vía `rlc d / rla / rlc d / rla / srl d / srl d`).

#### 1.2 Calling convention (SDCC_STACKCALL)

Firma C: `void dzx0vram(unsigned char* source, uint32_t vram_offset) SDCC_STACKCALL;`

Stack al entrar (arriba → abajo):
```
SP+0: return addr (2 bytes)
SP+2: source ptr lo/hi (2 bytes)
SP+4: vram_offset bytes 0-3 (4 bytes, little-endian: low word first)
```

Patrón de prólogo (idéntico a `pletter2vram`):
```asm
_dzx0vram::
    push ix
    ld   ix,#4
    add  ix,sp
    ld   l, 0(ix)        ; HL = source
    ld   h, 1(ix)
    ld   e, 2(ix)        ; DE = vram_offset low 16 bits
    ld   d, 3(ix)
    ld   a, 4(ix)        ; A = vram_offset byte 16 (page)
    push bc
    push de
    push hl
    push iy
    ; ... cuerpo ZX0 adaptado ...
.depack_out:
    pop  iy
    pop  hl
    pop  de
    pop  bc
    pop  ix
    ret
```

#### 1.3 Setup inicial de VDP (modo escritura en `vram_offset`)

Mismo bloque que `pletter2vram` líneas 35-54:
```asm
    ld   (#page),a
    push de
    rlc  d
    rla
    rlc  d
    rla
    srl  d
    srl  d
    di
    out  (#0x99),a       ; R#14 data: address bits 14-16 (3 bits)
    ld   a,#128+14
    out  (#0x99),a       ; select R#14
    ld   a,e
    out  (#0x99),a       ; address bits 0-7
    ld   a,d
    or   #0b01000000     ; bit 6 = WRITE mode
    ei
    out  (#0x99),a       ; address bits 8-13
    pop  de
```

Conviene factorizar este bloque y el equivalente de modo lectura en dos sub-rutinas internas:
- `setVDP_writeAt_DE_pageA` (modo escritura, A=page byte salvado)
- `setVDP_readAt_HL_pageA` (modo lectura, para back-refs)

`pletter2vram` no las factoriza (las inlinea por velocidad). Para consistencia con el código existente, **inlinear igual** salvo que el tamaño quede prohibitivo.

#### 1.4 Bucle ZX0 adaptado — pseudocódigo

```asm
dzx0vram_main:
    ld   bc,#0xffff      ; preserve default offset 1
    push bc
    inc  bc
    ld   a,#0x80
.literals:
    call .elias
    ; LDIR → VRAM write loop (HL=src RAM, DE=VRAM offset, BC=count)
    call .ldir_ram_to_vram
    add  a,a
    jr   c,.new_offset
    call .elias
.copy:
    ; LDIR back-ref → VRAM read+write loop
    ;   src VRAM = (DE - offset) ; offset on stack at top
    ;   dst VRAM = DE
    ;   count = BC
    ex   (sp),hl         ; preserve src RAM ptr, HL = offset
    push hl              ; preserve offset
    ; calculate VRAM_src = DE - offset (17-bit subtraction with borrow into page)
    ; ...
    call .ldir_vram_to_vram
    pop  hl              ; restore offset
    ex   (sp),hl         ; preserve offset, restore src RAM ptr
    add  a,a
    jr   nc,.literals
.new_offset:
    pop  bc              ; discard last offset
    ld   c,#0xfe
    call .elias_loop
    inc  c
    jp   z,.depack_out   ; end marker
    ld   b,c
    ld   c,(hl)
    inc  hl
    rr   b
    rr   c
    push bc              ; preserve new offset
    ld   bc,#1
    call nc,.elias_backtrack
    inc  bc
    jr   .copy

.elias:
    inc  c
.elias_loop:
    add  a,a
    jr   nz,.elias_skip
    ld   a,(hl)
    inc  hl
    rla
.elias_skip:
    ret  c
.elias_backtrack:
    add  a,a
    rl   c
    rl   b
    jr   .elias_loop
```

#### 1.5 Sub-rutinas críticas

**`.ldir_ram_to_vram`** (literales, HL=RAM src, DE=VRAM low 16, BC=count):
```asm
.ldir_ram_to_vram:
    ld   a,b
    or   c
    ret  z
    ; VDP ya está configurado en modo escritura en VRAM(DE) por la llamada previa
.lrv_loop:
    ld   a,(hl)
    out  (#0x98),a
    inc  hl
    inc  de              ; tracker VRAM dst
    ; ¿overflow de 14 bits low? si DE pasa de 0x3FFF a 0x4000, hay que recalcular página
    ; Solución simple: detectar wrap byte 0xFF→0x00 en E AND D=0 → propagar a página
    ; Mejor: dejar al VDP auto-incrementar y NO tocar nada hasta el siguiente back-ref
    ; (port #98 auto-increment maneja los bits 0-13 internamente; bit 14-16 quedan en R#14)
    ; Si BC pasa el límite de 16KB, hay que reconfigurar R#14
    dec  bc
    ld   a,b
    or   c
    jr   nz,.lrv_loop
    ret
```

**Punto delicado**: el auto-increment del VDP solo cubre 14 bits (0..0x3FFF) dentro de la página seleccionada por R#14. Si la zona scratch está alineada a frontera de 16KB (0x1D400), **un literal de 11264 bytes a partir de 0x1D400 NO cruza frontera de 16KB** (0x1D400 + 0x2C00 = 0x20000 = exactly start of next 16KB). Pero rectángulos grandes podrían. **Mitigación**: tras cada `out (#0x98),a`, comprobar si el contador VDP rebasó el límite de 16KB y reconfigurar R#14. Pletter2vram no lo hace porque su scratch zone es <16KB; lo mismo aplica aquí (cap `CHUNK_PLETTER_MAX_UNCOMP=11264 < 16384`).

**Decisión**: Igual que `pletter2vram`, **asumir que cada chunk decompressed cabe en un mismo bloque de 16KB de VRAM** (cap impuesto por imgwizard). Si la decompresión cruzase frontera, sería un bug de imgwizard a detectar en validación, no del decoder.

**`.ldir_vram_to_vram`** (back-ref, src=DE-offset, dst=DE, count=BC):
```asm
.ldir_vram_to_vram:
    ; HL = src VRAM address (DE - offset)
    ; DE = dst VRAM address
    ; BC = count
.lvv_loop:
    ; --- READ MODE en HL ---
    push de
    ld   a,(#page)       ; recalcular page byte considerando back-ref puede haber cruzado bit 16
    ; ... mismo bloque rlc/rla/srl que setup ... pero con HL en lugar de DE
    ; ... out R#14, out address bits 0-7, out address bits 8-13 SIN bit 6 (read mode) ...
    ; (ver pletter2vram líneas 154-176 para el patrón)
    in   a,(#0x98)
    ex   af,af'          ; preservar byte leído
    pop  de

    ; --- WRITE MODE en DE ---
    push hl
    ld   a,(#page)
    ; ... mismo bloque pero con DE y bit 6 (write mode) ...
    ex   af,af'
    out  (#0x98),a
    pop  hl

    inc  hl
    inc  de
    dec  bc
    ld   a,b
    or   c
    jr   nz,.lvv_loop
    ret
```

Esta es la parte costosa: **2 reconfiguraciones VDP por byte de back-ref**. Cada reconfig son ~12 OUTs a port #99 + 1 OUT a port #98. Es lento — pletter2vram tiene exactamente la misma penalización en su loop `$9` (líneas 152-203).

**Optimización opcional (post-PRP)**: leer N bytes consecutivos en buffer RAM, luego escribir N bytes consecutivos. Requeriría buffer ~256 bytes y modifica el loop. ZX0 no se queja si el back-ref se procesa en chunks más pequeños siempre que se respete el orden. Dejar para optimización futura.

#### 1.6 Cálculo de VRAM_src = VRAM_dst - offset (17-bit)

ZX0 mantiene el offset en el stack como un valor de 16 bits (`bc`/`hl` después del `ex (sp),hl`). El destino actual es `DE` (low 16 bits) + `(#page)` (bit 16).

Cálculo:
```asm
; Input:  DE = dst low 16, (#page) = dst bit 16, HL = offset (16 bits)
; Output: HL = src low 16, A = src bit 16 (NO sobrescribe (#page))
    push de
    ex   de,hl           ; DE = offset, HL = dst low
    or   a               ; clear carry
    sbc  hl,de           ; HL = dst_low - offset; carry = borrow del bit 16
    ld   a,(#page)
    sbc  a,#0            ; A = page - borrow
    pop  de
```

Nota: si `dst_low < offset`, el resultado underflow propaga al bit 16 (decrementa la página). Esto es el comportamiento correcto en aritmética 17-bit.

### 2. Cambios en `bin/imgwizard.php`

Añadir entradas al array `$compressors`:

```php
$compressors = array(
    array("raw",     "raw",   CHUNK_RAW,     "RAW"),
    array("rle",     "rle",   CHUNK_RLE,     "RLE"),
    array("pletter", "plet5", CHUNK_PLETTER, "PLETTER"),
    array("zx0",     "zx0",   CHUNK_ZX0,     "ZX0"),         // [PRP025] NEW
);
define('ZX0', 3);
```

Y la nueva constante de chunk type compID:
```php
define('CMP_ZX0', 3);
```

Adaptar la mapping en `buildV9938CmdSequence`:
```php
$compID = $comp[COMP_ID]==CHUNK_RAW    ? CMP_RAW
       : ($comp[COMP_ID]==CHUNK_RLE    ? CMP_RLE
       : ($comp[COMP_ID]==CHUNK_PLETTER ? CMP_PLETTER
       :                                 CMP_ZX0));
```

Y actualizar la ayuda en `showSyntax()` para listar `ZX0` como opción de compressor.

**Cap de tamaño**: `CHUNK_PLETTER_MAX_UNCOMP = 11264` se reusa para ZX0 (mismo scratch). El nombre debería renombrarse a `CHUNK_DECOMP_MAX_UNCOMP` para reflejar que sirve para Pletter y ZX0. Cambio cosmético.

**Compresor de host**: el binario `bin/zx0` (Linux) y `bin/zx0.exe` (Windows) ya existen. La función `compress()` invoca el compressor por el campo `COMP_APP`:

```php
function compress($tmp, $in, $pos, $sizeIn, $comp, $transparent=-1)
{
    ...
    if ($comp[COMP_APP]=="zx0") {
        file_put_contents($tmp, $data);
        exec("zx0 $tmp", $out);   // produce $tmp.zx0
    }
    ...
}
```

Hay que comprobar si zx0 acepta el mismo patrón de invocación que pletter (`./zx0 <input>` produce `<input>.zx0`). Si no, ajustar el wrapper.

```
ZX0 v2.2: Optimal data compressor by Einar Saukas
Usage: zx0 [-f] [-c] [-b] [-q] input [output.zx0]
  -f      Force overwrite of output file
  -c      Classic file format (v1.*)
  -b      Compress backwards
  -q      Quick non-optimal compression
```

Usar `zx0 -f <input> <output>` para sobrescribir el mismo archivo temporal.

### 3. Cambios en runtime (`src/daad_platform_msx2.c`)

Activar el case `3` en el dispatcher de `IMG_CHUNK_CMDDATA`:

```c
switch (compID) {
    case 0:  copyCmdData(((IMG_V9938_CMDDATA*)chunk)->data, uncomp);     break;   // RAW
//  case 1:  unRLE_Data(((IMG_V9938_CMDDATA*)chunk)->data);              break;   // RLE
//  case 2:  pletter2Data(((IMG_V9938_CMDDATA*)chunk)->data, uncomp);    break;   // PLETTER
    case 3:  dzx0Data(((IMG_V9938_CMDDATA*)chunk)->data, uncomp);        break;   // ZX0 [PRP025]
}
```

(Mantener PRP024 y PRP023 sus comentarios — la activación de RLE/Pletter5/ZX0 es independiente.)

### 4. Renombrado conceptual `VRAM_PLETTER_SCRATCH` → `VRAM_DECOMP_SCRATCH`

[include/utils.h:8](include/utils.h#L8) actualmente:
```c
#define VRAM_PLETTER_SCRATCH  0x1D400UL
```

Renombrar a `VRAM_DECOMP_SCRATCH` y mantener un alias deprecado:
```c
#define VRAM_DECOMP_SCRATCH   0x1D400UL    // Shared scratch for Pletter5/ZX0/...
#define VRAM_PLETTER_SCRATCH  VRAM_DECOMP_SCRATCH  // Legacy alias
```

Actualizar referencias en `utils_pletter2data.c` y `utils_dzx0data.c` opcionalmente. Cambio cosmético.

### 5. Listado de chunks en `imgwizard l <file>`

Añadir 'zx0' al array de nombres en [bin/imgwizard.php:623](bin/imgwizard.php#L623):
```php
$compName = ['raw','rle','pletter','zx0'][$compID] ?? '?';
```

## Compatibilidad

- Imágenes generadas con `RAW`/`RLE`/`PLETTER` siguen funcionando sin cambios (compID 0/1/2 en V9938CmdData se interpretan igual).
- Imágenes nuevas con `ZX0` (compID 3) requieren el runtime con el nuevo `case 3` activado. Imágenes ZX0 en runtime antiguo: el switch case sin entrada saltea silenciosamente → render incompleto. Aceptable porque ZX0 es una extensión, no un cambio de formato existente.

## Consideraciones de rendimiento

| Compresor | Ratio típico (vs RAW) | Velocidad descomp. | Tamaño decoder |
|-----------|------------------------|--------------------|-----------------|
| RAW       | 100%                   | Velocidad pura VDP | ~30 bytes (`copyCmdData`) |
| RLE       | 30-70% (bitmaps)       | ~2× RAW            | ~50 bytes (`unRLE_Data`) |
| PLETTER   | 15-40% (bitmaps)       | ~3-4× RAW          | ~244 bytes (`pletter2vram`) + ~30 wrapper |
| **ZX0**   | **10-35% (bitmaps)**   | ~5-8× RAW          | **~80-120 bytes** (estimado adaptación VRAM) |

ZX0 gana en compresión ~5-15% sobre Pletter5 a cambio de ser ~1.5-2× más lento en descompresión. Para imágenes que se cargan **una sola vez** en una aventura DAAD (típico) el coste de descompresión es despreciable comparado con la ganancia en tamaño de fichero.

**Casos de uso recomendados**:
- Imágenes fullscreen que se ven una vez al inicio de una localización → ZX0 (max compresión).
- Sprites/iconos repintados frecuentemente → PLETTER o RLE (más rápido).
- Datos pequeños o muy redundantes → RLE.
- Datos críticos en velocidad de carga → RAW.

## Test Plan

### Tests automáticos

1. Compresión host: comprimir un buffer conocido con `bin/zx0` y descomprimirlo en host con `dzx0` (versión RAM standard) — verificar round-trip byte-perfect.
2. Test de tamaño: comprimir `assets/FONT.SC8` con `cx ZX0` vs `cx PLETTER`. Confirmar que ZX0 genera tamaño ≤ Pletter5.

### Tests runtime en openMSX

3. **Test correctness**: imagen 64×32 SC8 con `cx ZX0` — render correcto sin artefactos.
4. **Test multi-chunk**: imagen 256×100 SC8 (>11264 bytes uncomp) con `cx ZX0` — fuerza ≥2 chunks ZX0 consecutivos. Render correcto.
5. **Test back-ref largo**: imagen con patrones repetitivos a >2KB de distancia (e.g., bordes de marco) — verifica que el back-ref VRAM funciona aunque la distancia exceda los 256 bytes.
6. **Test transparencia + ZX0**: `cx --transparent-color=255 ZX0` — verifica que ZX0 funciona con LMMC AND+OR.

### Tests unitarios

No aplicable — los tests existentes en `unitTests/` no cubren VDP. La verificación es visual en openMSX.

## Riesgos / Unknowns

1. **`bin/zx0` invocación**: hay que verificar que el wrapper `zx0 <input>` produce `<input>.zx0` con el mismo patrón que `pletter`. Si no, ajustar el campo `COMP_EXT` o lanzar el binario diferente.

2. **Cruce de frontera 16KB en el destino**: tanto en escritura inicial como en back-ref. Si el VDP cruza el bit 14 con auto-increment, el byte siguiente se escribe en una página VRAM incorrecta. **Mitigación**: cap de chunks `CHUNK_DECOMP_MAX_UNCOMP=11264 < 16384` garantiza que un chunk no cruza, **pero** el VRAM_DECOMP_SCRATCH a 0x1D400 + 11264 = 0x1FF00 está dentro de la misma página de 16KB (bits 14-16 = 7 = páginas 7 = 0x1C000..0x1FFFF). ✓

3. **Velocidad de back-ref**: la reconfiguración del VDP por byte es lenta (~50 ciclos Z80 por byte vs ~6 ciclos para LDIR). Para imágenes con muchos back-refs largos, ZX0 puede ser 10× más lento que Pletter5 en VRAM. **Aceptable** para carga única; **inaceptable** para sprites animados.

4. **Tamaño del decoder**: la adaptación VRAM puede inflar significativamente el código (estimado 80-120 bytes vs 68 del estándar RAM). Aún manejable.

5. **Versión ZX0 "Standard" vs "Turbo"**: el `utils_dzx0.asm` actual es la versión Standard (más pequeña, más lenta). Existe una versión Turbo (~125 bytes, ~25% más rápida). Para VRAM la velocidad de descompresión está dominada por las reconfiguraciones de VDP, así que **Standard es suficiente**.

6. **`copyVRam2CmdData` requiere R#17 = 44|0x80**: ya gestionado por PRP023 fix (`setIndirectRegisterPointer` antes del loop). Aplica igual a ZX0 → no cambio adicional.

## Cambios resumidos

### Archivos NUEVOS

- `src/libs/utils_dzx0vram.s` — decoder ZX0 con destino VRAM (~80-120 bytes).

### Archivos MODIFICADOS

- `bin/imgwizard.php` — añadir compresor ZX0 al array, constantes `CMP_ZX0`/`CHUNK_ZX0` (este último en el sense de COMP_ID, no chunk type), mapping en `buildV9938CmdSequence`, listado de chunks.
- `src/daad_platform_msx2.c` — activar `case 3: dzx0Data(...)` en handler IMG_CHUNK_CMDDATA.
- `include/utils.h` — añadir alias `VRAM_DECOMP_SCRATCH`, mantener `VRAM_PLETTER_SCRATCH` legacy.

### Archivos NO modificados

- `src/libs/utils_dzx0.asm` (decoder RAM standard, sin cambios).
- `src/libs/utils_dzx0data.c` (wrapper, ya escrito y referenciando `dzx0vram` que ahora existirá).
- `src/libs/vdp_copyvram2cmddata.s` (ya validado por PRP023, agnóstico al origen del chunk en VRAM scratch).

## Acción inmediata sugerida

1. **Verificar el binario `bin/zx0`** invocándolo con un fichero de test y confirmando que produce `<file>.zx0`. Si la invocación difiere, anotar en el array `$compressors`.
2. **Implementar `utils_dzx0vram.s`** siguiendo el pseudocódigo §1.4 + helpers §1.5. Empezar con la versión "byte-a-byte" en back-refs (más simple); optimizar después si los tests muestran lentitud inaceptable.
3. **Añadir `ZX0` al array de compresores** en `imgwizard.php` y testear `cx FONT.SC8 0 0 64 32 ZX0`.
4. **Activar `case 3`** en runtime y rebuild.
5. **Test en openMSX** con la imagen 000 (regenerar con `cx ... ZX0`).

## Referencias

- [ZX0 official repo (Einar Saukas)](https://github.com/einar-saukas/ZX0) — algoritmo y decoder estándar.
- [src/libs/utils_dzx0.s](src/libs/utils_dzx0.s) — decoder RAM existente (basis para adaptar).
- [src/libs/utils_pletter2vram.s](src/libs/utils_pletter2vram.s) — patrón de adaptación VRAM (referencia arquitectónica).
- [src/libs/utils_pletter2data.c](src/libs/utils_pletter2data.c) — wrapper análogo al que usará `dzx0Data`.
- PRP022 — formato V9938CmdData con `compressorID`.
- PRP023 — descompresores HMMC streaming (R#17, persistencia de comando).
- PRP024 — pipeline LMMC para transparencia (compatible automáticamente con ZX0).

---

## Anexo A — Descubrimientos durante la implementación

Esta sección documenta correcciones, decisiones y bugs descubiertos durante la implementación que extienden o modifican el diseño original.

### A.1 — Renombrar `utils_dzx0.asm` → `utils_dzx0.s`

**Problema**: el Makefile sólo tiene reglas para extensiones `.s` y `.c` ([Makefile:62-83]). El archivo original `utils_dzx0.asm` no era compilado por el wildcard `utils_*` y rompía la build cuando se intentaba.

**Fix**:
1. Renombrar `src/libs/utils_dzx0.asm` → `src/libs/utils_dzx0.s`.
2. Convertir sintaxis Z80 estándar (`$ffff`, `bc, 1`) a sintaxis sdasz80 (`#0xffff`, `bc, #1`).
3. Añadir directiva `.area _CODE` al inicio del fichero.

**Implicación**: el decoder RAM-only ZX0 estándar (no usado en runtime, pero referenciado en `include/utils.h:32`) queda correctamente compilable.

### A.2 — Renombrado `dzx0data` → `dzx0Data` para coherencia con `pletter2Data`

**Problema**: el wrapper original se llamaba `dzx0data` (minúsculas) mientras que el equivalente Pletter usa `pletter2Data` (camelCase con D mayúscula).

**Fix**: renombrar la función a `dzx0Data` en `src/libs/utils_dzx0data.c` y añadir su prototipo en `include/utils.h:34`.

### A.3 — `VRAM_PLETTER_SCRATCH` → `VRAM_DECOMP_SCRATCH` con alias retrocompatible

Aplicado según diseño en §4. `include/utils.h:8-10`:
```c
#define VRAM_DECOMP_SCRATCH   0x1D400UL    // shared scratch zone (Pletter5/ZX0)
#define VRAM_PLETTER_SCRATCH  VRAM_DECOMP_SCRATCH    // legacy alias
```

Tanto `utils_pletter2data.c` como `utils_dzx0data.c` usan ahora `VRAM_DECOMP_SCRATCH`.

### A.4 — Bug del doble `pop bc` en `zx0v_done`

**Problema**: durante la primera implementación, `zx0v_done` hacía un `pop bc` extra "para descartar el offset en el top del stack". Pero el path `zx0v_new_offset` ya hace `pop bc` al inicio (descartar offset anterior antes de leer el nuevo), y de ahí salta a `zx0v_done` con `jp z, zx0v_done` cuando llega el end marker. El segundo `pop bc` consumía un byte que NO era el offset — corrompía la pila y el `ret` final saltaba a una dirección errónea.

**Síntoma observado**: openMSX se colgaba durante la carga de FONT con ZX0. El traza mostraba que dzx0vram parecía completar la decompresión (scratch tenía datos válidos) pero el control de ejecución no volvía limpiamente al caller.

**Fix**: eliminar el `pop bc` redundante de `zx0v_done`. El comentario que justifica el cambio está incrustado en el código fuente.

```asm
zx0v_done:
    ; Reached only via `jp z` from zx0v_new_offset, where the previous offset
    ; was already discarded with `pop bc` and no new offset has been pushed.
    ; So the stack at this point holds only the 4 saved regs (bc/de/hl/iy).
    pop iy
    pop hl
    pop de
    pop bc
    ret
```

### A.5 — Restricción ZX0 al path V2 (`cx`/`cxl`)

**Decisión** (durante implementación): la usuaria pidió que ZX0 NO fuera disponible en los comandos legacy `c`/`cl`/`s` (que emiten chunks v1). Solo via `cx`/`cxl` con V9938CmdData.

**Fix en `bin/imgwizard.php`**:
1. NO definir `CHUNK_ZX0` (no hay chunk type v1 para ZX0).
2. La entrada en `$compressors` usa `COMP_ID = -1` como sentinel "no v1 chunk type":
   ```php
   array("zx0",     "zx0",   -1,            "ZX0"),       // [PRP025] cx[l] only
   ```
3. Validación explícita en el handler `c`/`cl`:
   ```php
   if ($comp[COMP_ID] < 0) {
       echo "ERROR: ZX0 compression is only supported via 'cx'/'cxl' commands, not 'c'/'cl'.\n";
       exit;
   }
   ```
4. El mapping en `buildV9938CmdSequence` usa fallthrough, así que ZX0 (COMP_ID=-1) cae correctamente en `CMP_ZX0=3` al emitir el V9938CmdData chunk.

### A.6 — Optimizaciones de tamaño aplicadas (-57 bytes)

Tras la primera versión funcional, se aplicaron 10 optimizaciones para reducir el tamaño del decoder. **Aplicadas y validadas una a una** en openMSX para localizar bugs sutiles que sólo se manifiestan al combinarlas. Las opts 8-10 fueron detectadas usando `bin/mdl.jar` (Z80 optimizer de Santiago Ontañón).

| # | Opt | Δ bytes | Notas |
|---|-----|---------|-------|
| 1 | Eliminar 2 `nop` finales en setVDP_read_DE | -2 | El `pop de; ret` + caller's `pop de; in a` proveen suficiente delay para el VDP. |
| 2 | Mover offset de stack a `(#zx0_offset)` memoria | -9 | Elimina el `ex (sp),hl; push hl; pop hl; ex (sp),hl` en `zx0v_copy` y el stack-peek con IX en `ldir_vv`. **Requiere añadir `push hl` / `pop hl` en ldir_vv** (ver A.7). |
| 3 | Merge `setVDP_write_DE` + `setVDP_read_DE` en `setVDP_DE` (carry=mode) | -16 | Función única con flag de carry como modo. B se usa como OR mask (preservado vía push/pop bc al inicio para no destruir BC del caller). |
| 4 | Prólogo basado en HL+SP en lugar de IX | -10 | Sustituye `push ix; ld ix,#4; add ix,sp; ld r,N(ix); ...; pop ix` por `push bc; push de; push hl; push iy; ld hl,#10; add hl,sp; ld r,(hl); inc hl; ...`. |
| 5 | Refactor `zx0v_lvv_loop` con `ex de,hl` | -3 | Elimina la secuencia `pop de; push de; ld e,l; ld d,h` (4 bytes) reemplazándola por `ex de,hl` (1 byte). Un único `push de` al inicio del loop preserva el dst, restaurado tras la lectura VRAM y antes del setVDP write. |
| 6 | `push af` / `pop af` en lugar de `(#zx0_acc)` | -8 | En ambos helpers `zx0v_ldir_*_to_vram`, sustituye `ld (#zx0_acc),a` (3) y `ld a,(#zx0_acc)` (3) por `push af` / `pop af` (1+1). También elimina la variable `zx0_acc` del segmento `_DATA` (-1 byte RAM). |
| 7 | Simplificar lógica de máscara write/read en `setVDP_DE` | -2 | `jr c,w; ld b,#0; jr common; w: ld b,#0x40` (8 bytes) → `ld b,#0; jr nc,common; ld b,#0x40` (6 bytes). Invertir el branch elimina un `jr` redundante. |
| 8 | **[MDL]** Inline `zx0v_ldir_ram_to_vram` (single call site) | -4 | El helper sólo se llama desde `zx0v_literals`. Inlinearlo elimina el `call` (3 bytes) y el `ret` final (1 byte). |
| 9 | **[MDL]** Inline `zx0v_ldir_vram_to_vram` (single call site) | -4 | Idem para el helper VRAM→VRAM, sólo llamado desde `zx0v_copy`. |
| 10 | **[MDL]** `cpi; jp pe,loop` en lugar de `inc hl/dec bc/ld a,b/or c/jr nz` | -1 | En el loop de literales (RAM→VRAM). `cpi` (ED A1) hace `inc hl; dec bc` y deja P/V=1 si BC≠0. Bonus: -6 ciclos por iteración. No aplicable al loop VRAM→VRAM (no hay `inc hl` ahí). |

**Total: -57 bytes** (~22% reducción del decoder original) + **-1 byte RAM**.

**Herramientas usadas:**
- `bin/mdl.jar` (MDL v2.6.2): patrones, reorganización, peephole. Detectó opts 8-10 que no eran obvias por inspección manual.
- Análisis comparativo con [saukas/ZX0 z80 variants](https://github.com/einar-saukas/ZX0/tree/main/z80) (Standard/Turbo/Fast/Mega): confirmado que Standard es la base óptima. Las variantes mayores trade size-by-speed que no se traduce en ganancia neta cuando el bottleneck es el VDP byte-a-byte.

### A.7 — Bug crítico al aplicar Opt 2: HL destruido en `ldir_vv`

**Problema**: al mover el offset de stack a memoria estática (`(#zx0_offset)`), la primera instrucción de `zx0v_ldir_vram_to_vram` se convirtió en `ld hl,(#zx0_offset)`, **destruyendo HL** (que contenía el src RAM ptr).

En la versión stack original, HL se preservaba **implícitamente** en la pila: el caller `zx0v_copy` hacía `ex (sp),hl; push hl` antes de la llamada → src ptr quedaba en la pila. La función ldir_vv leía el offset peekeando la pila (sin tocar el src ptr).

Con offset en memoria, el src ptr ya no se preserva en pila. Tras `ldir_vv`, HL contenía garbage, y el siguiente paso `zx0v_new_offset` falla en `ld c,(hl)` (lee LSB del offset desde src ptr) leyendo bytes aleatorios de RAM.

**Síntoma observado**: scratch tenía bytes corruptos a partir de offset ~10 (los primeros bytes coincidían porque eran literales secuenciales, los siguientes fallaban cuando entraba el primer back-ref).

**Fix**: añadir `push hl` / `pop hl` dentro de `zx0v_ldir_vram_to_vram` para preservar el src RAM ptr a través de la llamada:

```asm
zx0v_ldir_vram_to_vram:
    ld   (#zx0_acc),a
    push hl                     ; preserve src RAM ptr (HL destroyed inside loop)
    ld   a,b
    or   c
    jr   z,zx0v_lvv_done
    ; ... loop body destroys HL ...
zx0v_lvv_done:
    pop  hl                     ; restore src RAM ptr
    ld   a,(#zx0_acc)
    ret
```

**Coste**: 2 bytes (push hl + pop hl). Pero compensado por la eliminación del `ex/push/pop` dance en `zx0v_copy` (-7 bytes).

**Lección general**: cuando se migra estado de stack a memoria, hay que rastrear todas las dependencias implícitas que la pila estaba preservando.

### A.8 — Estrategia de validación incremental para optimizaciones

Cuando se aplicaron las 4 optimizaciones de A.6 todas a la vez, el sistema falló sin permitir localizar la causa. **Aplicación incremental** (una optimización + build + test en openMSX por iteración) localizó el bug A.7 al paso 2 inmediatamente.

**Recomendación para futuras optimizaciones**: aplicar siempre cambio-por-cambio cuando se modifique código de bajo nivel con dependencias implícitas (stack, registros, flags, estado VDP).

### A.9 — Tabla final de archivos modificados

| Archivo | Cambio |
|---------|--------|
| `src/libs/utils_dzx0vram.s` | NUEVO. Decoder ZX0→VRAM optimizado (~219 bytes tras todas las optimizaciones). |
| `src/libs/utils_dzx0data.c` | Renombrado `dzx0data` → `dzx0Data`, usa `VRAM_DECOMP_SCRATCH`. |
| `src/libs/utils_dzx0.s` | Renombrado de `.asm`, sintaxis sdasz80. |
| `bin/imgwizard.php` | Compresor ZX0 con `COMP_ID=-1` sentinel + validación rechazo en `c`/`cl`. |
| `src/daad_platform_msx2.c` | `case 3: dzx0Data(p, uncomp)` activado en handler IMG_CHUNK_CMDDATA. |
| `include/utils.h` | Alias `VRAM_DECOMP_SCRATCH` + prototipo `dzx0Data`. |
