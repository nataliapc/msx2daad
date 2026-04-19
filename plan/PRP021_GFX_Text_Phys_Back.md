# PRP021 — GFX 7 / GFX 8: Text Write to Phys / Back

- **ID**: PRP021
- **Tipo**: Implementación (feature ausente)
- **Ficheros afectados**:
  - [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c)
  - [`include/daad_platform_api.h`](../include/daad_platform_api.h)
  - [`unitTests/src/condacts_stubs.c`](../unitTests/src/condacts_stubs.c)
  - [`unitTests/src/condacts_stubs.h`](../unitTests/src/condacts_stubs.h)
  - [`unitTests/src/tests_condacts6.c`](../unitTests/src/tests_condacts6.c)
  - [`wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md`](../wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md)
- **Severidad**: MINOR — funcionalidad opcional ausente en MSX2 (marcada `**UNUSED YET**` en el header)
- **Fecha**: 2026-04-19
- **Estado**: Completado ✅

---

## Problem

Las rutinas `GFX_TEXTS_IN_PHYS` (7) y `GFX_TEXTS_IN_BACK` (8) están definidas en [include/daad_platform_api.h:89-90](../include/daad_platform_api.h#L89-L90) con el comentario `//**UNUSED YET**`, pero el `switch` de [`gfxRoutines()`](../src/daad_platform_msx2.c#L943) no las maneja. El resultado es que un `GFX 0 7` o `GFX 0 8` en una aventura es silenciosamente ignorado en MSX2, mientras que el manual DAAD y la tabla del AtariST sí las definen:

**Referencia** — [docs/GFX.md:13-14](../docs/GFX.md#L13-L14):
```
7       Text Write to Phys      -ST only
8       Text Write to Back      -ST only
```

La semántica es simétrica a GFX 3/4 (`GRAPHICS_IN_PHYS/BACK`): **cambia la página VRAM destino de la escritura de texto**. Por defecto el texto va a Phys (página visible); tras `GFX 0 8` se dirige a Back (página oculta) hasta el siguiente `GFX 0 7`. Permite preparar texto en una página oculta y luego hacer un SWAP/COPY para mostrarlo sin flicker.

El comentario `"ST only"` en el manual es histórico (referencia a las capacidades de doble buffer completo del AtariST); en MSX2 tenemos exactamente la misma infraestructura de doble página VRAM que ya explota GFX 3/4, por lo que es implementable sin dependencias adicionales.

---

## Análisis técnico

### Infraestructura VRAM ya existente

En MSX2DAAD la VRAM está organizada en dos páginas de 256 líneas:
- Página 0 (Phys por defecto): `y = 0..255`
- Página 1 (Back por defecto): `y = 256..511`

Todas las operaciones gráficas usan coordenadas `y` absolutas sobre este espacio de 512 líneas. El engine ya tiene **una** variable de redirección equivalente para imágenes:

```c
// Offset to set/unset the VRAM page where load images
uint32_t gfxPictureOffet;   // 0 o 256*BYTESxLINE
```
manejada por `GFX_GRAPHICS_IN_PHYS` (pone 0) y `GFX_GRAPHICS_IN_BACK` (pone `256 * BYTESxLINE`), y sumada a `posVRAM` en `gfxPicturePrepare()` y `gfxPictureShow()`.

El patrón está ya probado en [src/daad_platform_msx2.c:880-881](../src/daad_platform_msx2.c#L880-L881), donde el chunk `IMG_CHUNK_CLS` usa un offset de **32 filas de caracteres** (= 256 px / FONTHEIGHT):

```c
uint8_t pageOffset = gfxPictureOffet ? 32 : 0;
gfxClearScreenBlock(cw->winX, cw->winY+pageOffset, cw->winW, cw->winH);
```

### Rutas de escritura de texto

El texto llega a la VRAM por cuatro puntos de entrada, todos con coordenadas `y` computadas a partir de `cw->cursorY + cw->winY`:

| Función | Archivo:línea | Rol |
|---|---|---|
| `gfxPutChPixels(c, dx, dy)` | [daad_platform_msx2.c:713](../src/daad_platform_msx2.c#L713) | Dibuja un carácter (destino = `dy`) |
| `gfxScrollUp()` | [daad_platform_msx2.c:635](../src/daad_platform_msx2.c#L635) | Scroll 1 línea + clear última fila |
| `gfxClearWindow()` | [daad_platform_msx2.c:611](../src/daad_platform_msx2.c#L611) | CLS de la ventana actual (llamada por `do_CLS`) |
| `gfxClearCurrentLine()` | [daad_platform_msx2.c:623](../src/daad_platform_msx2.c#L623) | Limpia la línea del cursor (eco input) |

Ninguna de ellas participa en la ruta de renderizado de imágenes, por lo que un offset que afecte sólo a estas cuatro no interfiere con GFX 3/4. El contrato deseado:

> Cuando `gfxTextOffset` es activo (256 px / 32 char-rows), **todas** las escrituras de texto y sus clears asociados dirigen a Back. La ruta de imágenes sigue siendo controlada por `gfxPictureOffet`, independientemente.

### Interacción con doble buffer (SWAP) y `currentPage`

`GFX_SWAP_PHYS_BACK` (2) sólo conmuta `currentPage` y el registro VDP de página visible — **no mueve datos**. Por tanto, la relación "Phys/Back" que ve el autor de la aventura es *relativa a la página visible actual*:

- Al arranque: Phys = página 0 (y=0..255), Back = página 1 (y=256..511).
- Tras un SWAP: Phys = página 1, Back = página 0.

El código actual de GFX 3/4 (pictures) **no** tiene en cuenta `currentPage`: asume siempre que Phys ≡ página 0 y Back ≡ página 1. Es un diseño consciente: tras un SWAP las coordenadas siguen siendo las de la página física, no las de la "visible" lógica. Para mantener coherencia, **GFX 7/8 debe seguir la misma convención** (Phys = offset 0, Back = offset 256). Cualquier interacción con SWAP queda como responsabilidad del autor, igual que con GFX 3/4.

> **Nota**: en una lectura estricta del manual DAAD, "Phys" es la página visible y "Back" la oculta. La implementación actual de GFX 3/4 no cumple esa lectura estricta tras un SWAP. Corregir esa semántica queda fuera de alcance de este PRP: **GFX 7/8 replica el mismo comportamiento que GFX 3/4** para no introducir inconsistencia interna.

### Tamaño del cambio

- 1 variable estática nueva (`gfxTextOffset`, `uint16_t`, 2 bytes RAM).
- 2 `case` en el `switch` de `gfxRoutines()` (8 líneas).
- 1 línea añadida en `gfxPutChPixels()` (suma offset a `dy`).
- 3 ajustes puntuales en `gfxScrollUp`, `gfxClearWindow`, `gfxClearCurrentLine`.
- 1 inicialización en el bloque de init (línea 528-530).
- Total estimado: **~15 líneas** en `src/daad_platform_msx2.c`.

---

## Cambios necesarios

### 1. Declarar `gfxTextOffset` (sección de variables)

**Archivo**: [src/daad_platform_msx2.c:66-71](../src/daad_platform_msx2.c#L66-L71)

```c
// Offset to set/unset the graphical charset
static bool offsetText;
// Offset to set/unset the VRAM page where load images
uint32_t gfxPictureOffet;
// Offset (in pixels) to set/unset the VRAM page where write text (GFX 7/8)
static uint16_t gfxTextOffset;
// Current VRAM page visible
static bool currentPage;
```

> `static` porque no hay acceso externo. Tipo `uint16_t` porque se suma a coordenadas `y` en píxeles (rango 0..511).

### 2. Inicializar en `gfxSetScreen()`

**Archivo**: [src/daad_platform_msx2.c:527-530](../src/daad_platform_msx2.c#L527-L530)

```c
//Initialize variables
offsetText = false;
currentPage = false;
gfxPictureOffet = 0;
gfxTextOffset = 0;        // <-- nuevo
```

### 3. Sumar offset en `gfxPutChPixels()`

**Archivo**: [src/daad_platform_msx2.c:713-714](../src/daad_platform_msx2.c#L713-L714)

```c
static void gfxPutChPixels(uint8_t c, uint16_t dx, uint16_t dy)
{
    dy += gfxTextOffset;     // <-- redirige a Back cuando GFX 8 activo
    c -= 16;
    ...
}
```

> Se suma en el destino `dy`, nunca en `sy` (sy es la fuente del tipo de letra, ubicada fuera del área de páginas).

### 4. Ajustar `gfxScrollUp()` — src + dst + clear final

**Archivo**: [src/daad_platform_msx2.c:635-642](../src/daad_platform_msx2.c#L635-L642)

```c
inline void gfxScrollUp()
{
    ASM_HALT;
    uint16_t py = gfxTextOffset;                 // 0 o 256 px
    uint8_t  pr = gfxTextOffset ? 32 : 0;        // 0 o 32 filas char
    if (cw->winH > 1) {
        bitBlt(cw->winX*FONTWIDTH, (cw->winY+1)*FONTHEIGHT + py,
               cw->winX*FONTWIDTH,  cw->winY   *FONTHEIGHT + py,
               cw->winW*FONTWIDTH, (cw->winH-1)*FONTHEIGHT,
               0, 0, CMD_HMMM);
    }
    gfxClearScreenBlock(cw->winX, cw->winY+cw->winH-1 + pr, cw->winW, 1);
}
```

### 5. Ajustar `gfxClearWindow()` y `gfxClearCurrentLine()`

**Archivo**: [src/daad_platform_msx2.c:611-626](../src/daad_platform_msx2.c#L611-L626)

```c
inline void gfxClearWindow()
{
    uint8_t pr = gfxTextOffset ? 32 : 0;
    gfxClearScreenBlock(cw->winX, cw->winY + pr, cw->winW, cw->winH);
}

inline void gfxClearCurrentLine()
{
    uint8_t pr = gfxTextOffset ? 32 : 0;
    gfxClearScreenBlock(cw->winX, cw->winY + cw->cursorY + pr, cw->winW, 1);
}
```

> Reutilizamos el mismo patrón `pageOffset = gfxTextOffset ? 32 : 0` que ya existe en el manejo de `IMG_CHUNK_CLS` ([daad_platform_msx2.c:880](../src/daad_platform_msx2.c#L880)).

### 6. Añadir los `case` en `gfxRoutines()`

**Archivo**: [src/daad_platform_msx2.c:973-975](../src/daad_platform_msx2.c#L973-L975) — dentro del bloque `#ifndef DISABLE_GFX`, justo después de `GFX_GRAPHICS_IN_BACK`:

```c
        //=================== Graphics Write to Back (4)
        case GFX_GRAPHICS_IN_BACK:
            gfxPictureOffet = 256l * BYTESxLINE;
            break;
        //=================== Text Write to Phys (7)
        case GFX_TEXTS_IN_PHYS:
            gfxTextOffset = 0;
            break;
        //=================== Text Write to Back (8)
        case GFX_TEXTS_IN_BACK:
            gfxTextOffset = 256;
            break;
#endif//DISABLE_GFX
```

> Ubicados **dentro** de `#ifndef DISABLE_GFX` por coherencia con 3/4: si se compila sin soporte gráfico opcional, estas rutinas tampoco tienen sentido. Si el usuario usa `DISABLE_GFX`, las rutinas 5/6 (CLEAR) siguen activas para borrado mínimo — igual que antes.

### 7. Actualizar el header

**Archivo**: [include/daad_platform_api.h:89-90](../include/daad_platform_api.h#L89-L90)

```c
#define GFX_TEXTS_IN_PHYS           7
#define GFX_TEXTS_IN_BACK           8
```

Quitar el comentario `//**UNUSED YET**` de ambas líneas.

### 8. Actualizar la wiki

**Archivo**: [wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md:751-752](../wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md#L751-L752)

Cambiar las celdas MSX2 de rutinas 7 y 8 de `:x:` a `:heavy_check_mark:` y retirar la anotación `-ST only-` del texto de la columna de descripción (o mantenerla sólo en la documentación histórica). Propuesta:

```
| 7 | Text Write to Phys | :heavy_check_mark: | :x: | :x: | :heavy_check_mark: |
| 8 | Text Write to Back | :heavy_check_mark: | :x: | :x: | :heavy_check_mark: |
```

---

## Casos límite y consideraciones

### Interacción con `gfxPictureOffet`

Las dos variables son **independientes**. Un uso típico:

```
GFX 0 4    ; imágenes → Back
PICTURE 5  ; carga imagen en Back
GFX 0 8    ; texto    → Back
MESSAGE 10 ; escribe texto sobre la imagen en Back
GFX 0 0    ; copia Back → Phys (presenta compuesto)
```

El engine nunca mezcla las dos (texto no llama a `gfxPictureOffet`, imágenes no llaman a `gfxTextOffset`).

### `do_CLS` con texto en Back

`do_CLS()` ([daad_condacts.c:1730](../src/daad_condacts.c#L1730)) llama a `gfxClearWindow()` y luego `_internal_windowReset()`. Con el cambio, si `gfxTextOffset = 256`, el CLS borra la ventana en la página Back, no en Phys. Esto es el comportamiento esperado: el texto subsiguiente va a Back, así que el borrado previo debe ir también a Back.

### Windows con scrolling cruzando páginas

`cw->winH * FONTHEIGHT` máximo = 24 filas × 8 px = 192 px. Con offset de 256 el bitBlt alcanza `256 + 192 = 448 < 512`. No hay desbordamiento.

### Modos con VRAM distinta (SC10/SC12 YJK)

Los modos YJK (SCREEN 10, 12) usan la misma organización de 512 líneas de VRAM visibles y ya soportan GFX 3/4 con el mismo esquema. Los nuevos `case` no dependen del modo, por lo que funcionan sin `#if SCREEN` adicionales.

### Save state (`ramsave` / save game)

`gfxTextOffset` **no** forma parte del estado de la aventura (no es un flag DAAD ni una variable del parser). No necesita persistirse en save/load, igual que `gfxPictureOffet`. En cualquier caso, ambas vuelven a 0 en `gfxSetScreen()` (llamada tras `RAMLOAD`), por lo que el efecto post-load es determinista.

### `DISABLE_GFX`

Si se compila con `-DDISABLE_GFX`, los nuevos `case` quedan excluidos (están dentro del `#ifndef DISABLE_GFX` como 3/4). Una aventura que use `GFX 0 7` o `GFX 0 8` en esa build caerá al `default` del switch (no-op), mismo comportamiento que hoy para 3/4. Esto mantiene la semántica de `DISABLE_GFX` ("sin soporte de paginado de escritura"), coherente con la política actual.

### Tamaño de la ROM / RAM

- `gfxTextOffset` añade 2 bytes a `.data` (inicializado en runtime a 0 por `gfxSetScreen()`, pero la variable global consume 2 bytes BSS/data).
- Código: los nuevos 2 `case`, más las sumas en 4 funciones. Estimación < 40 bytes de Z80. Negligible frente al footprint actual del intérprete.

---

## Resumen de archivos modificados

| Archivo | Cambio |
|---------|--------|
| [src/daad_platform_msx2.c](../src/daad_platform_msx2.c) | Declara `gfxTextOffset`, lo inicializa en `gfxSetScreen()` |
| [src/daad_platform_msx2.c](../src/daad_platform_msx2.c) | `gfxPutChPixels()`: suma `gfxTextOffset` a `dy` |
| [src/daad_platform_msx2.c](../src/daad_platform_msx2.c) | `gfxScrollUp()`: aplica offset a src/dst del bitBlt y al clear final |
| [src/daad_platform_msx2.c](../src/daad_platform_msx2.c) | `gfxClearWindow()`, `gfxClearCurrentLine()`: suman `pr` (0/32) |
| [src/daad_platform_msx2.c](../src/daad_platform_msx2.c) | `gfxRoutines()`: añade `case GFX_TEXTS_IN_PHYS` y `case GFX_TEXTS_IN_BACK` |
| [include/daad_platform_api.h](../include/daad_platform_api.h) | Retira el comentario `//**UNUSED YET**` de las dos constantes |
| [unitTests/src/condacts_stubs.c](../unitTests/src/condacts_stubs.c) | Añade variable `test_gfxTextOffset` + casos 7/8 en el stub de `gfxRoutines()` + reset en `beforeEach()` |
| [unitTests/src/condacts_stubs.h](../unitTests/src/condacts_stubs.h) | `extern uint16_t test_gfxTextOffset;` |
| [unitTests/src/tests_condacts6.c](../unitTests/src/tests_condacts6.c) | 3 tests nuevos (ver §Tests) |
| [wiki/…Condacts quick reference](../wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md) | Marcar 7/8 como soportadas en MSX2 |

**Total**: ~25 líneas de código de producción + tests. Sin cambios de API pública, sin dependencias nuevas.

---

## Tests propuestos

En [unitTests/src/tests_condacts6.c](../unitTests/src/tests_condacts6.c), al nivel de la sección GFX (tras los tests de paleta del PRP020):

1. **`test_GFX_TEXTS_IN_BACK`** — `GFX 0 8` debe poner `test_gfxTextOffset == 256`.
2. **`test_GFX_TEXTS_IN_PHYS`** — tras un 8, un `GFX 0 7` debe devolver `test_gfxTextOffset` a 0.
3. **`test_GFX_TEXTS_default`** — `beforeEach()` resetea a 0; verificar antes de cualquier rutina que `test_gfxTextOffset == 0`.

Patrón idéntico al de `test_GFX_SET_PALETTE` (mismo fichero, líneas 587-606). El stub de `gfxRoutines()` en `condacts_stubs.c` añade:

```c
} else if (routine == GFX_TEXTS_IN_PHYS) {
    test_gfxTextOffset = 0;
} else if (routine == GFX_TEXTS_IN_BACK) {
    test_gfxTextOffset = 256;
}
```

Y `beforeEach()` lo resetea junto con el resto del estado de test (patrón ya establecido en PRP020 §Tests). No se necesitan mocks de VRAM.

---

## Verificación manual (fuera de unit tests)

Secuencia sugerida en una aventura DSF de prueba:

```dsf
/PRO 0
    GFX 0 8        ; text → Back
    MESSAGE 10     ; "Hola Back" escrito en página oculta
    GFX 0 0        ; copia Back → Phys
    GFX 0 7        ; restaura text → Phys (defensivo)
    DONE
```

Resultado esperado: el texto "Hola Back" aparece tras el `GFX 0 0` sin flicker, porque se compuso en la página no visible.

Comprobación cruzada con una build `DISABLE_GFX`: el mismo script no debe romper (los `GFX 0 7/8` se ignoran silenciosamente), y el texto se escribe directamente en Phys como hasta ahora.

---

## Notas de implementación

- El nombre `gfxTextOffset` es consistente con `gfxPictureOffet` (con el typo histórico `Offet` en vez de `Offset`); **no** se propone renombrar el existente en este PRP para mantener el diff mínimo.
- Se usa `uint16_t` (no `uint32_t` como en `gfxPictureOffet`) porque los sumandos de `dy` ya son `uint16_t` y 256 cabe holgadamente. Esto ahorra 2 bytes RAM y evita promociones en Z80.
- El `ifndef DISABLE_GFX` ya rodeaba 3/4 (GRAPHICS_IN_*) — las nuevas 7/8 quedan dentro del mismo bloque por simetría semántica.
- Se deja intencionalmente sin tocar la semántica de SWAP (no invierte Phys/Back lógicamente). Un posible PRP futuro podría introducir una indirección `currentPage`-aware para 3/4/5/6/7/8/128/129 de forma coherente; fuera de alcance aquí.
