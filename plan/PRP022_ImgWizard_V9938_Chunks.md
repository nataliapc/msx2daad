# PRP022 — ImgWizard: Chunk Info + V9938Cmd + V9938CmdData (comando `CX[L]`)

- **ID**: PRP022
- **Tipo**: Implementación (formato de imagen IMx — herramienta host)
- **Ficheros afectados**:
  - [`bin/imgwizard.php`](../bin/imgwizard.php) — generación + listado de los 3 chunks nuevos, comando `CX[L]`
  - [`include/daad_platform_msx2.h`](../include/daad_platform_msx2.h) — alineación de `IMG_INFO` con el formato v1 de imgwizard
  - [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c) — implementar handler `IMG_CHUNK_CMD` (dispara cada cmd con `fastVCopy` ya existente); `IMG_CHUNK_CMDDATA` queda como **skip-only** (consume bytes para no descuadrar offset) hasta un PRP futuro que diseñe descompresores con sumidero a port `#9B` sin buffer en RAM
- **Severidad**: FEATURE — los 3 chunks (`INFO=128`, `V9938Cmd=20`, `V9938CmdData=21`) están **declarados** en imgwizard.php (cabecera de comentarios) y en el header del intérprete pero **no se generan** desde la herramienta y los marcadores `**NOT IMPLEMENTED**` siguen presentes
- **Fecha**: 2026-04-25
- **Estado**: Pendiente

> **Alcance**: este PRP cubre la generación completa de los chunks desde `imgwizard.php` y, en el engine, la ejecución del **`V9938Cmd`** vía `fastVCopy()` ya existente. La ejecución del **`V9938CmdData`** (descompresión + envío al VDP) **queda diferida a un PRP posterior**: una implementación naive descomprimiría a RAM y volcaría al puerto `#9B`, pero el coste en RAM es prohibitivo para MSX-DOS1 (hasta varios KB por chunk). El diseño adecuado pasa por nuevas variantes de los descompresores (`unRLE`, `pletter2ram`, `copyToVRAM`) que vuelquen el flujo descomprimido **directamente** a port `#9B` sin buffer intermedio, lo cual merece su propio PRP. Mientras tanto, el handler de `CMDDATA` consume sus bytes con `fseek` para no descuadrar el offset del fichero; los chunks `CX[L]` se cargan sin crash pero el rectángulo no se renderiza. El INFO se ignora limpiamente con el `else` final actual (`chunkSize=10` + `auxData=0` ⇒ skip seguro).

---

## Problema

El formato `.IMx` documentado en [`bin/imgwizard.php:13-110`](../bin/imgwizard.php#L13-L110) define dos versiones:

- **Versión 1**: chunks `REDIRECT`, `PALETTE`, `RAW`, `RLE`, `PLETTER`, `RESET`, `CLS`, `SKIP`, `PAUSE` (todos generables; todos consumidos por el engine).
- **Versión 2**: chunks adicionales `INFO=128`, `V9938Cmd=20`, `V9938CmdData=21`. **Sólo aparecen en el comentario**: ni `imgwizard.php` los emite, ni `gfxPictureShow()` los consume.

El comentario en [`bin/imgwizard.php:72-110`](../bin/imgwizard.php#L72-L110) describe la cabecera de cada uno; las estructuras C correspondientes (`IMG_INFO`, `IMG_V9938_CMD`, `IMG_V9938_CMDDATA`) ya están **declaradas** en [`include/daad_platform_msx2.h:247-271`](../include/daad_platform_msx2.h#L247-L271), pero `gfxPictureShow()` las marca como `TODO`.

Faltan tres comportamientos:

1. **`Chunk INFO` como primer chunk de toda imagen generada**. Hoy la herramienta produce ficheros que arrancan directamente con `PALETTE` (modos paletizados) o `CLS` (no transparente). Sin INFO, el engine no puede saber a priori cuántos chunks hay ni el tamaño original lógico de la imagen. Decisiones futuras (preasignar buffers, validar coherencia de modo, optimizar carga) requieren INFO siempre presente.
2. **Comando nuevo `CX[L]`** que produce un fichero compuesto por: `INFO + V9938Cmd + V9938CmdData…` (con la paleta entre `INFO` y `V9938Cmd` para `CX`, o al final para `CXL`). A diferencia del comando `S` (rectángulo en `RLE`), `CX[L]` empaqueta los datos como **comandos V9938 listos para enviar al VDP** vía HMMC/LMMC, lo cual permite al engine pintar el rectángulo en cualquier posición sin recalcular `posVRAM` ni asumir un layout de 256 px/línea.
3. **`width`/`height` del chunk INFO** debe reflejar el tamaño del **recuadro** (no el tamaño de pantalla) cuando se generó con `CX[L]` o `S`; y el tamaño de la franja completa (ancho del modo × `lines`) cuando se generó con `C[L]`.

---

## Análisis técnico

### Formato actualizado del Chunk INFO (versión 1 — 10 bytes de extra-header)

El comentario en [`bin/imgwizard.php:74-87`](../bin/imgwizard.php#L74-L87) ya describe la versión 1. El struct C en [`include/daad_platform_msx2.h:247-254`](../include/daad_platform_msx2.h#L247-L254) corresponde a una **versión 0** (7 bytes: solo `infoVersion` + `chunkCount` + `width` + `height`) y necesita actualizarse para que el engine futuro lea el formato emitido.

**Layout final (v1, 10 bytes de extra header, dataSize=0)**:

| Offset | Tamaño | Campo            | Valores                                                                       |
|-------:|-------:|------------------|-------------------------------------------------------------------------------|
| `0x00` |   1    | `type`           | `128`                                                                         |
| `0x01` |   2    | `extraHeaderSize`| `10`                                                                          |
| `0x03` |   2    | `dataSize`       | `0`                                                                           |
| `0x05` |   1    | `infoVersion`    | `1`                                                                           |
| `0x06` |   2    | `chunkCount`     | nº total de chunks **incluido** el INFO                                       |
| `0x08` |   2    | `originalWidth`  | px (rectángulo si `CX[L]`/`S`; ancho de pantalla si `C[L]`)                   |
| `0x0a` |   2    | `originalHeight` | px (alto de rectángulo o `lines`)                                             |
| `0x0c` |   1    | `pixelType`      | `0=unspec, 1=BP2 (4col), 2=BP4 (16col), 4=BD8 (256col), 6=BYJK, 7=BYJKP`       |
| `0x0d` |   1    | `paletteType`    | `0=unspec, 1=GRB332, 2=GRB333`                                                |
| `0x0e` |   1    | `chipsetType`    | `0=unspec, 1=TMS9918, 2=V9938, 3=V9958, 4=V9990`                              |

Mapeo desde el modo de pantalla (`scr` = `'5'..'8','A','C'`):

| `scr` | `pixelType`  | `paletteType` | `chipsetType` | Notas                              |
|-------|--------------|---------------|---------------|------------------------------------|
| `'5'` | `2` (BP4)    | `2` (GRB333)  | `2` (V9938)   | SC5: 16 col paleta                 |
| `'6'` | `1` (BP2)    | `2` (GRB333)  | `2` (V9938)   | SC6: 4 col paleta                  |
| `'7'` | `2` (BP4)    | `2` (GRB333)  | `2` (V9938)   | SC7: 16 col paleta                 |
| `'8'` | `4` (BD8)    | `1` (GRB332)  | `2` (V9938)   | SC8: 256 col fijos                 |
| `'A'` | `7` (BYJKP)  | `1` (GRB333)  | `3` (V9958)   | SC10: YJK + 16 col paleta (mixto)  |
| `'C'` | `6` (BYJK)   | `0` (unspec)  | `2` (V9958)   | SC12: YJK puro, sin paleta         |

### Layout del Chunk V9938Cmd (`type=20`)

Re-documentación precisa (corrige el comentario actual, que confunde "Data length" con tamaño de **una** entrada vs. tamaño total):

| Offset      | Tamaño   | Campo              | Valores                                      |
|------------:|---------:|--------------------|----------------------------------------------|
| `0x00`      |    1     | `type`             | `20`                                         |
| `0x01`      |    2     | `extraHeaderSize`  | `1`                                          |
| `0x03`      |    2     | `dataSize`         | `n * 15` (siempre múltiplo de 15)            |
| `0x05`      |    1     | `cmdCount`         | `n` (1..136)                                 |
| `0x06+i*15` |   15     | `cmd[i]`           | un comando V9938 empaquetado (ver abajo)     |

**15 bytes por comando** (15 bytes de bloque de registros R32-R46):

| Offset (rel.) | Tamaño | Campo        | Descripción                                                                |
|--------------:|-------:|--------------|----------------------------------------------------------------------------|
| `0x00`        |   2    | `SX`         | R32-R33: source X (no usado en HMMC/LMMC; `0`)                             |
| `0x02`        |   2    | `SY`         | R34-R35: source Y (no usado en HMMC/LMMC; `0`)                             |
| `0x04`        |   2    | `DX`         | R36-R37: destination X (siempre en *bytes* para HMMC)                      |
| `0x06`        |   2    | `DY`         | R38-R39: destination Y (línea VRAM 0..511)                                 |
| `0x08`        |   2    | `NX`         | R40-R41: número de *bytes* en X                                            |
| `0x0a`        |   2    | `NY`         | R42-R43: número de líneas en Y                                             |
| `0x0c`        |   1    | `CLR`        | R44: primer byte de datos para HMMC (resto llega por port `#9B`)           |
| `0x0d`        |   1    | `ARG`        | R45: bits de dirección (`DIY/DIX/EQ/MAJ` etc.)                             |
| `0x0e`        |   1    | `CMD`        | R46: opcode (`CX[L]` siempre `0xF0=HMMC`; otros opcodes reservados para uso futuro) |

> El stream de `CmdData` se concibe como **una sola tira lógica de bytes** que el engine va consumiendo por orden conforme dispara comandos V9938 sucesivos. La sincronización entre cmd_i y su porción del stream es **posicional**: el cmd_i consume exactamente `NX * NY` bytes (HMMC, byte-mode), empezando donde terminó el cmd_(i-1).

> El layout de 15 bytes coincide exactamente con el bloque de registros R32-R46 listo para volcar al VDP vía `OUTI` (15× a port `#9B`) tras seleccionar R17 con auto-incremento — patrón clásico `DoCopy` (ver `msxdocs_programming__asm_execute_a_vdp_command`). No se añade metadata extra: `compressorID` y `uncompressedSize` viven en cada `V9938CmdData`, lo cual basta para descomprimir y enviar al VDP en streaming.

**Nota sobre opcode**: `CX[L]` usa **siempre HMMC** (opcode `0xF0`) — alta velocidad CPU→VRAM en byte-mode, sin operación lógica. `DX` y `NX` van **siempre en *bytes*** (no en dots ni en píxeles). La conversión de dots a bytes se hace en imgwizard según `pixelsByte[$scr]`:

| `scr` | Pixels/byte | Bytes/línea | `DX` y `NX` (CX) |
|-------|-------------|-------------|------------------|
| `'5'` | 2           | 128         | `x/2`, `w/2`     |
| `'6'` | 4           | 128         | `x/4`, `w/4`     |
| `'7'` | 2           | 256         | `x/2`, `w/2`     |
| `'8'` | 1           | 256         | `x`, `w`         |
| `'A'` | 1           | 256         | `x`, `w`         |
| `'C'` | 1           | 256         | `x`, `w`         |

> Para SC6 esto exige `x % 4 == 0` y `w % 4 == 0`; para SC5/7 exige `x % 2 == 0` y `w % 2 == 0`. Ya lo validamos con un `die()` para SC10/12 (mod 4); generalizar a todos los modos.

### Layout del Chunk V9938CmdData (`type=21`)

| Offset | Tamaño    | Campo              | Valores                                       |
|-------:|----------:|--------------------|-----------------------------------------------|
| `0x00` |    1      | `type`             | `21`                                          |
| `0x01` |    2      | `extraHeaderSize`  | `3`                                           |
| `0x03` |    2      | `dataSize`         | `1..2040` (longitud del payload comprimido)   |
| `0x05` |    1      | `compressorID`     | `0=RAW, 1=RLE, 2=PLETTER`                     |
| `0x06` |    2      | `uncompressedSize` | tamaño descomprimido en bytes (≤ `2*1024*8`)  |
| `0x08` | `dataSize`| `data[]`           | payload                                       |

> Constante `IMG_CMDDATA_SIZE = CHUNK_SIZE - 3 = 2040` (para que un chunk completo siga cabiendo en el buffer `IMG_MAXREAD` actual). Documentar como `define('CHUNK_CMDDATA_MAX', 2040)`.

### Estructura final de un fichero `CX[L]`

```
CX  (paleta entre INFO y comandos):

  [INFO]                                 (15 bytes total: 5 hdr + 10 extra)
  [PALETTE]            ← solo modos paletizados (SC5/6/7); ausente en SC8/A/C
  [V9938Cmd]                             (5 hdr + 1 extra + 15 data, cmdCount=1)
  [V9938CmdData #1]                      (5 hdr + 3 extra + payload)
  [V9938CmdData #2]                      ...
  [V9938CmdData #N]                      ← N franjas hasta cubrir NX·NY bytes

CXL (paleta al final):

  [INFO]
  [V9938Cmd]                             (cmdCount=1)
  [V9938CmdData #1]
  …
  [V9938CmdData #N]
  [PALETTE]            ← solo modos paletizados; movido al final
```

### Estructura para `C[L]` y `S` (con INFO añadido al inicio)

```
C[/CL]  (full-width image):
  [INFO]                                 ← NUEVO, primer chunk siempre
  [PALETTE]      ← solo paletizados, no en CL (CL → al final)
  [CLS]          ← solo si no es transparente
  [RAW|RLE|PLETTER #1]                   ← chunks de datos (RLE/RAW/PLETTER según -comp)
  [RAW|RLE|PLETTER #2]
  …
  [PALETTE]      ← solo CL (modos paletizados)

S       (rectangle):
  [INFO]                                 ← NUEVO
  [PALETTE]      ← si paletizado y no -lastPalette
  [CLS]          ← si no transparente
  [RLE #1] …                             ← compresión forzada a RLE
```

`R` (redirect), `5A`/`CA` (conversiones de formato): estos comandos producen formatos especiales (R no es una imagen completa; 5A/CA generan SCx, no IMx). **No se les añade INFO**. La función `addInfoChunk()` se llama sólo desde los comandos productores de `.IM*`.

### Generación de comandos V9938 desde un rectángulo

Dado un rectángulo `(x, y, w, h)` y un modo `scr`:

1. Computar `bytes/línea` y unidades de `DX`/`NX` (tabla anterior).
2. **Emitir un único `V9938Cmd` con `cmdCount=1`** que describe la operación CPU→VRAM completa para todo el rectángulo: `DX=x` (en *bytes*), `DY=y`, `NX=w` (en *bytes*), `NY=h`, `CMD=0xF0` (**HMMC** — High-speed Move CPU→VRAM, byte-mode, sin operación lógica), `ARG=0`. El VDP, una vez disparado el comando, irá consumiendo los bytes que el host le envíe a port `#9B` hasta completar `NX·NY` *bytes* sin necesidad de re-disparar nada.
3. **Particionar el payload** del rectángulo (no la geometría) en chunks `V9938CmdData` cuyo payload comprimido quepa en ≤ `CHUNK_CMDDATA_MAX = 2040` bytes. Como la compresión es variable, el algoritmo replica el bucle adaptativo de `compressChunks()`: pedir `sizeIn` agresivo, comprimir, ajustar. Cada `V9938CmdData` lleva su propio `compressorID` + `uncompressedSize`.
4. El engine, al cargar el `CX[L]`, leerá el único comando del `V9938Cmd`, lo disparará al VDP, y a continuación irá descomprimiendo cada `V9938CmdData` en orden y stream-eando los bytes resultantes a port `#9B`. La concatenación de todos los `uncompressedSize` debe ser exactamente `NX·NY` *bytes*.

**Nota de diseño**: la decisión de un único comando V9938 simplifica la ejecución en el engine (un único `DoCopy` + un único bucle de `OUTI` con cambio de chunk de datos cuando el actual se agota), evita reposicionamientos VRAM intermedios y mantiene el `V9938Cmd` mínimo (15 bytes de payload). El motivo de existir un *array* de hasta 136 entradas en el formato `V9938Cmd` queda como **infraestructura compartida** para futuros chunks/comandos que sí necesiten múltiples operaciones (p. ej. animaciones de rectángulos no contiguos), pero **`CX[L]` siempre emite `cmdCount=1`**.

**Por qué HMMC y no LMMC**: HMMC es estrictamente más rápido que LMMC (transfiere un byte completo por unidad, no un dot, lo que es 2× para SC5/7 y 4× para SC6) y al no haber operación lógica (`IMP`), LMMC no aporta nada que HMMC no haga. La única diferencia es la unidad de `DX`/`NX`: HMMC trabaja siempre en *bytes*, así que en SC5/6/7 la imagen "se ve" como si fueran columnas de bytes consecutivos en VRAM — exactamente lo que el SCx ya almacena. Por tanto, en cualquier modo (`'5'..'8','A','C'`) podemos volcar los bytes del SCx tal cual al VDP usando HMMC.

---

## Compatibilidad con el engine actual

El handler actual de [`gfxPictureShow()`](../src/daad_platform_msx2.c#L864) recorre los chunks con un `do-while` y un `if/else if` por tipo. **Los tipos no reconocidos caen al `else` final** que hace `fread(chunk->data, chunk->chunkSize)` y luego `posVRAM += chunk->auxData`. Para `type=128` (INFO):

- `chunkSize` = `10` (extraHeaderSize del INFO, empaquetado por PHP como `pack("vv", 10, 0)`).
- `auxData` = `0` (dataSize del INFO).

El `else` final, al ejecutar `fread(chunk->data, 10)`, **consume exactamente los 10 bytes del extra header** del INFO (que es lo que hay que saltar) y luego `posVRAM += 0` (no avanza la posición VRAM). Es decir, **el `else` final ya gestiona correctamente el INFO como skip**, sin tocar VRAM. Lo único "feo" es que los 10 bytes acaban en `chunk->data[]` como si fueran datos de imagen, pero ahí se quedan inertes hasta el siguiente chunk.

**Conclusión**: NO hace falta añadir un `case IMG_CHUNK_INFO` en el engine para que las imágenes con INFO se carguen sin error. El cambio en imgwizard.php es **autosuficiente** y compatible con el engine actual.

Lo que sí hace falta tocar en el engine son los TODOs de `IMG_CHUNK_CMD` y `IMG_CHUNK_CMDDATA` (ver siguiente sección): hoy leen sólo el extra header pero no avanzan los `dataSize`/`n*15` bytes restantes, lo que **descuadra el offset del fichero** y los chunks posteriores se interpretan como basura. Esa es la mínima coordinación obligatoria con el engine.

---

## Cambios en `bin/imgwizard.php`

### 1. Constantes nuevas

Cerca de [`bin/imgwizard.php:113-124`](../bin/imgwizard.php#L113-L124):

```php
    define('CHUNK_HEAD',        5);
    define('CHUNK_SIZE',        2043);
    define('CHUNK_CMDDATA_MAX', 2040);     // CHUNK_SIZE - 3 (extra header de CmdData)

    define('CHUNK_REDIRECT',    0);
    define('CHUNK_PALETTE',     1);
    define('CHUNK_RAW',         2);
    define('CHUNK_RLE',         3);
    define('CHUNK_PLETTER',     4);
    define('CHUNK_RESET',      16);
    define('CHUNK_CLS',        17);
    define('CHUNK_SKIP',       18);
    define('CHUNK_PAUSE',      19);
    define('CHUNK_V9938CMD',   20);        // NUEVO
    define('CHUNK_V9938DATA',  21);        // NUEVO
    define('CHUNK_INFO',      128);        // NUEVO

    define('INFO_VERSION',      1);
    define('CMP_RAW',           0);
    define('CMP_RLE',           1);
    define('CMP_PLETTER',       2);

    define('VDP_HMMC',       0xF0);     // CX[L] siempre usa HMMC (sin op lógica)
```

### 2. Función `buildInfoChunk($scr, $chunkCount, $width, $height): string`

```php
function buildInfoChunk($scr, $chunkCount, $width, $height)
{
    static $pixel   = ['5'=>2,'6'=>1,'7'=>2,'8'=>4,'A'=>7,'C'=>6];
    static $palette = ['5'=>2,'6'=>2,'7'=>2,'8'=>1,'A'=>1,'C'=>0];
    static $chipset = ['5'=>2,'6'=>2,'7'=>2,'8'=>2,'A'=>3,'C'=>2];

    $scr = strtoupper($scr);
    return chr(CHUNK_INFO)                                  // type
         . pack("vv", 10, 0)                                // extraHeaderSize=10, dataSize=0
         . chr(INFO_VERSION)                                // infoVersion=1
         . pack("vvv", $chunkCount, $width, $height)        // chunkCount, originalWidth, originalHeight
         . chr($pixel[$scr]   ?? 0)
         . chr($palette[$scr] ?? 0)
         . chr($chipset[$scr] ?? 0);
}
```

### 3. Inserción del INFO al inicio de cada generador

**`compressChunks()`** ([imgwizard.php:754-860](../bin/imgwizard.php#L754-L860)) — emitir el INFO inmediatamente después de la magic + screen-mode.

Patrón actual:
```php
$out = $magic;
…
$out .= $scr;
…
// Add palette …
```

Patrón nuevo:
```php
$out = $magic;
…
$out .= $scr;

// Reservar 15 bytes para el INFO (rellenado al final con chunkCount real)
$infoPos = strlen($out);
$out .= str_repeat("\0", 15);     // placeholder
$id = 1;                          // INFO ya cuenta como chunk 1
…
// resto igual
…
// Al terminar, sobreescribir el placeholder con el INFO real
$infoChunk = buildInfoChunk($scr, $id - 1 + 1 /* +INFO */, $width[$scr], $lines);
$out = substr_replace($out, $infoChunk, $infoPos, 15);
```

> El "originalWidth" para `C[L]` es el ancho **completo** del modo (`$width[$scr]`, ya tabulado). El "originalHeight" es `$lines`.

**`compressRectangle()`** ([imgwizard.php:666-751](../bin/imgwizard.php#L666-L751)) — análogo, con `originalWidth=$w` y `originalHeight=$h`.

**Comando `j` (join)** — re-cuenta los chunks del fichero combinado y escribe un INFO nuevo al inicio. El ancho/alto se toma del INFO del primer fichero (si existe); si los ficheros origen no tienen INFO, se omite (compatibilidad hacia atrás temporal). **Decisión**: en este PRP, el comando `j` simplemente *valida* que todas las entradas tengan el mismo `screenMode` y emite un INFO nuevo al frente; si algún input no tiene INFO, se computa con el `chunkCount` total y `width`/`height` se quedan en `0` (unspec) — el listado posterior con `imgwizard l` lo hará evidente.

**Comandos `R`, `5A`, `CA`**: NO se modifican.

### 4. Comando nuevo `CX[L]`

Añadir tras el bloque de `cl/c` ([imgwizard.php:211-256](../bin/imgwizard.php#L211-L256)):

```php
    // Create rectangle image as V9938 commands (CXL = palette at last chunk)
    if ($cmd == 'cxl') {
        $lastPalette = true;
        $cmd = 'cx';
    }
    if ($cmd == 'cx' && $argc>=7) {
        $fileIn = $argv[2];
        echo "### Loading $fileIn\n";
        if (!is_numeric($argv[3]) || !is_numeric($argv[4]) ||
            !is_numeric($argv[5]) || !is_numeric($argv[6])) {
            echo "ERROR: x, y, w, and h must be numeric and >= 0...\n";
            exit;
        }
        $x = intval($argv[3]); $y = intval($argv[4]);
        $w = intval($argv[5]); $h = intval($argv[6]);
        $compress = "RLE";   // default
        if ($argc>7) {
            $compress = strtoupper($argv[7]);
        }
        $comp = NULL;
        foreach ($compressors as $c) {
            if ($c[COMP_NAME]==$compress) { $comp = $c; break; }
        }
        if ($comp===NULL) {
            echo "ERROR: Unknown compression method ($compress). Use RAW/RLE/PLETTER...\n";
            exit;
        }
        echo "### Compressor: $compress (CX command — no transparency allowed)\n";
        compressV9938Rectangle($fileIn, $x, $y, $w, $h, $comp);
        exit;
    }
```

### 5. Función `compressV9938Rectangle()`

Implementación nueva (estructura simétrica a `compressRectangle()` pero con dos streams: `cmdStream` y `dataStream`).

```php
function compressV9938Rectangle($file, $x, $y, $w, $h, $comp)
{
    global $magic, $lastPalette;

    $scr   = checkScreemMode($file);
    $sup   = strtoupper($scr);
    echo "### Mode SCREEN ".hexdec($scr)."\n";

    // Tablas modo→unidades
    $pixelsByte  = ['5'=>2,'6'=>4,'7'=>2,'8'=>1,'A'=>1,'C'=>1];
    $bytesLine   = ['5'=>128,'6'=>128,'7'=>256,'8'=>256,'A'=>256,'C'=>256];

    // HMMC trabaja en bytes — validar que x y w son múltiplos de pixelsByte
    if ($x % $pixelsByte[$sup] || $w % $pixelsByte[$sup]) {
        die("\nERROR: SCREEN ".hexdec($sup)." needs x and w multiple of ".$pixelsByte[$sup]."...\n\n");
    }

    // Lectura SCx (descartar header de 7 bytes)
    $in = @file_get_contents($file);
    if ($in===FALSE) { die("File not found...\n"); }
    $in = substr($in, 7);

    // Conversión de DX/NX a bytes (HMMC siempre en bytes)
    $dxUnit = intval($x / $pixelsByte[$sup]);
    $nxUnit = intval($w / $pixelsByte[$sup]);

    // 1) Extraer el rectángulo completo en un buffer contiguo (uncompressed)
    $wb       = intval(round($w / $pixelsByte[$sup]));   // bytes por línea
    $fullRect = sliceRect($in, $x, $y, $w, $h, $pixelsByte[$sup], $bytesLine[$sup]);
    $totalUncomp = strlen($fullRect);                    // = $wb * $h

    // 2) Particionar el payload en franjas que quepan en CHUNK_CMDDATA_MAX bytes comprimidos
    $tmp = tempnam(sys_get_temp_dir(), 'imgwiz');
    $dataChunks = [];                                    // array de [compID, uncompSize, payload]
    $pos = 0;
    while ($pos < $totalUncomp) {
        // Bucle adaptativo: pedir lo más grande posible y reducir si no cabe
        $sizeIn    = min($totalUncomp - $pos, CHUNK_CMDDATA_MAX);
        $sizeDelta = intval(CHUNK_CMDDATA_MAX / 2);
        $end = false;
        do {
            $sizeOut = compress($tmp, $fullRect, $pos, $sizeIn, $comp, -1);
            if ($pos + $sizeIn >= $totalUncomp && $sizeOut <= CHUNK_CMDDATA_MAX) {
                $end = true;
            } else if ($sizeOut < CHUNK_CMDDATA_MAX - 1) {
                if ($sizeDelta > 0 && $pos + $sizeIn < $totalUncomp) {
                    $sizeIn = min($sizeIn + $sizeDelta, $totalUncomp - $pos);
                } else {
                    $end = true;
                }
            } else if ($sizeOut > CHUNK_CMDDATA_MAX) {
                $sizeIn -= $sizeDelta;
                $sizeDelta = intval($sizeDelta * 0.95);
            } else {
                $end = true;
            }
        } while (!$end);

        $payload = file_get_contents($tmp.'.'.$comp[COMP_EXT]);
        $compID  = $comp[COMP_ID]==CHUNK_RAW   ? CMP_RAW
                : ($comp[COMP_ID]==CHUNK_RLE  ? CMP_RLE : CMP_PLETTER);
        $dataChunks[] = [$compID, $sizeIn, $payload];
        echo "    franja pos=$pos uncomp=$sizeIn comp=".strlen($payload)." bytes\n";
        $pos += $sizeIn;
    }
    @unlink($tmp); @unlink($tmp.'.'.$comp[COMP_EXT]);

    // 3) Emitir un único V9938Cmd con cmdCount=1 que cubre todo el rectángulo.
    //    El primer byte de payload del primer CmdData va en CLR (R44) — lo
    //    consumirá el VDP al arrancar la operación; el resto fluye por port #9B.
    $firstByte  = ord($dataChunks[0][2][0]);
    $cmdEntry   = pack("vvvvvvCCC",
        0, 0,                       // SX, SY (R32-R35, no usados en HMMC/LMMC)
        $dxUnit, $y,                // DX, DY (R36-R39)
        $nxUnit, $h,                // NX, NY (R40-R43)
        $firstByte,                 // CLR    (R44)
        0,                          // ARG    (R45)
        VDP_HMMC                    // CMD    (R46) — siempre HMMC, sin op lógica
    );
    $cmdChunksBin = chr(CHUNK_V9938CMD).pack("vv", 1, 15).chr(1).$cmdEntry;

    // Empaquetar V9938CmdData chunks
    $dataChunksBin = "";
    foreach ($dataChunks as [$compID, $uncomp, $payload]) {
        $dataChunksBin .= chr(CHUNK_V9938DATA).pack("vv", 3, strlen($payload))
                        . chr($compID).pack("v", $uncomp).$payload;
    }

    // Paleta (si paletizado)
    $palBin = "";
    if (hexdec($scr) < 8) {
        list($in2, $paper, $ink) = checkPalettedColors($in, $scr);
        $palBin = addPalette($file, $in2, $scr, NULL);
    }

    // Ensamblar fichero final
    $out  = $magic.$scr;
    $infoPos = strlen($out);
    $out .= str_repeat("\0", 15);                  // INFO placeholder

    if ($palBin && !$lastPalette) $out .= $palBin;
    $out .= $cmdChunksBin;
    $out .= $dataChunksBin;
    if ($palBin && $lastPalette)  $out .= $palBin;

    // Contar chunks reales
    $chunkCount = countChunks(substr($out, $infoPos+15)) + 1;
    $infoBin = buildInfoChunk($scr, $chunkCount, $w, $h);
    $out = substr_replace($out, $infoBin, $infoPos, 15);

    // Escribir
    $fileOut = substr(basename($file), 0, -3)."IM".$scr;
    echo "### Writing $fileOut\n";
    file_put_contents($fileOut, $out);
    echo "### Done\n\n";
}
```

> `sliceRect()` es una refactorización de la lógica interna de `rle_encode_selection()` que extrae la franja en *bytes contiguos* del SCx (sin compresión). `countChunks()` recorre la salida y cuenta type-bytes hasta `EOF` (sólo se usa al final para rellenar el INFO; coste despreciable).

> `compress()` ya existe ([imgwizard.php:863](../bin/imgwizard.php#L863)) y soporta `raw`/`rle`/`pletter`. La firma actual recibe `($tmp, $in, $pos, $sizeIn, $comp, $transparent)` y escribe a fichero temporal — se reutiliza tal cual con `transparent=-1`.

### 6. Actualizar `showImageContent()` para listar los nuevos chunks

[`bin/imgwizard.php:425-479`](../bin/imgwizard.php#L425-L479) — añadir tres `case`:

```php
        case CHUNK_INFO:
            // sin: extraHeaderSize (10), sout: dataSize (0)
            fseek leyendo: ya tenemos los 10 bytes en $in[$pos+5 .. +14]
            $extra = substr($in, $pos+5, $sin);
            $ver   = ord($extra[0]);
            $cnt   = unpack("v", substr($extra, 1, 2))[1];
            $w     = unpack("v", substr($extra, 3, 2))[1];
            $h     = unpack("v", substr($extra, 5, 2))[1];
            $px    = ord($extra[7]); $pl = ord($extra[8]); $cs = ord($extra[9]);
            echo "    CHUNK $id: INFO v$ver chunks=$cnt w=$w h=$h px=$px pal=$pl chip=$cs\n";
            $size += $sin;
            break;

        case CHUNK_V9938CMD:
            $n = ord(substr($in, $pos+5, 1));
            echo "    CHUNK $id: V9938Cmd ($n commands, ".$sout." bytes)\n";
            // Listar cada uno de los $n comandos (15 bytes cada uno) con su tipo
            static $cmdNames = [
                0x00 => 'STOP',  0x40 => 'POINT', 0x50 => 'PSET',
                0x60 => 'SRCH',  0x70 => 'LINE',  0x80 => 'LMMV',
                0x90 => 'LMMM',  0xA0 => 'LMCM',  0xB0 => 'LMMC',
                0xC0 => 'HMMV',  0xD0 => 'HMMM',  0xE0 => 'YMMM', 0xF0 => 'HMMC',
            ];
            for ($k = 0; $k < $n; $k++) {
                $off = $pos + 6 + $k*15;
                $rs  = unpack("v6/C3", substr($in, $off, 15));
                // rs: 1..6 = SX,SY,DX,DY,NX,NY ; 7..9 = CLR,ARG,CMD
                $opName = $cmdNames[$rs[9] & 0xF0] ?? sprintf('?(0x%02X)', $rs[9]);
                $logic  = $rs[9] & 0x0F;
                printf("        cmd %2d: %s%s SX=%d SY=%d DX=%d DY=%d NX=%d NY=%d CLR=0x%02X ARG=0x%02X\n",
                    $k+1, $opName, $logic ? sprintf('|0x%X', $logic) : '',
                    $rs[1], $rs[2], $rs[3], $rs[4], $rs[5], $rs[6], $rs[7], $rs[8]);
            }
            $size += $sin + $sout;
            break;

        case CHUNK_V9938DATA:
            $extra = substr($in, $pos+5, 3);
            $compID = ord($extra[0]);
            $uncomp = unpack("v", substr($extra, 1, 2))[1];
            $compName = ['raw','rle','pletter'][$compID] ?? '?';
            echo "    CHUNK $id: V9938CmdData [$compName] $uncomp bytes ($sout comp)\n";
            $size += $sin + $sout;
            break;
```

> El parser actual usa `array_values(unpack('cType/vSizeOut/vSizeIn', …))`. Para los chunks de versión 2, `SizeOut` es `extraHeaderSize` y `SizeIn` es `dataSize`, por lo que el `size += 5 + $sin + $sout` cubre el extra header + data.

> Reescribir el cálculo de `$size` actual de los chunks v1 (que sólo suma `$sin`) para que sea uniforme: en v1 `extraHeaderSize` está empaquetado en la posición de `SizeOut` y `dataSize` (uncomp) en la de `SizeIn`. **Cuidado**: cambiar el parser de v1 podría romper imágenes existentes en `dsk/`. Solución: mantener el case-por-case con `$size += $sin` en v1 y `$size += $sin + $sout` sólo en v2 (los nuevos cases), porque para v1 la convención original era `$sin = chunkSize == bytes a saltar` y `$sout` es metadato (no contribuye al avance de cabeza).

### 7. Actualizar el bloque de sintaxis (`showSyntax()`)

[`bin/imgwizard.php:341-381`](../bin/imgwizard.php#L341-L381) — añadir entrada `CX[L]` y bumpear la versión:

```php
echo "\nIMGWIZARD v1.4.0 for MSX2DAAD\n"
   . …
   . "CX) Create a rectangle image as V9938 commands (CXL = palette last):\n"
   . "    $appname cx[l] <fileIn.SC?> <x> <y> <w> <h> [compressor]\n"
   . …
```

---

## Cambios en el engine (mínimo de coordinación)

### Actualizar `IMG_INFO` en el header al formato v1

[`include/daad_platform_msx2.h:247-254`](../include/daad_platform_msx2.h#L247-L254):

```c
typedef struct {
    IMG_CHUNK_HEADER header;        // { 128, 10, 0 } extraHeaderSize=10, dataSize=0
    uint8_t  infoVersion;           // { 1 }
    uint16_t chunkCount;            // total chunks (incluye INFO)
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t  pixelType;             // 0/1/2/4
    uint8_t  paletteType;           // 0/1/2
    uint8_t  chipsetType;           // 0/1/2/3/4
} IMG_INFO;                          // sizeof = 5 (hdr) + 10 (extra) = 15
```

> Quitar el `**NOT IMPLEMENTED**` del `#define IMG_CHUNK_INFO` (línea 227) — pasa a "**SKIPPED via else fallback**" hasta que un PRP futuro consuma sus campos.

### `case IMG_CHUNK_INFO`: no es necesario añadirlo

El `else` final del bucle ya consume los 10 bytes del extra header del INFO (`fread(chunk->data, chunk->chunkSize=10)`) y avanza `posVRAM += 0` (auxData=0), por lo que **el INFO se ignora limpiamente** sin handler explícito. Mantener el código actual y pasar al siguiente paso.

### Implementar `IMG_CHUNK_CMD` (y dejar `IMG_CHUNK_CMDDATA` como skip)

[`fastVCopy(char *bitbltData)`](../src/libs/vdp_fastvcopy.s) ya existe y recibe un puntero a un bloque de **exactamente 15 bytes** con la estructura R32-R46, volcándolos al VDP vía 15× `OUTI` a port `#9B` tras seleccionar `R17 + autoincrement`. Es la firma que coincide 1:1 con el layout que emite imgwizard, así que el handler de CMD se reduce a un bucle `while (cmdCount--) { fastVCopy(p); p += 15; }`.

Para `CMDDATA`, la ejecución queda **diferida** (ver §Alcance). El handler en este PRP sólo consume los bytes del payload con `fseek(chunk->chunkSize, SEEK_CUR)` para no descuadrar el offset del fichero. La lógica funcional queda como bloque `/* … */` comentado en el código, con la justificación de por qué descomprimir a RAM no es aceptable.

#### Lógica final en `gfxPictureShow()`

Reemplazar las dos ramas TODO en [`src/daad_platform_msx2.c:920-930`](../src/daad_platform_msx2.c#L920-L930):

```c
//=============================================
// V9938 commands chunk: dispara cada comando empaquetado
if (chunk->type==IMG_CHUNK_CMD) {
    size = fread(chunk->data, 1);                       // cmdCount
    if (!(size & 0xff00)) {
        uint8_t cmdCount = chunk->data[0];
        // Leer los n×15 bytes del bloque de comandos a chunk->data
        fread(chunk->data, (uint16_t)cmdCount * 15);
        // Disparar cada comando con fastVCopy (15 bytes c/u)
        char *p = chunk->data;
        while (cmdCount--) {
            fastVCopy(p);
            p += 15;
        }
        // Para HMMC el VDP queda esperando datos en R44 — los próximos
        // V9938CmdData los proporcionarán.
    }
} else
//=============================================
// V9938 command data chunk: por ahora SOLO consume bytes para no descuadrar
// el offset del fichero. La ejecución (descompresión + envío a port #9B)
// queda pendiente — ver PRP futuro de "stream-decompress to VDP" abajo.
if (chunk->type==IMG_CHUNK_CMDDATA) {
    size = fread(chunk->data, chunk->auxData);          // 3 bytes extra header
    if (!(size & 0xff00)) {
        fseek(chunk->chunkSize, SEEK_CUR);              // saltar payload comprimido
        /*
         * TODO (PRP futuro): ejecutar HMMC streaming sin buffer descomprimido en RAM.
         *
         * Opción A — descomprimir a RAM y volcar a #9B (rechazada por coste de RAM):
         *
         *     uint8_t  compID = chunk->data[0];
         *     uint16_t uncomp = *(uint16_t*)&chunk->data[1];
         *     fread(chunk->data, chunk->chunkSize);
         *     char *out = (char*)(heap_top + sizeof(IMG_CHUNK));
         *     switch (compID) {
         *         case 0: memcpy(out, chunk->data, uncomp);     break;   // RAW
         *         case 1: unRLE(chunk->data, out);              break;   // RLE
         *         case 2: pletter2ram(chunk->data, out);        break;   // PLETTER
         *     }
         *     // Volcar a port #9B (el VDP en HMMC redirige a R44 con TR interno)
         *     __asm
         *         ld   hl,#_out
         *         ld   l,(hl)
         *         inc  hl
         *         ld   h,(hl)
         *         ld   bc,#_uncomp
         *         ld   c,(bc)
         *         inc  bc
         *         ld   b,(bc)
         *     1$: ld   a,(hl)
         *         out  (0x9B),a
         *         inc  hl
         *         dec  bc
         *         ld   a,b
         *         or   c
         *         jr   nz,1$
         *     __endasm;
         *
         * Coste: hasta `uncompressedSize` bytes de RAM extra para el buffer (con
         * Pletter típico ~2× el comprimido — para 2040 bytes comprimidos podrían
         * ser ~4-8KB descomprimidos). Inaceptable en MSX-DOS1.
         *
         * Opción B (preferida) — descompresores con sumidero al puerto #9B:
         *   nuevas variantes `unRLE_port9B(src)`, `pletter2port9B(src)`,
         *   `copy2port9B(src, len)` que en lugar de `LD (DE),A` hacen
         *   `OUT (0x9B),A`. Cero RAM extra; mismo bucle de descompresión
         *   redirigido al bus VDP.  Esto se diseña en un PRP separado.
         */
    }
} else

---

## Casos límite

### Imágenes existentes en `dsk/`

Las 27 imágenes `*.IM8` actuales **no contienen INFO**. Tras el upgrade del engine (`fseek` en CMD/CMDDATA), siguen funcionando idénticamente: el bucle no encuentra `type=128` y procede como antes. No es necesario regenerarlas.

Tras este PRP, el comando `imgwizard l` mostrará "(no INFO chunk)" para imágenes legacy. Documentar como aviso (no bloqueante).

### Tamaño de buffer en el engine

`IMG_MAXREAD = CHUNK_SIZE + 5 = 2048`. En `CX[L]` el `V9938Cmd` siempre tiene `cmdCount=1` (15 bytes de payload + 6 de cabecera = 21 bytes) — muy holgado. Los `V9938CmdData` llegan hasta 2040 bytes de payload (`+ 8` de cabecera = 2048) y **caben justo** en `IMG_MAXREAD`. Verificar que `chunk->data[IMG_CHUNK_SIZE]` (2043 bytes) no se desborda al hacer `fread(chunk->data, chunk->chunkSize)` con `chunkSize=2040`. 2040 ≤ 2043 ⇒ OK. El límite teórico de `136·15 = 2040` para múltiples comandos en un solo `V9938Cmd` queda como margen para futuros chunks que sí emitan listas de comandos.

### Modos YJK (SC10 = `'A'`, SC12 = `'C'`)

`CX[L]` *no* asume nada sobre el formato YJK — sólo empaqueta los bytes tal cual los entrega el SCx (4 bytes = 4 píxeles). El comando V9938 generado es `HMMC` (byte-mode), que es lo correcto para YJK.

### Compresor `PLETTER` requiere binario externo

El comando `cx[l]` con `[compressor]=PLETTER` invoca `pletter` (`pletter.exe` en Windows) igual que `c[l]`. Si el binario no está en `$PATH`, falla con el mismo error existente — no se introduce dependencia nueva.

### `lastPalette` y join

`compressV9938Rectangle` respeta `$lastPalette` igual que las funciones existentes. El comando `j` que une un fichero `CXL` con uno `C` produce un fichero con dos paletas (al final y al medio): es responsabilidad del autor evitarlo. **No se valida** en este PRP — patrón ya presente en `j` actual.

### Versiones múltiples de INFO

Si un futuro PRP introduce `infoVersion=2` con un `extraHeaderSize` distinto, el `else` final del engine **sigue siendo correcto** porque hace `fread(…, chunk->chunkSize)` — y `chunkSize` es justamente el `extraHeaderSize` de la cabecera común. El skip se autoadapta al tamaño que indique cada versión sin recompilar el engine.

---

## Resumen de archivos modificados

| Archivo                                     | Cambio                                                                       |
|---------------------------------------------|------------------------------------------------------------------------------|
| [`bin/imgwizard.php`](../bin/imgwizard.php) | Constantes nuevas + `buildInfoChunk()` + INFO en `c/cl/s` + comando `cx[l]` + `compressV9938Rectangle()` + 3 cases nuevos en `showImageContent()` + sintaxis + bump versión 1.4.0 |
| [`include/daad_platform_msx2.h`](../include/daad_platform_msx2.h) | Extender `IMG_INFO` a v1 (10 bytes extra: + `pixelType`/`paletteType`/`chipsetType`); quitar `NOT IMPLEMENTED` de `IMG_CHUNK_INFO` |
| [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c) | Implementar `case IMG_CHUNK_CMD` (dispara cada cmd con `fastVCopy` existente); `case IMG_CHUNK_CMDDATA` queda **skip-only** con el código de ejecución comentado para PRP futuro; INFO se ignora vía el `else` final |

**Total estimado**: ~280 líneas en PHP + ~15 líneas en C (handler activo de CMD + skip de CMDDATA + ~40 líneas comentadas como referencia para el PRP futuro); +3 campos en el struct `IMG_INFO`. Sin nuevos ficheros en `src/libs/`.

---

## Tests propuestos

No hay infraestructura de unit tests para imgwizard.php. Se propone validación manual con un script `bin/imgwizard_test.sh` (opcional, fuera de alcance):

1. **`imgwizard c`** sobre una imagen SC8 conocida → verificar que `imgwizard l` muestra `CHUNK 1: INFO v1 chunks=N w=256 h=Y px=4 pal=1 chip=2`.
2. **`imgwizard cl`** → mismo INFO al inicio + `PALETTE` al final (si paletizado).
3. **`imgwizard cx`** sobre SC5 con rect (16,16,64,32) → INFO con `w=64 h=32`, paleta, **un único** V9938Cmd con 1 entrada de 15 bytes (`DX=8, DY=16, NX=32, NY=32, CMD=0xF0`) — DX/NX en bytes (16/2 y 64/2), 1+ V9938CmdData RLE cubriendo el payload completo (1024 bytes uncomp).
4. **`imgwizard cxl`** sobre SC8 con rect (0,0,256,212) → INFO `w=256 h=212`, V9938Cmd con `cmdCount=1` (`DX=0, DY=0, NX=256, NY=212, CMD=0xF0`), N CmdData (varios según compresión, sumando 256·212 = 54272 bytes uncomp), **sin paleta** (SC8 fija).
5. **`imgwizard cx`** sobre SC8 con `PLETTER` → comprobar `compressorID=2` en el CmdData.
6. **`imgwizard l`** sobre cualquier `.IMx` legacy → primer chunk debe ser `PALETTE` o `CLS` (no INFO); herramienta no falla.
7. **`make test`** con un `dsk/` que contenga una imagen recién generada con INFO → engine arranca y la pinta correctamente (los chunks v1 funcionan; INFO se salta).
8. **`make test`** con una imagen `CX[L]` → engine arranca, no crashea, los chunks se consumen correctamente (offset alineado), pero **el rectángulo no se renderiza**: el `V9938Cmd` se dispara con `fastVCopy()` y deja al VDP esperando datos en R44, pero `V9938CmdData` se salta con `fseek` sin volcar nada. El rectángulo de destino queda con los datos que tuviera previamente. La ejecución funcional vendrá en un PRP posterior con descompresores con sumidero a port `#9B`.

---

## Verificación manual

### Listado tras `cx`

```bash
$ bin/imgwizard.php cx test.sc5 16 16 64 32
### Loading test.sc5
### Mode SCREEN 5
### Compressor: RLE (CX command — no transparency allowed)
    franja pos=0 uncomp=1024 comp=312 bytes
### Writing test.IM5
### Done

$ bin/imgwizard.php l test.IM5
### Reading file test.IM5
### Mode SCREEN 5
    CHUNK 1: INFO v1 chunks=4 w=64 h=32 px=2 pal=2 chip=2
    CHUNK 2: RGB333 Palette 32 bytes
    CHUNK 3: V9938Cmd (1 commands, 15 bytes)
        cmd  1: HMMC SX=0 SY=0 DX=8 DY=16 NX=32 NY=32 CLR=0x?? ARG=0x00
    CHUNK 4: V9938CmdData [rle] 1024 bytes (312 comp)
### End of file
```

### Listado tras `cxl`

```bash
$ bin/imgwizard.php cxl test.sc5 0 0 128 64 PLETTER
…
$ bin/imgwizard.php l test.IM5
    CHUNK 1: INFO v1 chunks=N w=128 h=64 px=2 pal=2 chip=2
    CHUNK 2: V9938Cmd (1 commands, 15 bytes)
        cmd  1: HMMC SX=0 SY=0 DX=0 DY=0 NX=64 NY=64 CLR=0x?? ARG=0x00
    CHUNK 3..N+2: V9938CmdData [pletter] …
    CHUNK N+3: RGB333 Palette 32 bytes
```

---

## Notas de implementación

- La elección "INFO siempre como primer chunk" facilita parsers futuros que necesiten metadatos sin recorrer el fichero entero. Es una invariante explícita: **todo fichero `.IMx` generado con imgwizard ≥ 1.4.0 tiene INFO en posición 0**.
- El `chunkCount` se rellena al final con un placeholder de 15 bytes — patrón estándar en formatos contenedores cuando el conteo final no es conocido al inicio.
- El layout de 15 bytes coincide exactamente con el bloque de registros R32-R46 listo para volcar al VDP vía 15× `OUTI` a port `#9B` tras seleccionar R17 con auto-incremento (patrón clásico `DoCopy`). Cualquier metadata adicional (offsets, dataLength) viene implícita en el orden de los `V9938CmdData` y en sus campos propios.
- El opcode `HMMC` (`0xF0`) no soporta operación lógica (es un block-move por bytes). Si en el futuro se necesita componer con lógica (OR/XOR/AND/TIMP/etc.) sobre lo ya pintado, habría que cambiar a `LMMC` (`0xB0`) y emitir `DX`/`NX` en *dots* — el formato ya soporta cambiar el byte CMD sin tocar el ABI.
- `ARG=0` significa "DI dirección normal, sin expansion RAM" — suficiente para todos los casos de uso del engine actual.
- El bumpeo de versión a `1.4.0` es coherente con el cambio de formato de INFO (v0 → v1) y la introducción del comando `CX[L]`.
- La ejecución de V9938Cmd se apoya íntegramente en `fastVCopy()` ya existente en `vdp.lib` (Grauw `DoCopy` — ver `msxdocs_programming__asm_execute_a_vdp_command`).
- **Diferido a PRP futuro**: la ejecución del `V9938CmdData`. El enfoque "descomprimir a RAM y volcar a `#9B`" es trivial pero gasta hasta varios KB de RAM por chunk (Pletter típico ~50% ratio sobre 2040 bytes comprimidos ≈ 4KB; en peor caso podría llegar a 16KB según el `uncompressedSize`). El diseño correcto es añadir variantes "stream-to-port" de los descompresores: `unRLE_port9B(src)`, `pletter2port9B(src)`, `copy2port9B(src, len)` — el bucle interno de cada descompresor cambia su sumidero de `LD (DE),A` a `OUT (0x9B),A`. Con esto, el handler de CMDDATA queda en O(1) de RAM. Se diseñará en su propio PRP con los benchmarks de timing VDP correspondientes.
