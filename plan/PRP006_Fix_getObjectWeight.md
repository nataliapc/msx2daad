# PRP006 — BUG-07: `getObjectWeight` — recursión infinita y algoritmo incorrecto

- **ID**: PRP006
- **Bug ref**: [REPORT_DAAD_BUGS.md § BUG-07](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad.c:999-1025](../src/daad.c#L999-L1025)
- **Severidad**: BUG crítico (recursión infinita + cálculo incorrecto del peso de contenedores)
- **Fecha**: 2026-04-12
- **Estado**: Finalizado ✅
- **Resultado**: binario final **19739 → 19768 bytes** (+29 bytes). Coste asumible dado que se corrige un crash (recursión infinita), se arregla un algoritmo fundamentalmente roto que nunca sumaba contenidos de contenedores, y se preserva la semántica spec del "saco mágico" (contenedor de peso 0 transmite peso cero).

---

## 1. Resumen

`getObjectWeight` debe devolver el peso total de un objeto incluyendo su contenido si es un contenedor, o el peso agregado de los objetos que lleva el jugador (transportados + llevados puestos). La implementación actual tiene **tres defectos** simultáneos:

1. **Recursión infinita** cuando se invoca con un `objno` concreto sobre un contenedor de peso > 0: `getObjectWeight(i, false)` se recursa con los mismos parámetros que sigue matcheando en la iteración interna → bucle sin fin y desbordamiento de pila.
2. **Nunca suma el contenido** de los contenedores: el bucle sólo matchea `objno==i`, es decir, el propio objeto por su ID, no los objetos cuya `location == i` (que es lo que contiene el contenedor).
3. **Doble conteo** del peso del propio contenedor cuando la recursión no entra en bucle (en ejecuciones "suerte" donde weight=0): la llamada recursiva añade el peso del contenedor y la línea siguiente lo añade otra vez.

**Nota sobre el guard `weight!=0` del código original**: *no* es espurio. La spec del condact WEIGH indica explícitamente que "*objects in zero weight containers, also weigh zero*" — un contenedor de peso 0 es un "saco mágico" que transmite peso cero tanto de sí mismo como de su contenido. El bug NO está en ese guard sino en que la recursión que protege está rota por los defectos 1-3. El fix preservará la semántica del guard.

## 2. Código actual

[src/daad.c:1010-1025](../src/daad.c#L1010-L1025):

```c
uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn)
{
	uint16_t weight = 0;
	Object *obj = objects;

	for (uint16_t i=0; i<hdr->numObjDsc; i++) {
		if ((objno==NULLWORD || objno==i) && (!isCarriedWorn || obj->location==LOC_CARRIED || obj->location==LOC_WORN)) {
			if (obj->attribs.mask.isContainer && obj->attribs.mask.weight!=0) {
				weight += getObjectWeight(i, false);
			}
			weight += obj->attribs.mask.weight;
		}
		obj++;
	}
	return weight>255 ? 255 : (uint8_t)weight;
}
```

## 3. Especificación y llamadas reales

De la spec (`.agents/skills/daad-system/references/condacts.md`) y del comentario en el fuente:

- **WEIGH objno flagno**: coloca en `flagno` el peso total del objeto `objno` incluyendo sus contenidos si es contenedor.
- **WEIGHT flagno**: coloca en `flagno` el peso total de lo que lleva encima el jugador (carried + worn) incluyendo contenidos de contenedores.

Los únicos dos patrones de llamada en el código son:

1. [src/daad_condacts.c:684](../src/daad_condacts.c#L684), [src/daad_condacts.c:1036](../src/daad_condacts.c#L1036): `getObjectWeight(NULLWORD, true) + getObjectWeight(objno, false)` — checks de capacidad antes de `GET`/`WEAR`.
2. [src/daad_condacts.c:1383](../src/daad_condacts.c#L1383): `getObjectWeight(getValueOrIndirection(), false)` — en do_WEIGH.
3. [src/daad_condacts.c:1559](../src/daad_condacts.c#L1559): `getObjectWeight(NULLWORD, true)` — en do_WEIGHT.

Es decir, sólo se usan dos combinaciones:

- `getObjectWeight(NULLWORD, true)` — peso agregado carried + worn.
- `getObjectWeight(objno, false)` — peso del objeto concreto (con contenidos).

## 4. Análisis detallado del comportamiento actual

### 4.1. Recursión infinita

Trazado de `getObjectWeight(5, false)` con `objects[5]` siendo contenedor de peso 1:

- Bucle `i=0..N-1`. Condición `(objno==NULLWORD || objno==i)` es `(false || 5==i)`.
- En `i=5`: matchea. `obj->isContainer` true, `obj->weight==1` → entra al `if`.
- Llama `getObjectWeight(5, false)` — **mismos parámetros**.
- En la recursión: la única iteración que matchea es `i=5` otra vez → misma llamada recursiva.
- Se apila cada recursión en el stack → desbordamiento.

Igualmente para `getObjectWeight(NULLWORD, true)`: en el bucle externo, al toparse con un contenedor carried/worn de peso>0, se llama `getObjectWeight(i, false)`. Esa llamada entra en el bucle infinito descrito arriba.

**Conclusión**: cualquier partida con un contenedor *carried*/*worn* con peso > 0 produce desbordamiento de pila (*crash* o *freeze*) al calcular pesos. Las aventuras no crashean hoy porque: (a) muchos contenedores son sacos de peso 0 — esquiva la recursión gracias al `weight!=0`; (b) muchos juegos no usan contenedores.

### 4.2. Algoritmo nunca suma el contenido

El bucle filtra por `objno==i` (el *propio* objeto `i`). La semántica "contenido de un contenedor" exige iterar objetos cuya `location == containerId`, no los que tienen ese ID. El bucle actual **nunca toca los objetos contenidos**, sólo el propio contenedor.

Consecuencia: incluso si la recursión no entrara en bucle, el peso devuelto para un contenedor con objetos dentro sería sólo el peso del propio contenedor, no el total espec'd.

### 4.3. Doble conteo del peso del contenedor

Asumiendo hipotéticamente que la recursión "funcionara":

```c
if (obj->isContainer && obj->weight!=0) {
    weight += getObjectWeight(i, false);  // devuelve peso propio (otra vez)
}
weight += obj->attribs.mask.weight;      // añade peso propio aquí
```

Dado que `getObjectWeight(i, false)` matchearía `i==objno` en su iteración y sumaría `obj[i].weight`, el peso del contenedor se cuenta **dos veces**.

### 4.4. Guard `weight!=0` — spec-compliant, no espurio

El test `obj->attribs.mask.weight!=0` en la condición del `if` interno *sí tiene* justificación spec. Del docstring del condact WEIGH ([tests_condacts.c:3670-3675](../unitTests/src/tests_condacts.c#L3670-L3675)):

> *"If Object objno. is a container of zero weight, Flag flagno will be cleared as objects in zero weight containers, also weigh zero!"*

Es decir: un contenedor de peso 0 es un "saco mágico" que transmite peso cero — no suma ni el suyo ni el de sus contenidos. Esta semántica está integrada en DAAD como una feature deliberada (mochilas sin peso que no penalizan la capacidad del jugador). El guard del código original materializa esta regla.

El bug NO está en el guard sino en que la recursión que éste protege está rota (defectos §4.1-§4.3). El fix debe preservar el guard y arreglar la recursión.

### 4.5. Causa raíz

La función confunde dos operaciones distintas:

- "Sumar el peso de un objeto y su contenido" (la espec).
- "Sumar los pesos que matcheen `objno==i`" (lo que hace el código).

La recursión intenta "bajar al contenedor" pero lo hace con los mismos parámetros, lo que sólo vuelve a seleccionar el mismo objeto. La función es irrecuperable sin una reestructuración del algoritmo.

## 5. Análisis de optimización (Z80/SDCC)

### 5.1. Observación clave: unificar por `location`

`LOC_CARRIED` (254) y `LOC_WORN` (253) son **valores de `location` como cualquier otro**. Por tanto:

- "Suma el peso de los objetos carried + worn (con sus contenidos anidados)" = "suma objetos con `location==LOC_CARRIED`" + "suma objetos con `location==LOC_WORN`".
- "Suma el contenido de un contenedor X" = "suma objetos con `location==X`".

Las dos operaciones son la **misma** con distinto parámetro: "suma todo lo que esté en la localización L". Un único helper recursivo `_sumLocation(loc)` cubre ambos casos. La aggregate mode del `getObjectWeight` colapsa a dos llamadas sumadas.

### 5.2. Contador `uint8_t`, *pointer walk*

Igual que en PRPs anteriores: `numObjDsc` es `uint8_t` → usar `uint8_t` para el índice; recorrer `objects` via puntero (`obj++`) para evitar la multiplicación por `sizeof(Object)=6` en cada acceso. Estas optimizaciones ya estaban presentes en la versión buggy y se mantendrán.

### 5.3. Eliminación del `#pragma opt_code_size`

La función está en `src/daad.c` que no aplica `#pragma opt_code_size` explícitamente (sólo `daad_condacts.c` lo hace). No hay ajuste adicional que hacer aquí.

### 5.4. Elección preferida

**Helper único `_sumLocation` + `getObjectWeight` compacto**. Ver §6.

## 6. Solución propuesta

Un único helper recursivo que suma los pesos de todos los objetos en una localización dada (incluyendo contenedores anidados, salvo los de peso 0 por spec). `getObjectWeight` se convierte en un despachador compacto. Firma pública intacta.

```c
// Sum weights of all objects whose location == loc, descending recursively
// into nested containers. Works uniformly for player-held aggregates
// (LOC_CARRIED / LOC_WORN) and for the contents of a specific container.
// Per the WEIGH spec, a container of zero weight transmits zero weight for
// both itself and its contents ("magic" bag), so its contents are not summed.
static uint8_t _sumLocation(uint8_t loc)
{
	uint16_t weight = 0;
	Object *obj = objects;
	for (uint8_t i=0; i<hdr->numObjDsc; i++) {
		if (obj->location == loc) {
			weight += obj->attribs.mask.weight;
			if (obj->attribs.mask.isContainer && obj->attribs.mask.weight>0)
				weight += _sumLocation(i);
		}
		obj++;
	}
	return weight>255 ? 255 : (uint8_t)weight;
}

uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn)
{
	isCarriedWorn;  // only meaningful with NULLWORD; both callsites pass true there
	if (objno == NULLWORD)
		return _sumLocation(LOC_CARRIED) + _sumLocation(LOC_WORN);

	uint16_t weight = objects[objno].attribs.mask.weight;
	if (objects[objno].attribs.mask.isContainer && weight>0)
		weight += _sumLocation(objno);
	return weight>255 ? 255 : (uint8_t)weight;
}
```

### 6.1. Verificación de corrección

**Caso 1 — contenedor concreto** (backpack w=1, carried; apple w=2 loc=backpack; orange w=3 loc=backpack):

- `getObjectWeight(backpack, false)`: weight = 1 (propio). Container con peso>0 → += `_sumLocation(backpack)`:
  - Itera. apple: +2. orange: +3. Retorna 5.
- Total: 1 + 5 = 6. ✓

**Caso 2 — contenedores anidados con pesos > 0** (caja w=2 carried; mochila w=1 loc=caja; apple w=4 loc=mochila):

- `getObjectWeight(caja, false)`: weight = 2. Container w>0 → += `_sumLocation(caja)`:
  - mochila (loc==caja): +1. Container w>0 → += `_sumLocation(mochila)`:
    - apple (loc==mochila): +4. No container. Retorna 4.
  - Retorna 1+4 = 5.
- Total: 2 + 5 = 7. ✓

**Caso 3 — `getObjectWeight(NULLWORD, true)` con la caja del caso 2 carried**:

- `_sumLocation(LOC_CARRIED)`: caja matchea. +2. Container w>0 → += `_sumLocation(caja)` = 5. Retorna 7.
- `_sumLocation(LOC_WORN)`: 0.
- Total: 7. Sin doble conteo. ✓

**Caso 4 — contenedor "mágico" de peso 0** (saco w=0 carried; oro w=10 loc=saco; gema w=5 loc=saco):

- `getObjectWeight(saco, false)`: weight = 0. Container **pero weight==0** → no recursa. Retorna 0. ✓ (spec: "*objects in zero weight containers, also weigh zero*")
- `getObjectWeight(NULLWORD, true)`: `_sumLocation(LOC_CARRIED)`: saco matchea, +0, container pero w==0 → no recursa. Retorna 0. Total 0. ✓

**Caso 5 — objeto simple `getObjectWeight(apple, false)` con apple w=2, no contenedor**:

- weight = 2. No container. Return 2. ✓

**Caso 6 — `getObjectWeight(NULLWORD, true)` sin contenedores, 3 objetos carried (w=1,2,3)**:

- `_sumLocation(LOC_CARRIED)`: cada uno +1/+2/+3. Ninguno es container. Retorna 6.
- `_sumLocation(LOC_WORN)`: 0.
- Total: 6. ✓

**Ausencia de recursión infinita**: `_sumLocation(X)` sólo recursa en objetos cuya `location==X`, que son contenedores, *y* que tienen peso > 0. El guard de peso > 0 elimina un posible ciclo de recursión trivial (un "contenedor" auto-contenido con peso 0 seguiría siendo barrera). Para ciclos con peso > 0, el problema sería de *data* corrupta del DDB, no del código.

**Nota sobre `isCarriedWorn`**: los dos únicos *callsites* usan `(NULLWORD, true)` o `(objno, false)`. En ambos la semántica queda bien definida por nuestro código. La combinación `(NULLWORD, false)` — "sumar todos los objetos del mundo" — no la usa nadie; en esta implementación se trata igual que `(NULLWORD, true)` (se suma carried+worn). Documentamos el comportamiento y preservamos la firma pública sin cambios.

### 6.2. Compatibilidad con *callsites* existentes

| llamada                                    | semántica previa (buggy) | semántica nueva                   |
|--------------------------------------------|--------------------------|------------------------------------|
| `getObjectWeight(NULLWORD, true)`          | suma parcial + crash si hay contenedor carried con weight>0 | suma total correcta de carried+worn con contenidos |
| `getObjectWeight(objno, false)` (no-container) | peso propio (correcto por casualidad) | peso propio (idem) |
| `getObjectWeight(objno, false)` (container, w=0) | peso 0, contenido ignorado (spec-correcto por casualidad) | peso 0, contenido ignorado (spec-correcto) |
| `getObjectWeight(objno, false)` (container, w>0) | recursión infinita (crash) | peso correcto incluyendo contenido |

Todos los juegos:
- Sin contenedores: comportamiento idéntico.
- Con contenedores de peso 0 ("sacos mágicos"): comportamiento idéntico, spec-correcto. El contenido no suma al peso del jugador — feature de DAAD.
- Con contenedores de peso > 0: antes crasheaban; ahora calculan correctamente la suma del contenedor + contenido. Obvia mejora.

### 6.3. Impacto real de tamaño

| versión                                 | bytes totales | Δ vs. estado previo |
|-----------------------------------------|---------------|---------------------|
| Tras PRP005 (`getObjectWeight` buggy)   | 19739         | — (baseline)        |
| PRP006 inicial (sin guard weight>0)     | 19757         | +18 bytes           |
| **PRP006 final (con guard spec-correcto)** | **19768**  | **+29 bytes**       |

Balance aceptable: el incremento neto compra (a) eliminación de una recursión infinita que crasheaba juegos con contenedores de peso > 0, (b) un algoritmo que ahora sí suma el contenido de contenedores según la spec, (c) preservación de la feature del "saco mágico" (contenedor peso 0 ⇒ peso total 0, incluso con contenido). Sin las optimizaciones ya incorporadas (pointer walk, `uint8_t` counter, helper compartido) el aumento habría sido mayor.

### 6.4. Comentarios

Sin cambios en el docstring de [src/daad.c:999-1009](../src/daad.c#L999-L1009): describe la semántica externa (correcta con el fix) y no menciona detalles de implementación. El helper `_sumLocation` lleva un comentario de dos líneas explicando su contrato general (funciona tanto para contenedores como para agregados carried/worn).

## 7. Alternativas consideradas

### 7.1. Helper específico `_sumContentsOf` + bucle aggregate explícito

```c
static uint8_t _sumContentsOf(uint8_t containerId) {
    // Sum objects whose location == containerId, recursive on sub-containers.
}

uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn) {
    if (objno == NULLWORD) {
        // loop top-level, filter by carried/worn, add weight + _sumContentsOf(i) per container
    } else {
        // own weight + _sumContentsOf(objno) if container
    }
}
```

- Correcta funcionalmente, semánticamente equivalente a §6.
- Más código: el bucle aggregate duplica el patrón "itera + filtra + añade + recursa si container" que ya hace el helper.
- **Descartada** a favor de la variante unificada: aprovechar que `LOC_CARRIED`/`LOC_WORN` son valores de `location` cualquiera elimina el bucle aggregate por completo (se convierte en dos llamadas sumadas al helper).

### 7.2. Fix mínimo: sólo arreglar la recursión sin separar en helper

Intentar corregir el bucle externo para que también matchee `obj->location == objno` y así sume contenidos. Incrementa la complejidad de la condición y la gestión de doble-conteo sin mejora clara. Descartada.

### 7.3. Eliminar el parámetro `isCarriedWorn`

Los únicos patrones de llamada son `(NULLWORD, true)` y `(objno, false)`. El flag `isCarriedWorn` podría fusionarse con `objno` (p.ej. `NULLWORD` siempre implica carried+worn). Simplificaría el API pero cambia la firma pública y requiere tocar los *callsites*. **Descartada** por alcance: este PRP corrige el bug sin cambiar la interfaz.

## 8. Riesgos y compatibilidad

- **Compatibilidad con juegos existentes**: 
  - Juegos sin contenedores: comportamiento idéntico. Sin riesgo.
  - Juegos con contenedores de peso 0 ("sacos mágicos"): comportamiento idéntico, spec-correcto. Antes funcionaban por la combinación del guard `weight!=0` y la suerte de no crashear; ahora funcionan por diseño.
  - Juegos con contenedores de peso > 0 carried/worn: antes *crasheaban* por recursión infinita; ahora calculan correctamente la suma peso-propio + contenido. Obvia mejora.
- **Profundidad de recursión**: limitada por el anidamiento real de contenedores en el juego (típicamente 1-2 niveles). La spec menciona un límite de 10 niveles que **no implementamos** en esta fase — los juegos reales raramente superan 2-3. Un ciclo malformado con contenedores de peso > 0 podría causar desbordamiento; se asume DDB bien formado.
- **ABI**: sin cambios (firma pública idéntica).
- **Plataformas**: portable C.

## 9. Plan de verificación

1. **Inspección estática** ✅: `getObjectWeight` y `_sumLocation` quedan como §6 en [src/daad.c:1010-1036](../src/daad.c#L1010-L1036). Docstring externo intacto.
2. **Compilación** ✅: rebuild completo sin warnings nuevos.
3. **Medida de tamaño** ✅: binario final 19739 → **19768 bytes** (+29 bytes). Ver tabla en §6.3.
4. **Tests unitarios** ✅: batería completa ejecutada → **212 OK / 50 TODO / 0 fallos**. Sin regresiones. Los tests de `test_WEIGH_*` y `test_WEIGHT_*` siguen en TODO por "must mock better getObjectWeight()": el stub de `getObjectWeight` en [unitTests/src/tests_condacts.c:126](../unitTests/src/tests_condacts.c#L126) sustituye la función real, así que los tests no ejercitan el código arreglado. Activar esos tests requiere quitar el stub y dejar que los tests llamen a la implementación real — queda pendiente como mejora de infraestructura de testing.
5. **Regression guard**: añadir un test explícito `test_getObjectWeight_*` si el mockeo de `hdr->numObjDsc` y `objects[]` lo permite (debería, ya que `beforeEach` los inicializa). Tests sugeridos:
   - Objeto simple no-contenedor → su propio peso.
   - Contenedor con 2 ítems → peso propio + suma de ítems.
   - Contenedor con weight=0 con contenidos → peso del contenido (antes: 0).
   - Contenedores anidados (A contiene B contiene C) → suma total.
   - Contenedor carried con weight>0 con contenido: antes crash, ahora valor correcto (regresión clave).
6. **Prueba funcional con DDB real**: cargar un juego con contenedores (`PAWN`-style) y verificar que `WEIGH`/`WEIGHT` devuelven valores consistentes con la inspección manual. Especialmente juegos con sacos/mochilas con contenidos.

## 10. Referencias

- Informe: [REPORT_DAAD_BUGS.md § BUG-07](../REPORT_DAAD_BUGS.md)
- Código afectado: [src/daad.c:999-1025](../src/daad.c#L999-L1025)
- Callsites: [src/daad_condacts.c:684](../src/daad_condacts.c#L684), [src/daad_condacts.c:1036](../src/daad_condacts.c#L1036), [src/daad_condacts.c:1383](../src/daad_condacts.c#L1383), [src/daad_condacts.c:1559](../src/daad_condacts.c#L1559)
- Estructura `Object` (campo `location`, `attribs.mask.isContainer`, `attribs.mask.weight`): [include/daad.h:159-173](../include/daad.h#L159-L173)
- Spec: `.agents/skills/daad-system/references/condacts.md` — WEIGH, WEIGHT
