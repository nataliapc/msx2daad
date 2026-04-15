# PRP011 — Optimizaciones y correcciones en daad_platform_msx2.c

- **ID**: PRP011
- **Tipo**: Bugfix
- **Ficheros afectados**: [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c)
- **Severidad**: Crítico (bug de hardware PSG)
- **Fecha**: 2026-04-15
- **Estado**: Completado ✅
- **Commit**: staged

---

## 1. Resumen

Análisis del fichero `daad_platform_msx2.c` a partir de la documentación técnica MSX (PSG AY-3-8910). Se detectaron **1 bug crítico de hardware** y **1 mejora menor de robustez**.

### Prioridad

| # | Tipo | Severidad |
|---|---|---|
| 1 | Bug PSG registro 7 — bits 6/7 incorrectos | **Crítico** — puede dañar hardware real |
| 2 | `sfxWriteRegister()` sin guarda para registro 7 | **Bajo** — robustez |

---

## 2. Bug crítico — PSG Registro 7, bits 6 y 7 incorrectos

### 2.1 Contexto

La documentación MSX (MSX Red Book, MSX I/O Ports Overview, MSX2 Technical Handbook cap. 5) es explícita:

> *"Register 7 must always contain `10xxxxxx` or possible damage could result to the PSG — there are active devices connected to its I/O pins."*

- **Bit 7** (`IOB`): siempre debe ser **1** (port B = output hacia Joystick)
- **Bit 6** (`IOA`): siempre debe ser **0** (port A = input desde Joystick)
- Rango válido para registro 7: `0x80`–`0xBF` (`0b10xxxxxx`)

Los bits 0–5 controlan el mixer (tone/noise por canal, 0=ON, 1=OFF).

### 2.2 Ubicación del bug

**`sfxInit()`**:
```asm
ld   a,#0b00111111   ; ← BIT 7 = 0, BIT 6 = 0 — PELIGROSO
```

**`sfxTone()`** — dos puntos:
```asm
ld   e,#0b00111110   ; ← enable ChannelA tone — BIT 7 = 0 — PELIGROSO
; ...
ld   e,#0b00111111   ; ← disable ChannelA — BIT 7 = 0 — PELIGROSO
```

### 2.3 Corrección aplicada

| Uso | Valor anterior | Valor corregido |
|---|---|---|
| `sfxInit()` — todo silenciado | `0b00111111` (0x3F) | `0b10111111` (0xBF) |
| `sfxTone()` — enable channel A | `0b00111110` (0x3E) | `0b10111110` (0xBE) |
| `sfxTone()` — disable channel A | `0b00111111` (0x3F) | `0b10111111` (0xBF) |

---

## 3. Mejora menor — `sfxWriteRegister()` sin protección para registro 7

### 3.1 Contexto

`sfxWriteRegister()` escribe directamente al PSG sin filtrar los bits 6/7 del valor cuando `reg=7`, lo que reproduce el mismo riesgo que el bug del apartado 2.

### 3.2 Implementación aplicada

Se combinaron opción A (comentario de advertencia) y opción B (guarda en ensamblador):

```asm
out  (0xa0),a           ; Escribir número de registro
cp   #7                 ; Comparar reg con 7
ld   a,l                ; Cargar valor
jr   nz,keep_mixer$     ; Si reg!=7: pasar valor tal cual (sin modificar)
and  #0b00111111        ; Si reg==7: aplicar guarda
or   #0b10000000        ;   → bit7=1, bit6=0 asegurados en el mixer
keep_mixer$:
out  (0xa1),a           ; Escribir valor
ret
```

La guarda aplica la máscara **únicamente cuando `reg==7`** (el único registro del PSG con requisitos en los bits de dirección I/O). Para el resto de registros el valor se escribe sin modificar.

---

## 4. Plan de implementación

### Fase 1 — Bug crítico ✅

- [x] Corregir `sfxInit()`: `0b00111111` → `0b10111111`
- [x] Corregir `sfxTone()` — enable: `0b00111110` → `0b10111110`
- [x] Corregir `sfxTone()` — disable: `0b00111111` → `0b10111111`
- [ ] Verificar en openMSX que el sonido funciona igual tras el cambio

### Fase 2 — Mejoras menores ✅

- [x] Añadir comentario de advertencia en `sfxWriteRegister()`
- [x] Añadir guarda ASM en `sfxWriteRegister()` para proteger bits 6/7

---

## 5. Referencias

- MSX Red Book — PSG Register 7: *"10xxxxxx or damage could result"*
- MSX I/O Ports Overview — PSG: *"bit 6 must be 0, bit 7 must be 1"*
- MSX2 Technical Handbook, Cap. 5 — PSG registers
- [`src/daad_platform_msx2.c`](../src/daad_platform_msx2.c) — fichero modificado
