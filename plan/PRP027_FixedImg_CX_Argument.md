# PRP027 — `--fixed=X,Y` para `cx[l]`: chunk FIXEDIMG con offset global

## Contexto

El motor MSX2DAAD ya soporta el chunk `IMG_CHUNK_FIXEDIMG` (tipo 129) introducido recientemente en [include/daad_platform_msx2.h:228](../include/daad_platform_msx2.h#L228) y manejado en [src/daad_platform_msx2.c:914-920](../src/daad_platform_msx2.c#L914-L920). Cuando se encuentra ese chunk, el motor sobreescribe `gfxWinOffsetX`/`gfxWinOffsetY` con los valores del chunk, de modo que los siguientes `V9938Cmd` (HMMC/LMMC) se dibujen en una posición **fija respecto a la pantalla**, ignorando la ventana DAAD activa.

Caso de uso: imágenes que deben aparecer siempre en una zona concreta de la pantalla (UI, marco, sprite con posición absoluta) sin que la ventana DAAD actual desplace el dibujo. PRP022/024/026 producen chunks V9938 cuyo `DX/DY` se suma al `gfxWinOffset` de la ventana actual ([src/daad_platform_msx2.c:931-932](../src/daad_platform_msx2.c#L931-L932)).

Falta el lado de `imgwizard.php`: el comando `cx`/`cxl` no genera todavía el chunk `FIXEDIMG`. Este PRP añade la opción CLI `--fixed=X,Y` para producirlo automáticamente.

## Objetivo

Añadir el flag opcional `--fixed=X,Y` al comando `cx`/`cxl` de `bin/imgwizard.php`. Cuando se especifica:

1. **Validar** que `X + W ≤ ScreenWidth` y `Y + H ≤ ScreenHeight`, donde:
   - `ScreenWidth` = 256 para SC5/SC8/SC10/SC12, **512** para SC6/SC7.
   - `ScreenHeight` = 212 (todos los modos bitmap MSX2/MSX2+).
   - Si la validación falla, abortar con un error descriptivo (que incluya: el modo, X, Y, W, H, los límites, y la dimensión que se excedió).
2. **Emitir** un chunk binario `IMG_CHUNK_FIXEDIMG` (tipo 129) **inmediatamente después** del INFO chunk y **antes** de cualquier paleta o V9938Cmd. Layout:
   ```
   Offset Size  Description
   0x00   1     Chunk type = 129 (CHUNK_FIXEDIMG)
   0x01   2     Extra header size = 4
   0x03   2     Data size = 0
   0x05   2     offsetX (uint16, little-endian) = X
   0x07   2     offsetY (uint16, little-endian) = Y
   ```

Sin `--fixed=X,Y`, el flujo `cx[l]` actual queda inalterado (no se emite chunk FIXEDIMG → `gfxWinOffset` se mantiene en el valor de la ventana DAAD activa).

## Por qué este diseño

- **Coherencia con el motor**: el chunk ya está definido en `daad_platform_msx2.h` con `extraHeaderSize=4, dataSize=0` y dos `uint16_t` (offsetX, offsetY). El generador en PHP debe emitir ese mismo layout exacto.
- **Posición del chunk en el IMA**: el motor procesa los chunks en orden secuencial. `FIXEDIMG` debe leerse **antes** de los `V9938Cmd` para que `gfxWinOffsetX/Y` ya esté actualizado cuando los comandos se dispatchen. La posición natural es justo tras el INFO (que es siempre el primer chunk).
- **No afecta a paleta**: la paleta (chunk 1) carga colores en VDP, no usa `gfxWinOffset`. Puede ir antes o después de `FIXEDIMG` sin diferencia. Por simplicidad y orden de lectura, se coloca FIXEDIMG **antes** de la paleta.
- **Validación temprana**: comprobar `X+W` y `Y+H` antes de generar cualquier byte del IMA evita producir un fichero inválido que el motor podría dibujar fuera de pantalla.

## Restricciones

- **Solo aplica a `cx[l]`**. Los comandos legacy `c[l]` y `s` (DEPRECATED desde PRP024) no soportan `--fixed`.
- **`X` e `Y` deben ser enteros no negativos**. Si se especifican como negativos, error.
- **Validación de bounds estricta**: `X+W > ScreenWidth` o `Y+H > ScreenHeight` aborta con código de salida no-cero.
- **Sin chunk FIXEDIMG si no se pasa el flag**: comportamiento backward-compatible.
- **Coexiste con `--transparent-color=N`**: ambos flags son independientes y pueden combinarse.

## Diseño técnico

### 1. Cambios en `bin/imgwizard.php`

#### 1.1 Constantes nuevas

Añadir junto a las demás `CHUNK_*` ([bin/imgwizard.php:128-129](../bin/imgwizard.php#L128-L129)):

```php
define('CHUNK_FIXEDIMG',  129);   // [PRP027] Fixed-position image (sets gfxWinOffsetX/Y)
```

#### 1.2 Parser CLI en el handler de `cx`/`cxl`

[bin/imgwizard.php:315-327](../bin/imgwizard.php#L315-L327): añadir parsing del flag `--fixed=X,Y` junto al de `--transparent-color`:

```php
$compress    = "RLE";
$transparent = -1;
$fixedX      = -1;        // -1 = no FIXEDIMG chunk
$fixedY      = -1;
for ($i = 7; $i < $argc; $i++) {
    $arg = $argv[$i];
    if (strpos($arg, "--transparent-color=") === 0) {
        $val = substr($arg, strlen("--transparent-color="));
        if (!is_numeric($val) || intval($val) < 0) {
            echo "ERROR: --transparent-color requires a non-negative numeric value...\n";
            exit;
        }
        $transparent = intval($val);
    } else if (strpos($arg, "--fixed=") === 0) {
        $val = substr($arg, strlen("--fixed="));
        $parts = explode(",", $val);
        if (count($parts) !== 2 || !is_numeric($parts[0]) || !is_numeric($parts[1])
            || intval($parts[0]) < 0 || intval($parts[1]) < 0) {
            die("\nERROR: --fixed=X,Y requires two non-negative integers (e.g. --fixed=64,32).\n\n");
        }
        $fixedX = intval($parts[0]);
        $fixedY = intval($parts[1]);
    } else {
        $compress = strtoupper($arg);
    }
}
// ... resto del handler ...
compressV9938Rectangle($fileIn, $x, $y, $w, $h, $comp, $transparent, $fixedX, $fixedY);
```

#### 1.3 Validación de bounds y emisión del chunk en `compressV9938Rectangle()`

Firma actualizada (añadir `$fixedX = -1, $fixedY = -1`):

```php
function compressV9938Rectangle($file, $x, $y, $w, $h, $comp, $transparent=-1, $fixedX=-1, $fixedY=-1)
```

Tras `checkScreemMode()` y antes de leer el rectángulo, validar bounds si `$fixedX >= 0`:

```php
if ($fixedX >= 0 && $fixedY >= 0) {
    // Tabla ScreenWidth (en píxeles) por modo
    $screenWidth  = ['5'=>256, '6'=>512, '7'=>512, '8'=>256, 'A'=>256, 'C'=>256];
    $screenHeight = 212;
    $sw = $screenWidth[$sup];
    if ($fixedX + $w > $sw) {
        die("\nERROR: --fixed X+W out of bounds for SC".hexdec($sup).":\n".
            "       fixedX=$fixedX + width=$w = ".($fixedX+$w)." > ScreenWidth=$sw\n\n");
    }
    if ($fixedY + $h > $screenHeight) {
        die("\nERROR: --fixed Y+H out of bounds for SC".hexdec($sup).":\n".
            "       fixedY=$fixedY + height=$h = ".($fixedY+$h)." > ScreenHeight=$screenHeight\n\n");
    }
    echo "### Fixed image position: ($fixedX, $fixedY) — chunk FIXEDIMG will be emitted\n";
}
```

#### 1.4 Inserción del chunk FIXEDIMG en el ensamblado del IMA

Tras el placeholder INFO y antes de `$cmdChunksBin`/paleta, generar el chunk si procede. Layout binario:

```php
$fixedImgBin = "";
if ($fixedX >= 0 && $fixedY >= 0) {
    // Chunk type=129, extraHeaderSize=4, dataSize=0
    // Extra header: offsetX (uint16 LE), offsetY (uint16 LE)
    $fixedImgBin = chr(CHUNK_FIXEDIMG)
                 . pack("vv", 4, 0)            // extraHeaderSize=4, dataSize=0
                 . pack("vv", $fixedX, $fixedY);  // offsetX, offsetY
}

$out  = $magic.$scr;
$infoPos = strlen($out);
$out .= str_repeat("\0", 15);                   // INFO placeholder (rellenado al final)
$out .= $fixedImgBin;                           // FIXEDIMG (si aplica) — antes de paleta y cmds
if ($palBin && !$lastPalette) $out .= $palBin;
$out .= $cmdChunksBin;
if ($palBin && $lastPalette)  $out .= $palBin;
```

#### 1.5 Soporte en `showImageContent()` (comando `l`)

Para que `imgwizard l fichero.IMA` muestre el chunk legible. Añadir un `case` en el switch ([bin/imgwizard.php:577-665](../bin/imgwizard.php#L577-L665)):

```php
case CHUNK_FIXEDIMG:
    $offX = unpack("v", substr($in, $pos+5, 2))[1];
    $offY = unpack("v", substr($in, $pos+7, 2))[1];
    echo "    CHUNK $id: FIXEDIMG offsetX=$offX offsetY=$offY\n";
    $size += $sin + $sout;
    break;
```

Y en la lógica de actualización de tamaño para chunks v2 ([bin/imgwizard.php:509](../bin/imgwizard.php#L509)):

```php
if ($type==CHUNK_INFO || $type==CHUNK_V9938CMD || $type==CHUNK_V9938DATA || $type==CHUNK_FIXEDIMG) {
    $pos += 5 + $sin + $sout;
}
```

#### 1.6 Actualizar `showSyntax()`

Añadir entrada para `--fixed=X,Y`:

```php
" --fixed=X,Y\n".
"               Optional flag for 'cx[l]': emit a FIXEDIMG chunk with global pixel\n".
"               offset (X,Y) so the image renders at a fixed screen position\n".
"               regardless of the active DAAD window. Engine sets gfxWinOffsetX/Y.\n".
"               Bounds: X+W <= 256 (SC5/SC8/SC10/SC12) or 512 (SC6/SC7); Y+H <= 212.\n".
```

### 2. Sin cambios en runtime / motor

- `src/daad_platform_msx2.c`: ya maneja `IMG_CHUNK_FIXEDIMG` (líneas 914-920 en el commit staged).
- `include/daad_platform_msx2.h`: ya define `IMG_CHUNK_FIXEDIMG=129` y la struct `IMG_FIXEDIMG`.
- Build flags: ninguno nuevo. El binario actual reproduce IMA con FIXEDIMG sin recompilar.

### 3. Sin cambios en otros comandos

- `c`/`cl`/`s` (DEPRECATED): no soportan `--fixed`. Si el usuario lo intenta, el flag se ignora (no se parsea en sus handlers). Decisión consciente: nuevo flag = solo para flujo nuevo.

## Casos de uso

### Caso 1: Sprite fijo en (64, 32)

```bash
imgwizard cx Sprite.SC5 0 0 64 64 rle --fixed=64,32 --transparent-color=0
```

**Resultado**: `Sprite.IM5` con orden de chunks: INFO, **FIXEDIMG(64,32)**, PALETTE, V9938Cmd(LMMC|AND), V9938CmdData×N, V9938Cmd(LMMC|OR), V9938CmdData×N. El motor renderiza el sprite siempre en pantalla absoluta (64,32), independiente de la ventana DAAD activa.

### Caso 2: Imagen fullscreen sin fixed (comportamiento actual)

```bash
imgwizard cx Background.SC8 0 0 256 212 pletter
```

**Resultado**: `Background.IM8` SIN chunk FIXEDIMG. El motor usa el `gfxWinOffsetX/Y` de la ventana actual (típicamente 0,0 si la window cubre toda la pantalla).

### Caso 3: Bounds excedidos → error

```bash
imgwizard cx Sprite.SC5 0 0 200 50 rle --fixed=100,200
# ERROR: --fixed Y+H out of bounds for SC5:
#        fixedY=200 + height=50 = 250 > ScreenHeight=212
```

```bash
imgwizard cx Wide.SC5 0 0 200 50 rle --fixed=80,0
# ERROR: --fixed X+W out of bounds for SC5:
#        fixedX=80 + width=200 = 280 > ScreenWidth=256
```

### Caso 4: SC6/SC7 con ScreenWidth=512

```bash
imgwizard cx Hires.SC7 0 0 384 100 rle --fixed=128,50
# OK: 128+384=512 ≤ 512, 50+100=150 ≤ 212
```

## Validación

### Tests unitarios (manual, vía CLI)

1. **Generación correcta**: `cx ... --fixed=10,20 ...` produce IMA con chunk FIXEDIMG en posición 2 (tras INFO). Verificar con `imgwizard l fichero.IMA` que muestra `CHUNK 2: FIXEDIMG offsetX=10 offsetY=20`.
2. **Sin flag**: `cx ... ...` (sin `--fixed`) NO produce chunk FIXEDIMG. Verificar con `imgwizard l`.
3. **Bounds X+W**: `cx Img.SC5 0 0 200 50 rle --fixed=100,0` → error.
4. **Bounds Y+H**: `cx Img.SC5 0 0 100 100 rle --fixed=0,150` → error.
5. **Bounds OK**: `cx Img.SC5 0 0 64 32 rle --fixed=192,180` → OK (64+192=256≤256, 32+180=212≤212).
6. **Combinación con `--transparent-color`**: `cx Img.SC5 0 0 64 64 rle --fixed=10,20 --transparent-color=0` → IMA con FIXEDIMG + 2-pass LMMC AND+OR.
7. **SC6/SC7 (W=512)**: `cx Img.SC7 0 0 256 100 rle --fixed=200,50` → OK (200+256=456≤512).

### Test de no-regresión

- `cx in.SC8 0 0 256 212 rle` (sin `--fixed`) genera IMA bit-equivalente al actual.
- `cx in.SC5 ... --transparent-color=N` (sin `--fixed`) genera IMA bit-equivalente al actual.
- Comandos `c`/`cl`/`s` siguen funcionando sin cambios.

### Test runtime en openMSX

1. Crear un asset SC5 y generar `IMA1.IM5` sin `--fixed` (renderiza en window activa).
2. Generar `IMA2.IM5` con `--fixed=80,40` (renderiza en (80,40) absoluto).
3. Cargar ambos en una build `make CXXFLAGS="-DSCREEN=5 ..." clean all` y verificar visualmente la diferencia de posicionamiento.

## Coste estimado

| Cambio | LOC PHP | Riesgo |
|--------|---------|--------|
| Constante `CHUNK_FIXEDIMG=129` | 1 | Nulo |
| Parser CLI en handler `cx` | ~12 | Nulo |
| Validación bounds + emisión chunk en `compressV9938Rectangle` | ~25 | Nulo |
| Soporte en `showImageContent` (comando `l`) | ~6 | Nulo |
| Documentación en `showSyntax` | ~5 | Nulo |
| **Total** | **~50 líneas PHP** | **Cero modificaciones runtime/engine** (engine ya soporta el chunk) |

## Plan de implementación

1. Añadir constante `CHUNK_FIXEDIMG`.
2. Parser de `--fixed=X,Y` en el handler de `cx`/`cxl`. Pasar `$fixedX, $fixedY` a `compressV9938Rectangle`.
3. Validación de bounds en `compressV9938Rectangle` (tras `checkScreemMode`, antes de leer el rectángulo).
4. Generar `$fixedImgBin` y concatenarlo en el IMA tras INFO y antes de paleta/cmds.
5. Añadir caso `CHUNK_FIXEDIMG` en `showImageContent` y en el cálculo de tamaño v2.
6. Documentar en `showSyntax`.
7. Validar con CLI: invocaciones positivas, casos de bounds excedidos, `imgwizard l` lista el chunk correctamente.
8. Validar runtime en openMSX con un IMA `--fixed`: ver que la imagen aparece en posición absoluta independiente de la window DAAD.

## Referencias

- [PRP022 — ImgWizard V9938 chunks](PRP022_ImgWizard_V9938_Chunks.md) — formato V2 (.IMx) con INFO + V9938Cmd + V9938CmdData
- [PRP024 — CX[L] transparency](PRP024_CXL_Transparency.md) — flag `--transparent-color`
- [PRP026 — SC10 paletted+YJK transparency](PRP026_SC10_Paletted_Transparency.md) — extensión para SCREEN 10
- [include/daad_platform_msx2.h:228](../include/daad_platform_msx2.h#L228) — `IMG_CHUNK_FIXEDIMG=129`
- [include/daad_platform_msx2.h:260-266](../include/daad_platform_msx2.h#L260-L266) — struct `IMG_FIXEDIMG`
- [src/daad_platform_msx2.c:914-920](../src/daad_platform_msx2.c#L914-L920) — handler runtime que actualiza `gfxWinOffsetX/Y`
- [src/daad_platform_msx2.c:931-932](../src/daad_platform_msx2.c#L931-L932) — uso de `gfxWinOffset` al sumar a `DX/DY` de los V9938Cmd
