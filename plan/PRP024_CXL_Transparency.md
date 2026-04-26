# PRP024 — `CX[L]` con transparencia: máscara + imagen vía LMMC

## Contexto

PRP022 introdujo el comando `CX[L]` en `imgwizard.php` para emitir rectángulos como secuencia INFO + V9938Cmd (HMMC) + V9938CmdData (RAW/RLE/Pletter5). PRP023 implementó los descompresores en runtime para que el stream comprimido se reconstituya y vuelque por puerto #9B sin descomprimir antes en RAM.

Ahora se extiende `CX[L]` para soportar **transparencia con un color elegible**, sin alterar el flujo existente cuando no se usa transparencia.

## Objetivo

Añadir el flag `--transparent-color=N` a los comandos `cx` y `cxl` de `imgwizard.php`. Cuando se especifica:

1. `imgwizard` preprocesa la imagen para generar **dos buffers**:
   - **Máscara**: 1 byte por píxel, bits del píxel a `1` donde transparente, `0` donde visible.
   - **Imagen sin transparente**: 1 byte por píxel con el color sustituido por `0` en píxeles transparentes.
2. Ambos buffers se emiten al `.IMx` como secuencia de chunks: V9938Cmd(LMMC|AND) + V9938CmdData(máscara) + V9938Cmd(LMMC|OR) + V9938CmdData(imagen).
3. El runtime los reproduce con el handler ya existente — **cero cambios en el motor**.

## Por qué este diseño

Diseño elegido tras estudio comparativo entre 7 estrategias (ver el informe técnico previo en chat / `assets/v9938_transparency_options.md` si se decide archivar):

- **LMMC streaming directo** (Diseño 1) en lugar de HMMC packed → hidden VRAM + LMMM (Diseño 2):
  - Solo 2 comandos VDP por imagen (LMMC AND + LMMC OR), no 4.
  - Sin gestión de hidden zones ni asignación dinámica de scratch.
  - Runtime intacto: el handler `IMG_CHUNK_CMD`/`IMG_CHUNK_CMDDATA` ya soporta secuencias arbitrarias.
  - El stream "raw" en SC5/7 (×2) y SC6 (×4) parece más voluminoso que packed, pero la **redundancia de bits altos** (6 bits constantes a 0 en SC6, 4 en SC5/7) hace que la compresión Pletter5/RLE compense con creces.
  - La máscara, por ser bicolor a nivel de píxel, comprime extremadamente bien en cualquier modo.

## Restricciones

- **No se permite transparencia en SC10/SC12 (YJK)**. La codificación YJK comparte componentes J/K cada 4 píxeles → `LMMC AND/OR` por bits no produce colores válidos en bordes. `imgwizard` debe rechazar `--transparent-color` con error claro si la imagen es `.SCA` o `.SCC`.
- **Se permite transparencia en SC5, SC6, SC7, SC8** (todos los modos bitmap "puros" G4/G5/G7).
- El usuario elige qué código de color `N` marca como transparente. `imgwizard` lo traduce a la representación interna (`mask=full_bits, image=0` para píxeles transparentes) sin necesidad de rotar paleta ni sacrificar el código 0: la desambiguación entre "color 0 visible" y "transparente" la lleva el buffer de máscara, no el de imagen (ver sección de diseño técnico §1.3).

## Diseño técnico

### 1. Cambios en `bin/imgwizard.php`

#### 1.1 CLI

Extender los comandos `cx` y `cxl`:

```
cx[l] <fileIn.SC?> <x> <y> <w> <h> [compressor] [--transparent-color=N]
```

- `N` es el código de color en el espacio de paleta del modo origen (0..255 SC8, 0..15 SC5/7, 0..3 SC6).
- El argumento es **opcional**. Si no se especifica → comportamiento actual de PRP022 (HMMC raw).

#### 1.2 Lógica de preprocesado (cuando `--transparent-color=N`)

```python
# Pseudocódigo
for each pixel P in input rectangle:
    if P == N:
        mask_byte  = pixel_full_bits_for_mode (0xFF SC8 / 0x0F SC5/7 / 0x03 SC6)
        image_byte = 0
    else:
        mask_byte  = 0
        image_byte = P  # color real, posiblemente con bits altos a 0 para SC5/6/7

mask_buffer  ← collect (1 byte por píxel, w*h bytes total)
image_buffer ← collect (1 byte por píxel, w*h bytes total)
```

**Nota crítica:** ambos buffers son **1 byte por píxel** independientemente del modo. Esto es requisito de LMMC (la spec garantiza "one byte is required for the information of one pixel in all screen modes"). Los bits altos no usados (6 en SC6, 4 en SC5/7) se ponen a 0 — son ignorados por el VDP en R#44 pero ayudan a la compresión Pletter (patrones predecibles).

#### 1.3 Color 0 puede coexistir libremente con la transparencia (sin rotar paleta)

La pareja `mask=0x00, image=0` significa "color 0 visible" y la pareja `mask=full_bits, image=0` significa "transparente". El byte `image=0` es ambiguo por sí solo, pero el byte de máscara correspondiente lo desambigua:

| Caso | mask byte | image byte | Pass 1 (AND) | Pass 2 (OR) | Resultado final |
|------|-----------|------------|---------------|--------------|------------------|
| Visible color C≠0 | 0x00 | C | dest ← 0 | dest ← C | **C** ✓ |
| Visible color 0 | 0x00 | 0 | dest ← 0 | dest ← 0 | **0** ✓ |
| Transparente | full_bits (0xFF/0x0F/0x03) | 0 | dest preservado | dest preservado | **original** ✓ |

Por tanto:
- **No hace falta rotar la paleta** en SC5/SC6/SC7 si la imagen usa el índice 0.
- **No hace falta sustituir 0x00 por 0x04** en SC8 si la imagen contiene negro puro.
- El usuario es libre de elegir cualquier `N` como transparente, incluso `N=0` (en cuyo caso la imagen-original con color 0 = transparente se preserva tal cual; los demás colores quedan visibles).

`imgwizard` solo necesita generar los dos buffers según la regla del pseudocódigo §1.2; el comportamiento de las dos pasadas LMMC AND+OR garantiza la corrección sin más trabajo.

#### 1.4 Emisión de chunks al `.IMx`

Secuencia generada cuando `--transparent-color` está activo, manteniendo INFO al inicio:

```
INFO chunk (chunkCount actualizado para reflejar los chunks adicionales)

V9938Cmd #1 (LMMC + AND)
  cmdCount=1
  data[15]: SX=0,SY=0,DX=x,DY=y,NX=w,NY=h,CLR=0,ARG=0,CMD=0xB1 (LMMC opcode B0 + LOG_AND 01)

V9938CmdData #1 (máscara, comprimida con `compressor`)
  ... posiblemente N chunks Pletter5 si excede 11264 bytes ...

V9938Cmd #2 (LMMC + OR)
  cmdCount=1
  data[15]: SX=0,SY=0,DX=x,DY=y,NX=w,NY=h,CLR=0,ARG=0,CMD=0xB2 (LMMC opcode B0 + LOG_OR 02)

V9938CmdData #2 (imagen sin transparente, comprimida con `compressor`)
  ... posiblemente M chunks Pletter5 ...
```

**Opcodes finales**:
| Operación lógica | Bits LO3..LO0 | LMMC opcode (R#46) |
|------------------|----------------|---------------------|
| AND              | 0001           | `0xB1`              |
| OR               | 0010           | `0xB2`              |

#### 1.5 Validación / errores

- Si modo destino es SC10/SC12 (`.SCA` o `.SCC`) y se especifica `--transparent-color`, error fatal:
  ```
  Error: --transparent-color is not supported in SC10/SC12 (YJK).
         Use a different screen mode or pre-composite the image.
  ```
- Si `N` está fuera del rango válido del modo (>15 en SC5/7, >3 en SC6, >255 SC8), error.
- Si la imagen no contiene ningún píxel con valor `N`, warning ("imagen no tiene píxeles transparentes; se emite máscara vacía") y continúa generando los 2 chunks (no es fatal).

### 2. Cambios en runtime: ninguno

El handler de `IMG_CHUNK_CMD` ya soporta `cmdCount=1` y se llama múltiples veces si hay varios V9938Cmd. El handler de `IMG_CHUNK_CMDDATA` despacha al descompresor correcto (RAW/RLE/Pletter5) y esos descompresores **funcionan exactamente igual con LMMC que con HMMC** porque solo escriben a R#44 vía puerto #9B. El opcode del comando (HMMC=0xF0 vs LMMC+AND=0xB1 vs LMMC+OR=0xB2) ya está dentro del bloque de 15 bytes que `fastVCopy` despacha.

**Validación pendiente**: confirmar empíricamente que el comportamiento es correcto (ver Test Plan).

## Compatibilidad

- Sin `--transparent-color`: el comando `cx[l]` se comporta idénticamente a PRP022 (HMMC, sin cambios). Imágenes ya generadas siguen funcionando.
- Con `--transparent-color`: imágenes nuevas. Para repintar una zona transparente con la imagen, basta con dispatchar la secuencia AND+OR sobre el rectángulo destino actual (no se requiere "limpiar" antes — la pasada AND ya borra los píxeles visibles a 0 preservando el resto).

## Consideraciones de rendimiento

Para una imagen WxH píxeles:

| Métrica                             | CX[L] sin trans (HMMC) | CX[L] con trans (LMMC×2)  |
|-------------------------------------|------------------------|---------------------------|
| Comandos VDP                        | 1                      | 2                         |
| Bytes streamed (raw, SC8)           | W*H                    | 2 * W*H                   |
| Bytes streamed (raw, SC5/7)         | W*H/2                  | 2 * W*H                   |
| Bytes streamed (raw, SC6)           | W*H/4                  | 2 * W*H                   |
| Tiempo VDP (fullscreen 256×212 SC8) | ~7 frames @ 50Hz       | ~20 frames @ 50Hz         |
| Tiempo VDP (sprite 64×64 SC8)       | ~0.5 frames            | ~1.5 frames               |
| Compresión esperada del mask        | n/a                    | excelente (1-3% del raw)  |

Para los casos típicos de aventura DAAD (sprites, retratos, ítems), la penalización de tiempo es imperceptible. Para imágenes fullscreen con transparencia (raras en aventuras textuales), el coste es aceptable: 0.4 segundos a 50 Hz.

## Test Plan

### Tests automáticos

No hay tests automatizados en `imgwizard.php`. Se añadirán tests manuales documentados en `assets/test_transparency.md`:

1. **Test SC8 minimal**: imagen 8×8 SC8 con un único color transparente.
   - Verificar: la imagen pintada conserva el fondo previamente dibujado en los píxeles transparentes.
2. **Test SC5 nibble alignment**: imagen 16×8 SC5 con transparencia en píxeles individuales (no en pares).
   - Verificar: en bytes con un píxel visible y otro transparente, ambos resultan correctos (preservación parcial).
3. **Test SC6 quad alignment**: imagen 32×8 SC6 con transparencia en patrones 1100, 1010, 0011 dentro de un mismo byte.
   - Verificar: los 4 píxeles de cada byte se procesan correctamente independientes.
4. **Test color 0 visible coexistiendo con transparencia**: imagen SC7 (o SC8) que usa el código 0 como color sólido en una zona y `--transparent-color=15` (o cualquier N≠0).
   - Verificar: los píxeles con código 0 se renderizan como color 0 (no como transparentes), y los píxeles con código N=15 preservan el destino. Sin rotación de paleta.
5. **Test imagen vacía / completamente transparente**: imagen de un solo color = `--transparent-color`.
   - Verificar: máscara = todo 0xFF/0x0F/0x03, imagen = todo 0; render no toca nada del destino.
6. **Test imagen sin transparencia presente**: imagen sin píxeles del color transparente.
   - Verificar: warning, máscara = todo 0, imagen = idéntica al original. Render funciona como un blit normal.

### Test rejection SC10/SC12

7. **Test rejection**: `cx LOAD.SCA 0 0 256 212 PLETTER --transparent-color=0`.
   - Verificar: error fatal con mensaje claro, no se genera output.

### Test runtime en hardware/openMSX

8. **Test integración**: imagen 64×64 SC8 con transparencia, repintada sobre un fondo no negro.
   - Verificar visualmente que el fondo se preserva en los píxeles transparentes.
9. **Test compresión PLETTER en SC6**: imagen 128×64 SC6 con transparencia y compressor=PLETTER.
   - Verificar: chunk size razonable, render correcto.
10. **Test multi-chunk PLETTER**: imagen 256×100 SC8 (25600 bytes raw → 2-3 chunks Pletter).
    - Verificar: la secuencia de N V9938CmdData consecutivos para la máscara y luego M para la imagen funciona sin estados residuales.

## Riesgos / Unknowns

1. **Estado de R#17 entre el primer LMMC y el segundo**:
   - Tras streamar la máscara, R#17 = 44 con AII=1.
   - El siguiente `fastVCopy` para LMMC+OR resetea R#17 = 32 (auto-inc ON) y vuelve a 47 tras los 15 OUTI.
   - Luego `setIndirectRegisterPointer` lo pone otra vez a 44+0x80.
   - **Sin riesgo**: el flujo es idéntico al primer LMMC.

2. **Persistencia del comando LMMC durante pletter2vram**:
   - PRP023 ya validó que pletter2vram no toca R#32-R#46 ni #9B. La misma garantía aplica a LMMC.
   - **Sin riesgo nuevo**.

3. **Compresión Pletter5 en máscara muy pequeña**:
   - Si la imagen es 8×8 con transparencia esparcida, la máscara raw es 64 bytes; comprimida puede salir mayor por overhead Pletter.
   - **Mitigación**: imgwizard ya elige el compresor óptimo (RAW/RLE/PLETTER) si se le da el flag adaptativo.

4. **No se ha implementado todavía un mecanismo "patch del opcode" en runtime**:
   - El opcode LMMC+AND/OR está dentro del bloque de 15 bytes del V9938Cmd, así que no requiere lógica especial en runtime — `fastVCopy` lo despacha igual que un HMMC.
   - **Sin riesgo**: si el opcode es LMMC, el VDP ejecuta LMMC; si es HMMC, ejecuta HMMC.

## Cambios resumidos

### Archivos modificados

- `bin/imgwizard.php`: nueva función `processTransparency()` (genera buffers mask+image a 1 byte/píxel desempaquetando del formato packed nativo del modo) y extensión de `cmdCx()`/`cmdCxl()` con parsing de `--transparent-color`.

### Archivos NO modificados

- Runtime (`src/libs/`, `src/daad_platform_msx2.c`, `include/daad_platform_msx2.h`). El motor ya soporta esta funcionalidad implícitamente, sin flags ni cambios. La transparencia es **transparente** al intérprete: solo ve una secuencia más larga de chunks V9938Cmd+V9938CmdData y los procesa con el mismo dispatcher que para HMMC.

### Tests / docs

- `assets/test_transparency.md` (nuevo): plan de pruebas manual.
- `wiki/MSX2DAAD-Wiki:-DAAD-imgwizard.md` (si existe): documentar `--transparent-color`.

## Acción inmediata sugerida

1. Implementar `processTransparency($pixels, $width, $height, $screenMode, $transparentColor)` en `imgwizard.php` que devuelve `[$maskBuffer, $imageBuffer]` (ambos arrays de bytes a 1 byte/píxel, longitud `width*height`).
2. Wire-up en `cmdCx()` / `cmdCxl()`: si flag `--transparent-color=N` presente, llamar `processTransparency()` y emitir secuencia V9938Cmd(LMMC|AND) + V9938CmdData(mask) + V9938Cmd(LMMC|OR) + V9938CmdData(image); si no, comportamiento actual de PRP022.
3. Validación previa: rechazo SC10/SC12, rango de `N` válido para el modo (0..255 SC8, 0..15 SC5/7, 0..3 SC6).
4. Tests manuales en openMSX con imágenes simples (8×8, 64×64) para verificar correctness en todos los modos permitidos.

## Referencias

- V9938 Programmer's Guide §Logical operations (TIMP/TAND/TOR…)
- MSX2 Technical Handbook §6.5.5 LMMC, §6.3 Logical Operations
- Grauw — "VDP commands & speed measurements" (LMMC vs HMMC ratio)
- PRP022 — `CX[L]` base sin transparencia
- PRP023 — descompresores HMMC streaming (válidos también para LMMC)

---

## Anexo A — Idiosincrasia R#44 al dispatch (off-by-one descubierto)

Durante la implementación se descubrió un comportamiento del V9938 que afecta a **todos los paths** (HMMC/LMMC, con o sin transparencia) y obliga a un ajuste de `-1 byte` en el stream emitido por `imgwizard`. Este anexo lo documenta para que no se vuelva a perder de vista.

### Síntoma

Imágenes generadas con `cx[l]` (con o sin `--transparent-color`) renderizaban en openMSX/hardware con un **desplazamiento horizontal de 1 píxel** y el primer byte duplicado. Visualmente: cada fila aparecía corrida 1 columna a la derecha, con el último píxel de la fila previa apareciendo al inicio de la siguiente.

### Causa raíz

El V9938 al dispatchar HMMC/LMMC (escritura del opcode en R#46) **consume inmediatamente el valor actual de R#44 (CLR) como el primer píxel** del rectángulo. Cita textual:

> *"The value written to R#44 (CLR) is the source/target color for fills and serves as the initial byte for HMMC/LMMC."* — Grauw, [VDP commands](https://map.grauw.nl/articles/vdp_commands.php)

Y de la spec V9938:

> *"Specify the byte to be written first to the position (DX, DY) using R#44. Issue the HMMC command via R#46."* — V9938 Programmer's Guide §3.3.4

`fastVCopy` (basado en DoCopy de Grauw) escribe los 15 bytes R#32..R#46 en una ráfaga única vía OUTI con R#17 auto-inc desde 32. El byte 13 de la ráfaga aterriza en R#44, y el byte 15 (opcode CMD) en R#46 lanza el comando. **R#44 ya tenía un valor cargado al momento del trigger**, y ese valor se procesa como pixel 0.

Si el motor luego stream `N` bytes vía #9B, el VDP procesa:
- pixel 0 = R#44 inicial (cargado en la ráfaga)
- pixel 1 = streamed[0]
- pixel 2 = streamed[1]
- …
- pixel N-1 = streamed[N-2]
- streamed[N-1] **se ignora** (HMMC/LMMC ya completaron NX·NY píxeles)

Resultado neto: la imagen aparece desplazada +1 píxel y el byte 0 duplicado, exactamente el síntoma observado.

Adicionalmente, para RLE/Pletter5 el "byte que aterriza en R#44" durante la ráfaga era el primer byte del **payload comprimido** (un byte de control RLE/Pletter, no un píxel) — produciendo un pixel 0 con valor basura.

### Solución (aplicada en `imgwizard.buildV9938CmdSequence()`)

En lugar de generar chunks que sumen `N` bytes uncompressed, `imgwizard` ahora:

1. **Extrae el primer byte UNCOMPRESSED** (`data[0]` = primer píxel real del rectángulo o de los buffers mask/image).
2. **Lo coloca en el campo CLR (R#44) del bloque de 15 bytes del V9938Cmd**. Ese byte es el píxel real que el VDP procesará al dispatch.
3. **Comprime y emite chunks que cubren `data[1..N-1]`** (longitud `N-1` uncompressed), no `data[0..N-1]`.

Verificación de cuentas:
- HMMC/LMMC procesa NX·NY píxeles totales.
- Píxel 0 ← R#44 = `data[0]` ✓
- Píxeles 1..N-1 ← stream de `N-1` bytes ✓
- Suma = N píxeles = NX·NY ✓

### Por qué la solución va en `imgwizard` y no en el runtime

Alternativas consideradas:

| Solución | Pros | Contras |
|----------|------|---------|
| **`imgwizard` trim 1 byte** (elegida) | Cambio one-off offline; runtime intacto; aplica a HMMC y LMMC por igual; chunks ya correctos en disco | Cambio en el formato/cuenta de `uncompressedSize` (afecta a ratios mostrados) |
| Runtime salta primer byte del primer V9938CmdData tras V9938Cmd | No requiere regenerar imágenes existentes | Requiere estado entre chunks ("¿es el primero tras el CMD?"); afecta `copyCmdData`/`unRLE_Data`/`pletter2Data`; complica DCE |
| `fastVCopy` no cargar R#44 en la ráfaga | "Resuelve" el síntoma | No funciona en realidad: R#44 mantiene su valor previo (residual de un comando anterior) y HMMC consumiría ese valor; tampoco evita que algún byte aleatorio acabe en pixel 0 |

La opción `imgwizard` es la única que produce salida correcta sin estado en runtime ni dependencias entre chunks.

### Confirmación empírica

`dsk/000.IM8` regenerada tras el fix renderiza correctamente en openMSX (`msx2_eu`) — los 4 retratos del splash se ven nítidos, sin shift, sin duplicación de byte 0. Antes del fix la misma imagen mostraba el desplazamiento de 1 píxel.

### Implicación para PRP022 retroactivamente

El bug existía ya en PRP022 (CX[L] sin transparencia). Cualquier imagen `.IMx` generada con `imgwizard` antes del commit que aplicó este fix tiene el desplazamiento. **Regenerar todas las imágenes existentes** generadas con `cx[l]` después de aplicar el fix.

### Referencia cruzada con PRP023

PRP023 documentó la persistencia del comando HMMC durante `pletter2vram` y la necesidad de programar R#17 = 44|0x80 antes del streaming. Esos puntos son ortogonales al off-by-one de R#44 — los tres requisitos coexisten:

1. R#17 = 44|0x80 antes del streaming → cada byte vía #9B aterriza en R#44 (PRP023, fix en `setIndirectRegisterPointer`).
2. pletter2vram no toca R#32-R#46 ni #9B → comando HMMC/LMMC sobrevive a la decompresión Pletter (PRP023).
3. R#44 inicial = `data[0]` real, stream de `N-1` bytes → píxel 0 correcto, sin shift (PRP024 — este anexo).
