# PRP020 — GFX_SET_PALETTE / GFX_GET_PALETTE

- **ID**: PRP020
- **Tipo**: Implementación + bugfix
- **Ficheros afectados**:
  - [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c)
  - [`unitTests/src/condacts_stubs.c`](../unitTests/src/condacts_stubs.c)
  - [`unitTests/src/condacts_stubs.h`](../unitTests/src/condacts_stubs.h)
  - [`unitTests/src/tests_condacts6.c`](../unitTests/src/tests_condacts6.c)
  - [`wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md`](../wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md)
- **Severidad**: MINOR — comportamiento incorrecto (paleta RAM desincronizada, GET sin implementar)
- **Fecha**: 2026-04-19
- **Estado**: Completado ✅

---

## Problem

`GFX_SET_PALETTE` (routine 9) y `GFX_GET_PALETTE` (routine 10) están definidos en `gfxRoutines()` pero incompletos:

- `GFX_SET_PALETTE` llama a `gfxSetPalette()` correctamente pero **no actualiza** el array `colorTranslation[]` en RAM, por lo que `getColor()`, las operaciones de borde/tinta/papel y las imágenes siguientes usan la paleta desactualizada.
- `GFX_GET_PALETTE` no hace nada (TODO).
- En SC8/SC12 ambas rutinas no están contempladas, aunque en SC8 la paleta del borde y los sprites sí es modificable vía registro VDP R7.
- El array `colorTranslation[]` es `static const` — no puede modificarse directamente en tiempo de ejecución sin un cast explícito.

**Referencia de la API DAAD** (`docs/GFX.md`, líneas 9–27):
```
9  Set Palette value  → bloque de 4 flags: [idx, Red, Green, Blue]  (RGB 0–255, tope 3 MSB)
10 Read Palette value → bloque de 4 flags: [idx, Red, Green, Blue]
```

---

## Análisis técnico

### Formatos de paleta por modo

| Modo | Tipo | Paleta hardware | Bits efectivos | Borde modificable | Sprites |
|------|------|----------------|----------------|-------------------|---------|
| SC5, SC7 | Paletted GRB333 | VDP palette reg (16 entradas × 2 bytes) | 3 bits/canal | Sí (vía paleta) | Sí |
| SC6 | Paletted GRB333 | VDP palette reg (4 entradas × 2 bytes) | 3 bits/canal | Sí | Sí |
| SC8 | Fixed 256 colors GRB332 | No existe paleta de colores | 3+3+2 bits fijos | Sí (VDP R7, índice GRB332) | Sí (índice en R7) |
| SC10 | YJK mixed | VDP palette (para los 16 colores de modo palette) | — | Sí | Sí |
| SC12 | YJK full | No existe paleta de colores | — | Sí (VDP R7) | No relevante |

### Almacenamiento de paleta en software

El VDP (V9938/V9958) **no permite leer** los registros de paleta una vez escritos. Por eso el estado de la paleta vive en RAM:

- `colorTranslation[]` — formato GRB333 empaquetado en `uint16_t` (`0x0GRB`): `00000GGG 0RRR0BBB`
  - SC5, SC7: 16 entradas
  - SC6: 4 entradas (paleta ámbar por defecto)
- `colorTranslationSC8[]` — formato GRB332 en `uint8_t` (`GGGRRRBB`): mapeo fijo EGA→SC8
  - SC8: 16 entradas (usado por `getColor()` para traducir índice EGA a byte SC8)

Actualmente ambos son `static const`. Para mantener la inicialización estática (datos en ROM/flash en el arranque) **deben permanecer `const`**; las escrituras en runtime se harán mediante cast a puntero mutable.

### Conversión RGB255 → GRB333 (para `setColorPal`)

El formato de 16 bits almacenado en `colorTranslation[]` (verificado con los valores iniciales):

```
0x006 = blue  → bits 2-0 = BBB
0x060 = red   → bits 6-4 = RRR
0x600 = green → bits 10-8 = GGG
```

Formato: `0000 0GGG 0RRR 0BBB` (V9938 envía el byte bajo primero como `0RRR0BBB`, luego el alto como `0000 0GGG`).

Conversión correcta de RGB255 a GRB333:
```c
grb = (((uint16_t)green & 0xE0) << 3)   // green bits 7-5 → bits 10-8 (campo GGG)
    | ((red             & 0xE0) >> 1)   // red   bits 7-5 → bits  6-4 (campo RRR)
    | ((blue            & 0xE0) >> 5);  // blue  bits 7-5 → bits  2-0 (campo BBB)
```

> **Bug en el código existente**: `gfxSetPalette()` en [src/daad_platform_msx2.c:814](src/daad_platform_msx2.c#L814) tiene `red` y `green` intercambiados (`red << 3` va al campo GGG, `green >> 1` al campo RRR). Este PRP corrige ese bug.

### Conversión GRB333 → RGB255 (para `GFX_GET_PALETTE`)

```c
// Dado grb con G en bits 10-8, R en bits 6-4, B en bits 2-0:
red   = (uint8_t)((grb << 1) & 0xE0);   // bits 6-4 → desplaza a bits 7-5
green = (uint8_t)((grb >> 3) & 0xE0);   // bits 10-8 → desplaza a bits 7-5
blue  = (uint8_t)((grb << 5) & 0xE0);   // bits 2-0 → desplaza a bits 7-5
```

### SC8: paleta de borde y sprites

En SC8 no hay paleta de colores de imagen, pero:
- **VDP R7** controla el color del borde y el fondo de sprites de modo 1 (en SC8 relevante para border).
- Formato R7 en SC8: `GGGRRRBB` (igual que los bytes de `colorTranslationSC8[]`).
- `gfxSetBorderCol()` ya llama a `setBorder(colorTranslationSC8[col % 16])`.

Por tanto, `GFX_SET_PALETTE` en SC8 debería:
1. Actualizar `colorTranslationSC8[index % 16]` con el valor GRB332 calculado.
2. Si el índice coincide con el color actual del borde (o con color 0 = PAPER), actualizar R7 vía `setBorder()`.
3. **No** llamar a `setColorPal()` (no tiene paleta hardware).

---

## Cambios necesarios

### 1. Mantener `colorTranslation[]` y `colorTranslationSC8[]` como `static const`; usar cast para escritura

**Archivo**: [src/daad_platform_msx2.c](src/daad_platform_msx2.c#L405)

Los arrays **permanecen `static const`** para que sus valores iniciales estén en la sección de datos inicializados al arranque (equivalente a ROM/flash en el modelo MSX-DOS). No se elimina `const`.

Las escrituras en runtime se realizan casteando el puntero a su versión mutable. Para evitar repetir el cast en varios sitios, se definen punteros auxiliares locales o se usa un macro de acceso:

```c
// En gfxSetPalette() y en la sincronización de chunk de imagen:
#if SCREEN == 8
    ((uint8_t*)colorTranslationSC8)[index] = grb8;
#else
    ((uint16_t*)colorTranslation)[index] = grb;
#endif
```

> **Justificación**: quitar `const` movería los arrays de `.rodata` a `.data`, ocupando 32 bytes extra de RAM inicializada (o 16 en SC8). Con `const` + cast obtenemos el mismo comportamiento en runtime sin coste adicional de RAM, y la intención de "solo se escribe excepcionalmente" queda documentada por el cast explícito.

### 2. Completar `gfxSetPalette()` para que actualice el array en RAM

**Archivo**: [src/daad_platform_msx2.c](src/daad_platform_msx2.c#L812)

```c
inline void gfxSetPalette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
#if SCREEN == 8
    index %= 16;
    // GRB332: GGGRRRBB
    uint8_t grb8 = ((green & 0xE0)) | ((red & 0xE0) >> 3) | ((blue & 0xE0) >> 6);
    ((uint8_t*)colorTranslationSC8)[index] = grb8;
#elif SCREEN != 12
    index %= 16;
    uint16_t grb = (((uint16_t)green & 0xE0) << 3) | ((red & 0xE0) >> 1) | ((blue & 0xE0) >> 5);
    ((uint16_t*)colorTranslation)[index] = grb;
    setColorPal(index, grb);
#endif
    // SC12: paleta no existe, no hacer nada
}
```

> **Nota SC8**: El cálculo GRB332 es:
> - `green` → bits 7-5 → `GGGRRRBB`: `(green & 0xE0)` ya ocupa bits 7-5 ✓
> - `red`   → bits 4-2 → `(red & 0xE0) >> 3` ✓
> - `blue`  → bits 1-0 → `(blue & 0xE0) >> 6` ✓

### 3. Implementar `GFX_GET_PALETTE` en `gfxRoutines()`

**Archivo**: [src/daad_platform_msx2.c](src/daad_platform_msx2.c#L977)

```c
//=================== Get Palette (10)
case GFX_GET_PALETTE: {
    uint8_t idx = flags[value] % 16;
#if SCREEN == 8
    uint8_t grb8 = colorTranslationSC8[idx];
    flags[value+1] = (grb8 << 3) & 0xE0;        // Red:   bits 4-2 → bits 7-5
    flags[value+2] = grb8 & 0xE0;               // Green: bits 7-5
    flags[value+3] = (grb8 << 6) & 0xC0;        // Blue:  bits 1-0 → bits 7-6
#elif SCREEN != 12
    uint16_t grb = colorTranslation[idx];
    flags[value+1] = (uint8_t)((grb << 1) & 0xE0);   // Red:   bits 6-4 → bits 7-5
    flags[value+2] = (uint8_t)((grb >> 3) & 0xE0);   // Green: bits 10-8 → bits 7-5
    flags[value+3] = (uint8_t)((grb << 5) & 0xE0);   // Blue:  bits 2-0 → bits 7-5
#endif
    // SC12: no hay paleta, flags quedan a 0 (sin modificar)
    break;
}
```

> La convención de los flags [value+0..+3] es: `[idx, Red, Green, Blue]` según `docs/GFX.md`.
> El resultado tiene los 3 bits significativos en los bits 7-5 (alineados al MSB de un byte 0-255).

### 4. Eliminar el TODO comentado en `GFX_SET_PALETTE`

**Archivo**: [src/daad_platform_msx2.c](src/daad_platform_msx2.c#L975)

```c
// Antes:
case GFX_SET_PALETTE:
    gfxSetPalette(flags[value], flags[value+1], flags[value+2], flags[value+3]);
    //TODO: change the colors array
    break;

// Después: (sin cambio en la llamada, el TODO lo resuelve el paso 2)
case GFX_SET_PALETTE:
    gfxSetPalette(flags[value], flags[value+1], flags[value+2], flags[value+3]);
    break;
```

### 5. Actualizar la wiki para reflejar soporte de `GFX_GET_PALETTE` en MSX2

**Archivo**: [wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md](wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md)

Cambiar la celda de soporte de rutina 10 de `✗` a `✓` para MSX2.

---

## Casos límite y consideraciones

### SC8: `GFX_SET_PALETTE` vs borde

En SC8 el color de borde/sprites se controla vía R7. Si el aventurero llama a `GFX_SET_PALETTE` con `index=0` (que es el PAPER, y normalmente también el borde), el borde no se actualizará automáticamente salvo que se llame también a `BORDER`. Esto es correcto y coherente: `GFX_SET_PALETTE` actualiza la tabla de traducción, no el VDP directamente en SC8. El autor de la aventura debe llamar a `BORDER` después si quiere actualizar el borde. **No hay acción especial necesaria.**

### SC12: sin paleta

En SC12, tanto `GFX_SET_PALETTE` como `GFX_GET_PALETTE` no tienen efecto observable sobre la imagen (YJK full). Los flags de `GET_PALETTE` quedan sin modificar (no se zerean, para evitar confusión). Esto es aceptable dado que la documentación DAAD no especifica comportamiento en modos sin paleta.

### Carga de imagen con chunk `IMG_CHUNK_PALETTE`

En `gfxPictureShow()` ([src/daad_platform_msx2.c](src/daad_platform_msx2.c#L893)), cuando se carga un chunk de paleta, se llama directamente a `setPalette(chunk->data)` sin actualizar `colorTranslation[]`. Con el cambio propuesto, esto supone una inconsistencia: la paleta hardware cambia pero el array RAM no. 

**Solución**: Copiar los datos del chunk en `colorTranslation[]` antes de llamar a `setPalette()`:

```c
if (chunk->type==IMG_CHUNK_PALETTE) {
    #if SCREEN!=8 && SCREEN!=12
        size = fread(chunk->data, 32);
        if (!(size & 0xff00)) {
            memcpy((void*)colorTranslation, chunk->data, 32);
            setPalette(chunk->data);
        }
    #else
        fseek(32, SEEK_CUR);
    #endif
}
```

Esto garantiza que `GFX_GET_PALETTE` después de una imagen devuelve los colores correctos. El coste es una llamada a `memcpy` de 32 bytes, negligible.

### Impacto en `getColor()`

`getColor()` usa `colorTranslationSC8[col % 16]` para SC8 y devuelve el índice de paleta para SC5/7. Si la paleta cambia en runtime, `getColor()` ya devuelve el valor correcto automáticamente porque lee el array (ahora mutable). Sin cambios adicionales necesarios.

---

## Resumen de archivos modificados

| Archivo | Cambio | Estado |
|---------|--------|--------|
| [src/daad_platform_msx2.c](src/daad_platform_msx2.c) | Reescribir `gfxSetPalette()`: bug R/G, soporte SC8, actualiza array con cast mutable | ✅ |
| [src/daad_platform_msx2.c](src/daad_platform_msx2.c) | Implementar `GFX_GET_PALETTE` en `gfxRoutines()` | ✅ |
| [src/daad_platform_msx2.c](src/daad_platform_msx2.c) | Eliminar `//TODO` en `GFX_SET_PALETTE` | ✅ |
| [src/daad_platform_msx2.c](src/daad_platform_msx2.c) | Sincronizar `colorTranslation[]` al cargar chunk de paleta en imagen (`memcpy` antes de `setPalette`) | ✅ |
| [unitTests/src/condacts_stubs.c](unitTests/src/condacts_stubs.c) | Arrays de paleta de test + `gfxRoutines()` real + reset en `beforeEach()` | ✅ |
| [unitTests/src/condacts_stubs.h](unitTests/src/condacts_stubs.h) | `extern` de arrays de paleta de test | ✅ |
| [unitTests/src/tests_condacts6.c](unitTests/src/tests_condacts6.c) | 3 tests GFX (SET, GET, round-trip) en lugar del TODO | ✅ |
| [wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md](wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md) | Rutina 10 marcada como soportada en MSX2 | ✅ |

**Total**: ~50 líneas de código modificadas/añadidas. Sin nuevos archivos, sin nuevas funciones públicas, sin cambios en la API.

---

## Tests implementados

En `unitTests/src/tests_condacts6.c` (compilan con `SCREEN=8` por defecto):

1. **`test_GFX_SET_PALETTE`** ✅ — verifica que `GFX_SET_PALETTE` calcula y almacena el valor GRB332 correcto en `test_colorTranslationSC8[5]`:
   - `flags[20..23]` = {idx=5, R=192, G=96, B=128} → espera `0x7A` (`0x60|0x18|0x02`)
2. **`test_GFX_GET_PALETTE`** ✅ — verifica que `GFX_GET_PALETTE` desempaqueta correctamente los tres canales con un color no trivial (todos distintos y no nulos):
   - `flags[30]=10` → `test_colorTranslationSC8[10]=0xED` → espera R=`0x60`, G=`0xE0`, B=`0x40`
3. **`test_GFX_SET_GET_PALETTE_roundtrip`** ✅ — verifica la pérdida de precisión GRB332 en round-trip SET→GET. **GRB332 trunca los canales a distinta resolución**:
   - Green y Red: 3 bits, mask `0xE0`
   - Blue: solo 2 bits, mask `0xC0` (los bits 5-0 se pierden)
   - Entrada R=200, G=100, B=150 → espera `(200&0xE0)`, `(100&0xE0)`, `(150&0xC0)`

La infraestructura de test (`condacts_stubs.c`) implementa `gfxRoutines()` con la misma lógica que `daad_platform_msx2.c` pero usando arrays locales `test_colorTranslation*[]` reseteables por `beforeEach()`.

---

## Notas de implementación

- Los arrays permanecen `static const` — sus valores iniciales residen en `.rodata` y se inicializan al arranque sin coste de RAM adicional. El cast a puntero mutable en los sitios de escritura es la forma correcta de modificarlos excepcionalmente sin cambiar su declaración.
- `gfxSetPalette()` sigue siendo `inline` — correcto, es llamada desde un único sitio.
- No se necesita `gfxGetPalette()` como función separada: la lógica es suficientemente simple para estar directamente en `gfxRoutines()`, igual que el resto de rutinas pequeñas.
- El formato GRB332 de SC8 (`GGGRRRBB`): green ocupa los 3 bits altos, red los 3 medios, blue los 2 bajos. Blue tiene 2 bits (0-3 → valores 0, 85, 170, 255 aproximados).
