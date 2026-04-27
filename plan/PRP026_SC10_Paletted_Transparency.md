# PRP026 — Transparencia en `CX[L]` para SCREEN 10 (SCA paletted+YJK mixed)

## Contexto

PRP024 introdujo transparencia en `cx[l]` mediante el patrón LMMC|AND + LMMC|OR (PRP024 §Diseño técnico §1.3) para los modos bitmap "puros" SC5/SC6/SC7/SC8. La validación actual en [bin/imgwizard.php:1241-1244](../bin/imgwizard.php#L1241-L1244) **bloquea** explícitamente `--transparent-color` para SC10 (`.SCA`) y SC12 (`.SCC`) con el argumento de que la codificación YJK comparte las componentes de chroma (J, K) entre grupos de 4 píxeles consecutivos, lo que rompería las operaciones lógicas byte-a-byte de LMMC.

Sin embargo, **SCREEN 10 es YJK+YAE** (no YJK puro como SCREEN 12). Cada byte de pixel SCA contiene un bit "A" (bit 3) que actúa como **flag de modo per-pixel**:
- A=1 → pixel paletado (4 bits altos = índice de paleta 0-15; bits 0-2 ignorados al renderizar).
- A=0 → pixel YJK (Y per-pixel + J/K compartidos en grupos de 4).

Este PRP aprovecha el flag A como **flag de transparencia natural** y permite `cx` con `--transparent-color=N` sobre `.SCA`, tratando los pixels YJK como inherentemente "preservar destino" y los pixels paletados con índice N como transparentes (semántica idéntica a la de PRP024 sobre SC5/7/8).

SCREEN 12 (SCC) sigue rechazado: no tiene flag A, todos los pixels son YJK puros, no hay forma de marcar pixels individuales como "transparent overlay candidate".

## Objetivo

Permitir `cx[l] in.SCA <x> <y> <w> <h> [comp] --transparent-color=N` con la siguiente semántica:

| Pixel del rectángulo SCA | Acción en el IMA generado |
|--------------------------|---------------------------|
| A=0 (YJK) | **Implícitamente transparente** → mask=0xFF, image=0x00 (preserve dest) |
| A=1 ∧ índice paleta = N | **Transparente** → mask=0xFF, image=0x00 (preserve dest) |
| A=1 ∧ índice paleta ≠ N | **Visible** → mask=**0x07**, image=byte_orig & 0xF8 (sobrescribe Y/idx+A, **preserva bits 2-0 del destino**) |

Sin `--transparent-color`, `cx in.SCA` mantiene el flujo actual de PRP022 (HMMC raw, sin operaciones lógicas).

**Nota crítica sobre el caso paletado-visible**: la máscara NO es 0x00 (sería incorrecto). Es 0x07 para preservar los bits 2-0 del destino, que codifican K low/K high/J low/J high del grupo YJK alineado de 4 bytes al que pertenece este pixel. Ver §"Chroma sharing en grupos de 4" más abajo.

## Por qué este diseño

### El bit A es el "flag de transparencia natural" en SC10

Yamaha eligió YJK+YAE específicamente para que un autor pueda **mezclar** pixels paletados (UI/sprites con colores nítidos) con pixels YJK (fotorealismo). Cuando un autor quiere componer un sprite paletado sobre un fondo YJK ya dibujado en VRAM:

- Las regiones YJK del **sprite** no quieren tocar las regiones YJK del **fondo** (rompería el chroma compartido).
- Las regiones paletadas del sprite con un color "background" tampoco quieren overwrite el destino.
- Solo las regiones paletadas con colores "foreground" deben escribirse.

La regla `(A=0) → preserve | (A=1 ∧ idx=N) → preserve | resto → write` mapea **directamente** este caso de uso sin requerir ninguna heurística adicional.

### Compatibilidad con el workflow `5a` + `cx`

El comando `5a` (legacy) convierte un fichero SC5 a SCA crudo (BSAVE-compatible) generando bytes con bit A=1 en todos los pixels. Tras este PRP, el usuario podrá:

```bash
imgwizard 5a Sprite.SC5 Sprite.SCA 96
imgwizard cx Sprite.SCA 0 0 256 96 rle --transparent-color=0
```

Y obtener una `Sprite.IMA` con transparencia idéntica a la que tendría con `cx Sprite.SC5 ... --transparent-color=0`, pero **renderizable en una build `-DSCREEN=10`**.

### Cero cambios en runtime

El motor MSX2DAAD para SCREEN 10 ya despacha chunks `V9938Cmd` (LMMC opcode) sobre páginas paletadas (modo R#25=0x18 YJK+YAE). El renderizado de fuente actual ([src/daad_platform_msx2.c:744-756](../src/daad_platform_msx2.c#L744-L756)) prueba que las operaciones lógicas LMMC|AND/OR/XOR/TAND funcionan correctamente en SCA paletado. Los IMA generados por este PRP se reproducen con el handler `IMG_CHUNK_CMD`/`IMG_CHUNK_CMDDATA` existente, sin modificaciones.

## Restricciones

- **SCREEN 12 (`.SCC`)**: sigue **rechazado** para `--transparent-color`. No tiene flag A; todos los pixels son YJK puros y el chroma compartido haría que LMMC|AND/OR corrompa los pixels vecinos.
- **SCREEN 10 (`.SCA`)** sin `--transparent-color`: HMMC raw (flujo actual PRP022, sin cambios). Soporta SCAs all-paletted, all-YJK, o mixtos sin distinción.
- **SCREEN 10 (`.SCA`)** con `--transparent-color=N`: 0 ≤ N ≤ 15 (índice de paleta válido para A=1). Si N>15, error.
- El comando emite **2 secuencias V9938Cmd+V9938CmdData** por rectángulo (LMMC|AND para mask, LMMC|OR para image), idéntico al patrón PRP024.
- Cap de tamaño por chunk: igual que PRP022/024, `CHUNK_CMDDATA_MAX=2040` para los datos comprimidos, `CHUNK_PLETTER_MAX_UNCOMP=11264` para uncomp con Pletter (limitado por la VRAM scratch del engine).

## Diseño técnico

### 1. Cambios en `bin/imgwizard.php`

#### 1.1 Relajación del check de modo en `compressV9938Rectangle()`

[bin/imgwizard.php:1240-1250](../bin/imgwizard.php#L1240-L1250):

```php
// Antes (PRP024):
if ($transparent >= 0) {
    if ($sup == 'A' || $sup == 'C') {
        die("\nERROR: --transparent-color is not supported in SC10/SC12 (YJK).\n");
    }
    $maxColor = (1 << $bppMode[$sup]) - 1;
    if ($transparent > $maxColor) { die(...); }
    echo "### Transparent color: $transparent (LMMC AND+OR streaming)\n";
}

// Después (PRP026):
if ($transparent >= 0) {
    if ($sup == 'C') {
        die("\nERROR: --transparent-color is not supported in SC12 (YJK pure mode, no A flag).\n".
            "       Use SCREEN 10 (.SCA) instead, which supports paletted+YJK mixed mode.\n\n");
    }
    if ($sup == 'A') {
        // SCA: 16 colores paletados (índice 0-15) + YJK auto-preservado.
        if ($transparent > 15) {
            die("\nERROR: transparent color $transparent out of range for SC10 paletted (0..15)...\n\n");
        }
        echo "### Transparent color: $transparent (SCA paletted; YJK regions auto-preserved)\n";
    } else {
        $maxColor = (1 << $bppMode[$sup]) - 1;
        if ($transparent > $maxColor) {
            die("\nERROR: transparent color $transparent out of range for SC".hexdec($sup)." (0..$maxColor)...\n\n");
        }
        echo "### Transparent color: $transparent (LMMC AND+OR streaming)\n";
    }
}
```

#### 1.2 Caso SCA en `processTransparency()`

La función actual ([bin/imgwizard.php:1097-1132](../bin/imgwizard.php#L1097-L1132)) usa `$bppMode = ['5'=>4,'6'=>2,'7'=>4,'8'=>8]` para extraer pixels packed. SCA es 1 byte/pixel (no packed) y requiere lógica distinta basada en el bit A **y** preservación de bits 2-0 (chroma sharing del grupo YJK). Refactor con rama dedicada:

```php
function processTransparency($rectData, $w, $h, $sup, $transparent)
{
    if ($sup === 'A') {
        return processTransparencySCA($rectData, $w, $h, $transparent);
    }
    // ... rama original SC5/6/7/8 (packed pixels)
}

function processTransparencySCA($rectData, $w, $h, $transparent)
{
    // SCA layout per byte: bits 7-4 = Y/idx, bit 3 = A flag, bits 2-0 = K/J chroma
    // contribution shared across the 4-byte aligned horizontal group.
    //
    // Transparency rules:
    //   A=0 (YJK source pixel)    → preserve dest fully (mask=0xFF, image=0x00)
    //   A=1, idx == transparent   → preserve dest fully (mask=0xFF, image=0x00)
    //   A=1, idx != transparent   → overwrite Y/idx+A bits (7-3), preserve dest's
    //                                bits 2-0 to keep YJK group chroma intact:
    //                                  mask  = 0x07  (preserve dest bits 2-0)
    //                                  image = byte_orig & 0xF8  (zero own bits 2-0
    //                                          so the OR doesn't disturb the
    //                                          preserved-via-AND chroma bits)
    $mask  = '';
    $image = '';
    $yjkCount = 0;
    $transCount = 0;
    $visibleCount = 0;
    $totalBytes = strlen($rectData);

    for ($i = 0; $i < $totalBytes; $i++) {
        $byte = ord($rectData[$i]);
        if (($byte & 0x08) === 0) {
            // YJK pixel in source — never drawn (chroma sharing prevents safe overlay)
            $mask  .= chr(0xFF);
            $image .= chr(0x00);
            $yjkCount++;
        } else {
            $paletteIdx = ($byte >> 4) & 0x0F;
            if ($paletteIdx === $transparent) {
                // Paletted-transparent — preserve dest
                $mask  .= chr(0xFF);
                $image .= chr(0x00);
                $transCount++;
            } else {
                // Paletted-visible — overwrite bits 7-3, preserve dest's bits 2-0
                // (chroma contribution to the YJK group at this byte position)
                $mask  .= chr(0x07);
                $image .= chr($byte & 0xF8);
                $visibleCount++;
            }
        }
    }
    echo "    [SCA stats: $visibleCount visible, $transCount paletted-transparent, $yjkCount YJK auto-preserved]\n";
    if ($visibleCount === 0) {
        echo "WARNING: no visible paletted pixels in rectangle.\n";
    }
    return [$mask, $image];
}
```

#### 1.3 LMMC dot-mode en SCA

En [bin/imgwizard.php:1267-1277](../bin/imgwizard.php#L1267-L1277), el path con transparencia ya construye `buildV9938CmdSequence(..., $w, $h, ..., VDP_LMMC | LOG_AND/OR)` pasando coords en píxeles. SCA es 1 byte/pixel, así que `DX/NX` en píxeles == bytes; no requiere conversión adicional.

El opcode `VDP_LMMC | LOG_AND` y `VDP_LMMC | LOG_OR` funcionan idéntico en SCA paletado que en SC8 (ambos 8bpp = 1 byte/pixel desde el punto de vista del comando).

#### 1.4 Documentación en `showSyntax()`

Actualizar la sección `--transparent-color` con la nota específica para SCA:

```
" --transparent-color=N\n".
"               Optional flag for 'cx[l]': color index N treated as transparent.\n".
"               Generates 2-pass LMMC AND+OR streaming instead of 1-pass HMMC.\n".
"               Supported in: SC5 (0..15), SC6 (0..3), SC7 (0..15), SC8 (0..255),\n".
"                             SC10 (0..15 paletted; YJK pixels auto-preserved).\n".
"               NOT supported in SC12 (pure YJK, no per-pixel mode flag).\n".
```

### 2. Sin cambios en runtime / motor

- `src/daad_platform_msx2.c`: sin modificaciones. El despachador de chunks ya soporta secuencias V9938Cmd+V9938CmdData arbitrarias.
- `src/libs/utils_dzx0vram.s`, `utils_pletter2vram.s`, `utils_unrle_vram.s`: sin cambios. La lógica de descompresión a VRAM scratch + streaming LMMC es agnóstica al modo de pantalla.
- Build flags: ningún flag nuevo. El binario `-DSCREEN=10` existente reproduce los IMA generados sin recompilar.

### 3. Ningún cambio en el formato IMA

El IMA producido es estructuralmente idéntico al de PRP024 para SC8 con transparencia:

```
INFO chunk (chipset=V9938, pixelType=BD8/BP4 según SCA, palette=GRB333)
[Palette chunk si !lastPalette]
V9938Cmd  (1 cmd: LMMC | LOG_AND, DX,DY,NX,NY, CLR=mask[0])
V9938CmdData × M (compressorID, uncompSize, payload comprimido) → buffer mask[1..N-1]
V9938Cmd  (1 cmd: LMMC | LOG_OR,  DX,DY,NX,NY, CLR=image[0])
V9938CmdData × M (compressorID, uncompSize, payload comprimido) → buffer image[1..N-1]
[Palette chunk si lastPalette]
```

El `pixelType` reportado en INFO se mantiene como SCA (`BP4` en la tabla [bin/imgwizard.php:480](../bin/imgwizard.php#L480) — `'A'=>7`, valor reservado YJK+YAE; podría reasignarse pero no es necesario para este PRP).

## Casos de uso

### Caso 1: SC5 → SCA all-paletted con transparencia

```bash
imgwizard 5a SpriteHero.SC5 SpriteHero.SCA 96
imgwizard cx SpriteHero.SCA 0 0 256 96 rle --transparent-color=0
```

**Resultado**: `SpriteHero.IMA` con LMMC|AND+OR. Todos los pixels tienen A=1 (vienen de `5a`), por lo que ningún pixel YJK aparece; equivale exactamente a `cx SpriteHero.SC5 0 0 256 96 rle --transparent-color=0` pero renderizable en build `SCREEN=10`.

### Caso 2: SCA mixto (fondo YJK + sprite paletado)

```bash
imgwizard cx Background.SCA 0 0 256 212 pletter        # fondo YJK puro, 1 paso, sin transparencia
imgwizard cx Sprite.SCA 0 0 64 64 rle --transparent-color=15   # sprite mixto con transparencia
```

**Resultado**:
- `Background.IMA`: HMMC raw (PRP022 actual).
- `Sprite.IMA`: 2-pass LMMC. Las regiones YJK del sprite **no se dibujan** (preservan el fondo); las regiones paletadas con color 15 tampoco; el resto se dibuja como overlay nítido.

### Caso 3: SCC (SCREEN 12) sigue rechazado

```bash
imgwizard cx Photo.SCC 0 0 256 212 raw --transparent-color=0
# ERROR: --transparent-color is not supported in SC12 (YJK pure mode, no A flag).
#        Use SCREEN 10 (.SCA) instead, which supports paletted+YJK mixed mode.
```

## Mecánica V9938 explicada

Para cada byte del rectángulo destino tras la secuencia LMMC|AND + LMMC|OR:

| Caso source | Source mask | Source image | Resultado tras AND | Resultado tras OR | Bits 7-3 del dest | Bits 2-0 del dest |
|-------------|-------------|--------------|---------------------|-------------------|-------------------|-------------------|
| YJK (A=0) | 0xFF | 0x00 | dest & 0xFF = dest | dest \| 0x00 = dest | preservados | preservados |
| Paletted-transparent (A=1, idx=N) | 0xFF | 0x00 | dest & 0xFF = dest | dest \| 0x00 = dest | preservados | preservados |
| Paletted-visible (A=1, idx≠N) | **0x07** | byte_orig & 0xF8 | dest & 0x07 = chroma_dest | chroma_dest \| (byte_orig & 0xF8) | **sobrescritos** con sprite (Y/idx + A=1) | **preservados** del dest (chroma del grupo YJK intacta) |

**Resultado de overlay paletted-visible**: el bit A=1 + índice de paleta del source se transfieren correctamente, permitiendo que un destino YJK se "convierta" en una región paletada en los pixels visibles del sprite. Pero **los bits 2-0 del destino sobreviven**, manteniendo la codificación K/J del grupo de 4 pixels alineado al que pertenece este pixel — fundamental para que los pixels YJK vecinos (que NO se sobreescriben porque el source en sus posiciones era transparente o YJK) renderizen el chroma esperado.

## Chroma sharing en grupos de 4 — por qué la máscara debe ser 0x07

### El layout YJK+YAE de SC10 (Grauw)

Cada byte SCA contiene:
```
bit:    7  6  5  4   3   2  1  0
       +-----------+-----+--------+
       |   Y/idx   |  A  |  K/J   |
       +-----------+-----+--------+
```

Pero las componentes K (6 bits) y J (6 bits) de YJK **se reparten en grupos alineados de 4 bytes horizontales** (X mod 4 == 0), donde **los bits 2-0 de cada byte del grupo contribuyen al chroma**:

| Posición en el grupo | bits 2-0 contribuyen a |
|----------------------|------------------------|
| byte 0 (X mod 4 == 0) | K_low |
| byte 1 (X mod 4 == 1) | K_high |
| byte 2 (X mod 4 == 2) | J_low |
| byte 3 (X mod 4 == 3) | J_high |

con `K = K_low | (K_high << 3)` y `J = J_low | (J_high << 3)`, ambos signed 6-bit (-32..31).

### Punto crítico

El flag A solo determina si **ese pixel concreto** se renderiza como paletado (ignora chroma propio) o como YJK. **NO desactiva la contribución de los bits 2-0 del byte al encoding K/J usado por los OTROS pixels YJK del mismo grupo**.

Es decir: en un grupo `[paletted, yjk, yjk, yjk]`, los 3 pixels YJK leen K_low de los bits 2-0 del byte paletado. Si destruimos esos bits 2-0 al hacer overlay → **chroma erróneo en los 3 vecinos YJK**.

### Si usáramos mask=0x00 (diseño naive)

```
dest = (dest & 0x00) | byte_orig = byte_orig
```
Los bits 2-0 del destino se sustituyen por los de `byte_orig` (típicamente 0, porque el autor del SCA paletado no los considera relevantes). Resultado: **K_low del grupo destino se convierte en 0**, todos los YJK vecinos sufren un cambio de chroma drástico.

### Con mask=0x07, image=byte_orig & 0xF8 (diseño correcto)

```
dest = (dest & 0x07) | (byte_orig & 0xF8)
```
- Los bits 7-3 del destino reciben Y/idx + A=1 del sprite (este pixel pasa a paletado).
- Los bits 2-0 del destino se preservan → la contribución a K_low/K_high/J_low/J_high del grupo queda intacta.
- Los pixels YJK vecinos del grupo (no overlaid, source los marcó transparente o YJK) renderizan el chroma original sin halos de color.

### Limitación residual inherente a YJK+YAE

Cuando un grupo de 4 pixels destino tiene `[overlaid_paletted, original_yjk, original_yjk, overlaid_paletted]` (overlay parcial), el pixel paletado sobreescrito ya **no contribuye a chroma como YJK** (su A=1 ahora). Pero su contribución a `bits 2-0` sigue válida (preservada del destino original). Los YJK vecinos perciben el mismo chroma que antes del overlay. 

El único caso donde esto degrada visualmente es si el destino completo del grupo era YJK y se overlaid solo 1-2 pixels — los pixels YJK restantes ven menos "fuente de chroma común" pero los bits 2-0 del grupo siguen formando el mismo K/J. **No hay corrupción**, solo un efecto visual perceptual menor en bordes — inherente a YJK overlays y minimizado por el diseño.

## Validación

### Tests manuales en openMSX (build `-DSCREEN=10`)

1. Generar un asset SC5 de 256×96 con un personaje sobre fondo de color 0.
2. Convertirlo: `5a` → SCA all-paletted, luego `cx ... --transparent-color=0`.
3. Boot openMSX con build SC10: verificar que el sprite se dibuja sobre el fondo previamente cargado (LOADING.IMA o pintado con CLS) sin halo y sin pixels color 0 visibles.

### Test de no-regresión

- Builds existentes para SC5/SC6/SC7/SC8 no deben verse afectados (solo cambia la rama `'A'` en `processTransparency`).
- `cx in.SCA ...` sin `--transparent-color` debe seguir produciendo HMMC raw idéntico al actual (path PRP022 inalterado).
- `cx in.SCC ... --transparent-color=N` debe abortar con error claro.

### Test específico de chroma preservation (PRP026 critical)

Crear un SCA mixto con un grupo destino de 4 pixels alineados:
- byte 8: YJK con K_low=0b011
- byte 9: YJK con K_high=0b101
- byte 10: YJK con J_low=0b010
- byte 11: YJK con J_high=0b100

→ K = 0b101_011 = 43, J = 0b100_010 = 34.

Luego overlaid con un sprite paletado en byte 9 (color 12):
- Source byte 9 = `(12 << 4) | 0x08 = 0xC8` (bits 2-0 = 000).

Tras overlay con refined mask=0x07, image=0xC8&0xF8=0xC8:
- dest[9] = (orig[9] & 0x07) | 0xC8

Si orig[9] tenía bits 2-0 = 0b101 (K_high), el resultado:
- dest[9] = 0b101 | 0xC8 = 0xCD → A=1, idx=12, **bits 2-0 = 101** (K_high preservado)

Verificar visualmente en openMSX que los pixels YJK vecinos (8, 10, 11) NO cambian de chroma respecto al destino original. Compararlo contra una versión con mask=0x00 (naive) que SÍ debería mostrar halo de color.

### Test del cap de uncomp por chunk

Pletter sobre buffer SCA all-paletted: el mask es bicolor (0x00/0xFF) → comprime ~99%. El image es similar a la imagen original. Comprobar que `CHUNK_PLETTER_MAX_UNCOMP=11264` se respeta (ningún chunk supera 16KB de uncomp).

## Trade-offs

| Pro | Con |
|-----|-----|
| Cero cambios en runtime; build SC10 existente reproduce IMAs sin recompilar | El IMA con transparencia es ~2× más grande que sin transparencia (2-pass) |
| Mecánica idéntica a PRP024 en SC5/7/8 para los casos transparent/preserve, **distinta solo en el paletted-visible** (mask=0x07 en lugar de 0x00) | Solo aplica si el autor produce SCAs all-paletted o mixtos; SCAs YJK puros no se benefician |
| Workflow `5a` + `cx --transparent-color=N` cubre la mayoría de los casos prácticos | Requiere que el usuario entienda la diferencia entre SC10 (mixed) y SC12 (pure YJK) |
| Permite SCAs con sprite paletado sobre fondo YJK realista, **sin halos de color en los vecinos YJK** gracias a la preservación de bits 2-0 | El SCA all-YJK no soporta transparency (igual que SCC) |
| Mask buffer comprime extremadamente bien (típicamente solo 2 valores: 0xFF / 0x07) → Pletter/RLE >95% reducción | Image buffer pierde los bits 2-0 (= 0 forzado) — ligeramente menos compressible que la versión "byte completo", pero los autores no aprovechan estos bits en SCA paletado |

## Coste estimado

| Cambio | LOC PHP | Riesgo |
|--------|---------|--------|
| Caso `'A'` en `processTransparency` (rama nueva) | ~25 | Nulo |
| Relajar check en `compressV9938Rectangle` | ~10 | Nulo |
| Documentación en `showSyntax` | ~3 | Nulo |
| **Total** | **~38 líneas PHP** | **Cero modificaciones runtime/engine** |

## Plan de implementación

1. Refactor `processTransparency` para extraer la lógica SC5/6/7/8 a una función auxiliar y añadir rama dedicada SCA (`processTransparencySCA`) **con mask=0x07/image=byte&0xF8 para paletted-visible**.
2. Relajar check en `compressV9938Rectangle` con mensajes específicos por modo.
3. Actualizar `showSyntax` con la nota sobre SCA.
4. Generar dos assets de prueba:
   - **Caso A (all-paletted)**: `assets/TestPalSprite.SC5` (16-color) → `5a` → `TestPalSprite.SCA` → `cx --transparent-color=0` → `TestPalSprite.IMA`.
   - **Caso B (mixed YJK + paletted overlay)**: SCA destino con fondo YJK (vía `ca` desde SCC) + SCA source con sprite paletado. Crear un IMA que overlaid el sprite sobre el fondo previamente cargado y verificar:
     - los pixels YJK del fondo NO cambian de chroma cerca del sprite,
     - los pixels paletados-transparentes del sprite preservan el fondo,
     - los pixels paletados-visibles del sprite se dibujan correctamente.
5. Probar en openMSX con build `make CXXFLAGS="-DLANG_ES -DMSX2 -DDAADV3 -DSCREEN=10 ..." clean all` y verificar render correcto en ambos casos.
6. Documentar en este PRP los resultados de validación + screenshots, especialmente el caso B con/sin la preservación de bits 2-0 (comparativa visual del halo de color).

## Referencias

- [PRP022 — ImgWizard V9938 chunks](PRP022_ImgWizard_V9938_Chunks.md)
- [PRP024 — CX[L] transparency LMMC AND+OR](PRP024_CXL_Transparency.md)
- [PRP025 — DZX0 VRAM decompressor](PRP025_DZX0_VRAM_Decompressor.md)
- [The YJK screen modes - MSX Assembly Page (Grauw)](https://map.grauw.nl/articles/yjk.php) — descripción del bit A en YJK+YAE
- [V9938 Technical Data Book §6.5.5 LMMC](https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Chapter4b.md#655-lmmc) — operaciones lógicas válidas en SC10
- [src/daad_platform_msx2.c:486-487](../src/daad_platform_msx2.c#L486) — configuración R#25 = 0x18 (YJK+YAE) en runtime
- [src/daad_platform_msx2.c:744-756](../src/daad_platform_msx2.c#L744-L756) — uso existente de LMMC|TAND/LMMM|TIMP/LMMV|TAND en SC10 paletado en el motor (validación empírica de viabilidad)

---

## Anexo A — Resumen visual de los buffers mask/image

Esta sección resume de forma compacta los valores que `processTransparencySCA` debe emitir por pixel, pensada como referencia rápida para implementadores y revisores.

### A.1 Pase 1: LMMC|AND — buffer de máscara

| Caso del pixel source | Byte máscara | Binario | Efecto del `dest & mask` |
|------------------------|--------------|---------|--------------------------|
| A=0 (YJK) → preservar destino | `0xFF` | `0b11111111` | Todos los bits destino preservados |
| A=1 ∧ idx=N (paletted-transparent) → preservar destino | `0xFF` | `0b11111111` | Todos los bits destino preservados |
| A=1 ∧ idx≠N (paletted-visible) → sobrescribir Y/idx+A, preservar chroma | `0x07` | `0b00000111` | Bits 7-3 destino → 0 (limpios para el OR); bits 2-0 destino preservados (chroma del grupo YJK) |

### A.2 Pase 2: LMMC|OR — buffer de imagen

Donde `pppp` = índice de paleta del pixel source (4 bits, valor 0-15).

| Caso del pixel source | Byte imagen | Binario | Efecto del `dest \| image` |
|------------------------|-------------|---------|----------------------------|
| A=0 (YJK) → no escribir | `0x00` | `0b00000000` | Destino sin modificar |
| A=1 ∧ idx=N (paletted-transparent) → no escribir | `0x00` | `0b00000000` | Destino sin modificar |
| A=1 ∧ idx≠N (paletted-visible, color=p) → escribir paleta+A=1, no tocar chroma | `(p<<4) \| 0x08` | `0bpppp1000` | Bits 7-3 → `pppp1` (idx + A=1); bits 2-0 sin tocar (image=000 ya, OR no modifica) |

### A.3 Resultado neto por pixel tras los 2 pases

| Caso | bits 7-4 finales | bit 3 final | bits 2-0 finales | Render |
|------|------------------|-------------|------------------|--------|
| YJK source | preservados (Y o idx del dest) | preservado (A del dest) | preservados (chroma dest) | dest sin cambios |
| Paletted-transparent source | preservados | preservado | preservados | dest sin cambios |
| Paletted-visible source (color p) | `pppp` (sprite) | `1` (sprite, A=1) | **preservados (chroma dest)** | paletted con `palette[p]`; bits 2-0 mantienen contribución a K/J del grupo YJK destino |

### A.4 Ejemplo aritmético

Destino con 4 bytes consecutivos de un grupo YJK alineado (X=8..11). Layout binario: `Yyyy_A_kkk` (4 bits Y/idx + 1 bit A + 3 bits chroma):
```
dest[8]  = 0b0100_0_011  (Y=4, A=0, K_low =011)  → 0x43
dest[9]  = 0b0011_0_101  (Y=3, A=0, K_high=101)  → 0x35
dest[10] = 0b0010_0_010  (Y=2, A=0, J_low =010)  → 0x22
dest[11] = 0b0001_0_100  (Y=1, A=0, J_high=100)  → 0x14
```
→ K = `101_011` = 43, J = `100_010` = 34.

Source: sprite paletado en el pixel 9, color p=12 (`0b1100`). Mask=`0x07`=`0b0000_0_111`, image=`(12<<4)|0x08` = `0b1100_1_000` = `0xC8`.

Tras LMMC|AND: `dest[9] & 0x07 = 0b0011_0_101 & 0b0000_0_111 = 0b0000_0_101`.
Tras LMMC|OR: `0b0000_0_101 | 0b1100_1_000 = 0b1100_1_101`.

Resultado: `dest[9] = 0xCD` → A=1, idx=12, **bits 2-0 = 101 (K_high preservado)**.

Render del grupo:
- byte 8: YJK, K_low=011 (preservado).
- byte 9: paletted, `palette[12]` (sprite color).
- byte 10: YJK, J_low=010 (preservado).
- byte 11: YJK, J_high=100 (preservado).

→ K del grupo = 43 (intacto), J = 34 (intacto). Los pixels YJK vecinos (8, 10, 11) renderizan exactamente el mismo color que antes del overlay. El pixel 9 muestra el color paletado del sprite. **Sin halo de chroma. Comportamiento ideal.**

### A.5 Pseudocódigo final del generador

```
para cada byte src en el rectángulo:
    si (src & 0x08) == 0:                 # YJK
        mask  ← 0xFF
        image ← 0x00
    sino:                                 # paletted (A=1)
        idx ← (src >> 4) & 0x0F
        si idx == transparent_color:
            mask  ← 0xFF
            image ← 0x00
        sino:
            mask  ← 0x07
            image ← src & 0xF8            # equivalente a (idx<<4) | 0x08 con bits 2-0 = 0
```
