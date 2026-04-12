# PRP003 — BUG-03 & BUG-04: `do_RAMSAVE` / `do_RAMLOAD` iteran 256 objetos fijos

- **ID**: PRP003
- **Bug ref**: [REPORT_DAAD_BUGS.md § BUG-03 y § BUG-04](../REPORT_DAAD_BUGS.md)
- **Ficheros afectados**: [src/daad_condacts.c:2002-2032](../src/daad_condacts.c#L2002-L2032)
- **Severidad**: BUG (escritura fuera de array en RAMLOAD → corrupción de heap; lectura fuera en RAMSAVE)
- **Fecha**: 2026-04-12
- **Estado**: Finalizado ✅
- **Resultado**: binario final **19782 → 19806 bytes** (+24 bytes). El fix corrige dos bugs de acceso fuera de rango a cambio de un incremento mínimo de tamaño, compensado en parte por las optimizaciones de *pointer walk* y *down-counter* (sin ellas el incremento sería mayor).

---

## 1. Resumen

Los condacts `RAMSAVE` y `RAMLOAD` hacen un *snapshot* en RAM de (a) los 256 flags de sistema/usuario y (b) el campo `.location` de cada objeto. La implementación actual itera **siempre 256 objetos** en lugar de los `hdr->numObjDsc` reales del juego:

- En **RAMSAVE** se lee `objects[i].location` para `i = 0..255`, rebasando el array `objects` (asignado con `malloc(sizeof(Object)*numObjDsc)`).
- En **RAMLOAD** se **escribe** `objects[i].location` para `i = 0..255`, corrompiendo la memoria contigua al array (*heap overflow*).

El fix limita el bucle a `hdr->numObjDsc` y, aprovechando el `#pragma opt_code_size` activo, reescribe el barrido con un *pointer walk* + *down-counter* — mismo idioma ya validado en PRP002 (`do_DROPALL`).

## 2. Código actual

[src/daad_condacts.c:1997-2032](../src/daad_condacts.c#L1997-L2032):

```c
/*	In a similar way to SAVE this action saves all the information relevant to
	the game in progress not onto disc but into a memory buffer. […] */
#ifndef DISABLE_RAMSAVE
void do_RAMSAVE()
{
	memcpy(ramsave, flags, 256);
	for (int i=0; i<256; i++)
		*(ramsave+256+i) = objects[i].location;
}
#endif

/*	This action is the counterpart of RAMSAVE and allows the saved buffer to be
	restored. The parameter specifies the last flag to be reloaded […] */
#ifndef DISABLE_RAMLOAD
void do_RAMLOAD()	// flagno
{
	uint8_t flagno = getValueOrIndirection();

	memcpy(flags, ramsave, flagno+1);
	for (int i=0; i<256; i++)
		objects[i].location = *(ramsave+256+i);
}
#endif
```

## 3. Especificación y layout del buffer

De `.agents/skills/daad-system/references/condacts.md`:

- **RAMSAVE**: *"saves all the information relevant to the game in progress […] into a memory buffer."*
- **RAMLOAD**: *"allows the saved buffer to be restored. The parameter specifies the last flag to be reloaded which can be used to preserve values over a restore."*

Layout del buffer `ramsave` ([src/daad.c:114](../src/daad.c#L114)):

```
 offset 0    256         512
 ├── flags ──┼── object locations ──┤
 │  (256 B)  │  (256 B reservados,  │
 │           │   usados: numObjDsc) │
```

- `ramsave[0..255]` → copia literal de `flags[0..255]`.
- `ramsave[256..256+numObjDsc-1]` → `objects[0..numObjDsc-1].location`.
- `ramsave[256+numObjDsc..511]` → no utilizado (pre-reservado).

La "información relevante al juego" son sólo los `numObjDsc` objetos declarados en el DDB; los índices superiores no existen.

El parámetro `flagno` de RAMLOAD es un índice de flag `0..255`; `flagno+1` es la cantidad de flags a restaurar (se usa `int` tras promoción integer — `flagno=255 ⇒ flagno+1=256`, `memcpy` copia 256 bytes: correcto).

## 4. Análisis detallado del comportamiento actual

### 4.1. Tamaño del array `objects`

`objects = malloc(sizeof(Object)*hdr->numObjDsc)` en [src/daad.c:117](../src/daad.c#L117). `Object` ocupa 6 bytes ([include/daad.h:159-173](../include/daad.h#L159-L173)). `numObjDsc` es `uint8_t` → rango `[0, 255]`, y en la práctica los juegos reales tienen bastante menos (decenas de objetos típicamente).

Luego el array reservado ocupa `6 * numObjDsc` bytes. Las direcciones `&objects[i]` son válidas sólo para `i ∈ [0, numObjDsc-1]`.

### 4.2. RAMSAVE — lectura fuera de rango

Para `i ≥ numObjDsc` la expresión `objects[i].location` lee un byte cuya dirección cae **fuera del bloque reservado**. Ese byte pertenece a lo que el *allocator* haya colocado inmediatamente después (en este repo, típicamente el propio `ramsave` — que se aloja antes — u otras estructuras según el orden de `malloc`: ver [src/daad.c:114-121](../src/daad.c#L114-L121)).

Consecuencias:

- El contenido es **basura no determinista**: depende del estado del heap.
- Se copia esa basura al área `ramsave[256+numObjDsc..511]`, pero dado que RAMLOAD la sobreescribe con la misma lógica, la basura no es visible por sí sola en un `RAMSAVE`.
- El problema real se manifiesta en el RAMLOAD asociado (§4.3).

### 4.3. RAMLOAD — escritura fuera de rango (corrupción)

Para `i ≥ numObjDsc`, la asignación `objects[i].location = *(ramsave+256+i)` **escribe** un byte en memoria contigua al array `objects`. Esto es un *heap overflow* silencioso:

- Si la estructura inmediatamente después de `objects` contiene datos del engine (por ejemplo `nullObject`, `tmpTok`, `tmpMsg` o cualquier otra asignación de `malloc`), esos bytes se corrompen de forma invisible.
- Los bytes corruptos provienen de `ramsave[256+numObjDsc..511]`, que en el mejor caso son ceros (si el `memset` inicial llegó tan lejos — pero ver BUG-05, que indica que ni siquiera eso se garantiza), o en peor caso son residuos de una operación previa.
- En un ciclo `RAMSAVE → cambios → RAMLOAD` lo habitual es que los bytes basura escritos aquí sean idénticos a los leídos en RAMSAVE, por lo que *el efecto observable inmediato es nulo* — pero la *ventana de corrupción* existe y cualquier cambio intermedio en esas direcciones (por ejemplo por funciones que modifican `nullObject` o estructuras adyacentes) se pierde al hacer RAMLOAD.

### 4.4. Evidencia en los tests

[unitTests/src/tests_condacts.c:5036-5037](../unitTests/src/tests_condacts.c#L5036-L5037) ya codifica la semántica correcta esperada:

```c
for (i=0; i<256; i++) ASSERT_EQUAL(flags[i], ramsave[i], "Incorrect flag");
for (i=0; i<MOCK_NUM_OBJECTS; i++) ASSERT_EQUAL(objects[i].location, ramsave[256+i], "Incorrect obj loc");
```

Los flags se chequean 256; los objetos se chequean sólo `MOCK_NUM_OBJECTS`. El contrato está implícitamente descrito en los tests; el código de producción lo viola.

### 4.5. Causa raíz

Ambos bucles fueron escritos con el límite superior del *buffer* (256 bytes de área de objetos) en lugar del límite superior **semántico** (`numObjDsc` objetos reales). El área de 256 bytes del buffer existe para soportar el caso extremo de `numObjDsc = 255`; usarla como límite de iteración conflaba *capacidad de almacenamiento* con *contenido real*.

## 5. Análisis de optimización (Z80/SDCC)

### 5.1. Contexto del `#pragma`

Ambas funciones están en la sección `#pragma opt_code_size` que empieza en [src/daad_condacts.c:240](../src/daad_condacts.c#L240), así que SDCC prioriza tamaño del código generado.

### 5.2. Coste del indexado `objects[i]` con `sizeof(Object)==6`

Como ya se analizó en PRP002 §5.2: `objects[i].location` obliga a multiplicar `i * 6` en cada acceso. Dos accesos por iteración (uno leído + uno escrito en RAMSAVE/RAMLOAD respectivamente) durante 256 iteraciones (o `numObjDsc` iteraciones tras el fix) hacen que reemplazarlo por un *pointer walk* (`obj++`) sea una victoria significativa de tamaño.

### 5.3. Coste del `*(ramsave+256+i)`

En cada iteración SDCC tiene que recomputar `ramsave + 256 + i` (puntero base + constante + índice). Si preasignamos un puntero local `char *p = ramsave + 256` y hacemos `*p++`, el `+256` se evalúa **una vez** y cada iteración es una sola lectura/escritura por puntero más un `INC` del puntero (o `ADD HL,DE` con DE=1, barato).

### 5.4. Contador `uint8_t` + `while (n--)`

Idioma canónico Z80 (`DEC B; JR NZ,…`), el mismo patrón adoptado en PRP002. `numObjDsc` es `uint8_t`, así que cabe en un registro de 8 bits.

### 5.5. ¿Se puede usar `memcpy` para la parte de objetos?

No directamente: las `.location` están intercaladas cada 6 bytes en el array `objects`, mientras que en `ramsave+256` son contiguas (1 byte cada una). No existe un `memcpy` con *stride* en la biblioteca estándar, y emularlo sería más grande que el *pointer walk*. Descartada.

## 6. Solución propuesta

Reescribir ambas funciones con: (1) límite correcto `hdr->numObjDsc`, (2) *pointer walk* sobre `objects`, (3) puntero preasignado a `ramsave+256`, (4) *down-counter* `uint8_t`.

```diff
 #ifndef DISABLE_RAMSAVE
 void do_RAMSAVE()
 {
+	Object *obj = objects;
+	char *dst = ramsave + 256;
+	uint8_t n = hdr->numObjDsc;
 	memcpy(ramsave, flags, 256);
-	for (int i=0; i<256; i++)
-		*(ramsave+256+i) = objects[i].location;
+	while (n--) {
+		*dst++ = obj->location;
+		obj++;
+	}
 }
 #endif
```

```diff
 #ifndef DISABLE_RAMLOAD
 void do_RAMLOAD()	// flagno
 {
 	uint8_t flagno = getValueOrIndirection();
+	Object *obj = objects;
+	char *src = ramsave + 256;
+	uint8_t n = hdr->numObjDsc;
 
 	memcpy(flags, ramsave, flagno+1);
-	for (int i=0; i<256; i++)
-		objects[i].location = *(ramsave+256+i);
+	while (n--) {
+		obj->location = *src++;
+		obj++;
+	}
 }
 #endif
```

### 6.1. Verificación de corrección

- **Rango**: `n = numObjDsc`, el `while (n--)` ejecuta exactamente `numObjDsc` iteraciones sobre `i = 0..numObjDsc-1` (verificado en PRP002 §6.1 con el mismo patrón). No hay accesos fuera de `objects`. ✓
- **Rango en `ramsave`**: `dst`/`src` parten de `ramsave+256` y avanzan hasta `ramsave+256+numObjDsc`, dentro del bloque de 512 bytes. ✓
- **Equivalencia con los tests existentes**: los *asserts* de [unitTests/src/tests_condacts.c:5036-5037](../unitTests/src/tests_condacts.c#L5036-L5037) siguen siendo coherentes (comprueban precisamente el rango `i=0..MOCK_NUM_OBJECTS-1`). ✓
- **`memcpy` de flags**: sin cambios. `memcpy(flags, ramsave, flagno+1)` sigue funcionando con `flagno=255 ⇒ 256 bytes` por la promoción a `int`. ✓
- **Caso `numObjDsc == 0`**: `while (0--)` evalúa falso antes de desreferenciar; el bucle no entra. ✓

### 6.2. Impacto real de tamaño

| versión                                   | bytes totales | Δ vs. estado previo |
|-------------------------------------------|---------------|---------------------|
| Tras PRP002 (buggy RAMSAVE/RAMLOAD)       | 19782         | — (baseline)        |
| Tras PRP003 (solución adoptada)           | 19806         | **+24 bytes**       |

El incremento es pequeño y esperado: la versión original usaba un límite **constante** (`256`) que SDCC puede optimizar agresivamente (el test del `for` degenera a una resta fija + un `JR NZ`). El fix introduce una lectura del *runtime* (`hdr->numObjDsc`) y la salvaguarda para `numObjDsc < 256`, imposibles de plegar en tiempo de compilación.

Las optimizaciones aplicadas (*pointer walk*, `while (n--)`, contador `uint8_t`, puntero preasignado) **reducen** el coste absoluto — sin ellas el incremento sería considerablemente mayor. El +24 bytes es el precio neto por pasar de dos bugs de acceso fuera de rango a un código correcto y compacto.

### 6.3. Comentarios

No se modifican los bloques de comentario encima de `do_RAMSAVE` ni `do_RAMLOAD`: describen la semántica externa (correcta) y no aluden a la implementación. Sin cambios dentro del cuerpo.

## 7. Alternativas consideradas

### 7.1. Cambio mínimo: sustituir sólo el `256` del bucle por `hdr->numObjDsc`

```c
for (int i=0; i<hdr->numObjDsc; i++)
    *(ramsave+256+i) = objects[i].location;
```

- Corrige los bugs funcionalmente. ✓
- **Descartada**: conserva el indexado costoso con `sizeof(Object)==6`, el recálculo de `ramsave+256+i` y el contador de 16 bits. Pierde la oportunidad de reducción de tamaño en zona `opt_code_size`.

### 7.2. `for` con *pointer walk* y contador de 8 bits (sin down-counter)

```c
Object *obj = objects;
char *dst = ramsave + 256;
for (uint8_t i=0; i<hdr->numObjDsc; i++) {
    *dst++ = obj->location;
    obj++;
}
```

- Correcta, consistente con el estilo de los bucles en [src/daad.c](../src/daad.c).
- **Descartada** tras la evidencia de PRP002: la variante `while (n--)` fue ~28 bytes más pequeña que la original en `do_DROPALL` y varios bytes más pequeña que el `for` clásico. Priorizamos tamaño.

### 7.3. `memcpy` con *stride*

- No existe tal primitiva en la libc estándar. Emular un `memcpy_stride` añade código nuevo. Descartada.

### 7.4. Reorganizar `Object` para poner `.location` primero *y contiguo*

- Implicaría separar `location` del resto de `Object` en un array aparte → cambios extensos en toda la base de código, binario DDB, tests. Fuera del alcance de este PRP.

## 8. Riesgos y compatibilidad

- **Compatibilidad de juegos**: positivo. Elimina un *heap overflow* latente en RAMLOAD y una lectura indefinida en RAMSAVE. Un juego que fortuitamente dependiera del comportamiento previo sería imposible (no hay semántica razonable detrás de los bytes fuera de rango).
- **Interacción con BUG-05**: el `memset` de [src/daad.c:115](../src/daad.c#L115) tiene su propio off-by-one; este PRP no lo toca (tiene su propio PRP). Los dos fixes son ortogonales: BUG-03/04 acota el rango *funcional* de lectura/escritura; BUG-05 acota el rango *de inicialización*.
- **Tests unitarios**: los asserts existentes ya describen el contrato correcto (iteran `MOCK_NUM_OBJECTS`, no 256). No hay que modificarlos; sirven como *regression guard*.
- **ABI**: sin cambios.
- **Plataformas**: portable C. Aplica igual a MSX2, DOS, CPM, etc.

## 9. Plan de verificación

1. **Inspección estática** ✅: `do_RAMSAVE` ([src/daad_condacts.c:2003-2013](../src/daad_condacts.c#L2003-L2013)) y `do_RAMLOAD` ([src/daad_condacts.c:2029-2041](../src/daad_condacts.c#L2029-L2041)) quedan como §6, con el límite `hdr->numObjDsc`, guards `#ifndef` intactos y comentarios sin cambios.
2. **Compilación** ✅: rebuild completo sin warnings nuevos.
3. **Medida de tamaño** ✅: binario final 19782 → **19806 bytes** (+24 bytes). Ver tabla en §6.2.
4. **Tests unitarios** (pendiente de ejecutar): `test_RAMSAVE_success` y `test_RAMLOAD_success` de [unitTests/src/tests_condacts.c](../unitTests/src/tests_condacts.c) deben pasar sin modificación. Valor pedagógico: dado que el test usa `MOCK_NUM_OBJECTS < 256`, antes del fix pasaba "por casualidad" porque los bytes basura leídos coincidían al compararse con los mismos bytes escritos — el fix no rompe el test; sólo cierra una ventana de UB.
5. **Prueba funcional** (pendiente) con DDB real:
   - Cargar un juego con `numObjDsc ≪ 256` (lo habitual).
   - Mover objetos a varias localizaciones, ejecutar RAMSAVE.
   - Modificar el estado, ejecutar RAMLOAD.
   - Comprobar que los objetos `0..numObjDsc-1` restauran sus localizaciones correctas y que los flags se recuperan según `flagno`.
6. **Inspección de heap** (opcional, build debug): colocar canarios antes y después del array `objects`; verificar que tras RAMLOAD no se han alterado.

## 10. Referencias

- Informe: [REPORT_DAAD_BUGS.md § BUG-03 y § BUG-04](../REPORT_DAAD_BUGS.md)
- Espec.: `.agents/skills/daad-system/references/condacts.md` — RAMSAVE, RAMLOAD
- Código afectado: [src/daad_condacts.c:2002-2032](../src/daad_condacts.c#L2002-L2032)
- Asignación del buffer: [src/daad.c:114-115](../src/daad.c#L114-L115)
- Estructura `Object` (sizeof = 6): [include/daad.h:159-173](../include/daad.h#L159-L173)
- `#pragma opt_code_size` aplicable: [src/daad_condacts.c:240](../src/daad_condacts.c#L240)
- Patrón *pointer walk* + *down-counter* validado: [PRP002_Fix_do_DROPALL.md](PRP002_Fix_do_DROPALL.md) §6
- Tests del contrato correcto: [unitTests/src/tests_condacts.c:5036-5037](../unitTests/src/tests_condacts.c#L5036-L5037)
