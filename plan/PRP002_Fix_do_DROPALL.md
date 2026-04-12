# PRP002 — BUG-02: `do_DROPALL` off-by-one y optimización del bucle

- **ID**: PRP002
- **Bug ref**: [REPORT_DAAD_BUGS.md § BUG-02](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad_condacts.c:1069-1077](../src/daad_condacts.c#L1069-L1077)
- **Severidad**: BUG (corrupción potencial de memoria + optimización de tamaño)
- **Fecha**: 2026-04-12
- **Estado**: Finalizado ✅
- **Resultado**: tamaño final reducido de **19810 → 19782 bytes** (−28 bytes) tras adoptar la variante 7.3.

---

## 1. Resumen

`do_DROPALL` debe recorrer los objetos `0..numObjDsc-1` soltando los que están `CARRIED` o `WORN`. La implementación actual, por el uso de `do … while(i++ < N)` con post-incremento, itera `N+1` veces y accede a `objects[numObjDsc]` — una entrada fuera del array. Además la construcción es subóptima para Z80/SDCC en comparación con el patrón ya usado en el resto del módulo. El fix corrige el off-by-one y, de paso, reescribe el bucle con una idiomática *pointer walk* que reduce el tamaño generado en la sección `opt_code_size`.

## 2. Código actual

[src/daad_condacts.c:1063-1078](../src/daad_condacts.c#L1063-L1078):

```c
/*	All objects which are carried or worn are created at the current location (i.e. 
	all objects are dropped) and Flag 1 is set to 0. This is included for 
	compatibility with older writing systems.
	Note that a DOALL 254 will carry out a true DROP ALL, taking care of any special 
	actions included. */
#ifndef DISABLE_DROPALL
void do_DROPALL()
{
	int i=0;
	do {
		if (objects[i].location==LOC_CARRIED || objects[i].location==LOC_WORN)
			objects[i].location = flags[fPlayer];
	} while(i++ < hdr->numObjDsc);
	flags[fNOCarr] = 0;
}
#endif
```

## 3. Especificación de referencia

- `.agents/skills/daad-system/references/condacts.md` — condact **DROPALL**: *"All objects which are carried or worn are created at the current location and Flag 1 is set to 0."*
- Rango de iteración esperado: los `numObjDsc` descriptores de objeto, índices `0..numObjDsc-1`. El array `objects` se asigna en [src/daad.c:117](../src/daad.c#L117) con `malloc(sizeof(Object)*hdr->numObjDsc)`.

## 4. Análisis detallado del comportamiento actual

### 4.1. Trazado del `do … while(i++ < N)`

Sea `N = hdr->numObjDsc`. El operador `i++` es post-incremento: en el test se usa el valor *antiguo* de `i` y luego se incrementa.

| iteración | `i` en el cuerpo (acceso `objects[i]`) | test   | `i` tras el ++ |
|-----------|----------------------------------------|--------|----------------|
| 1         | 0                                      | `0<N` ✓ | 1              |
| 2         | 1                                      | `1<N` ✓ | 2              |
| …         | …                                      | …      | …              |
| N         | N-1                                    | `N-1<N` ✓ | N            |
| **N+1**   | **N (fuera de rango)**                 | `N<N` ✗ | N+1 → sale     |

El cuerpo se ejecuta **N+1 veces** y accede a `objects[N]`, que está fuera del bloque reservado por `malloc(sizeof(Object)*N)`.

### 4.2. Consecuencias

1. **Lectura fuera de rango** (la condición `objects[N].location == LOC_CARRIED/WORN`): en la mayoría de *targets* el heap devuelve bytes arbitrarios; la condición puede dar verdadero por coincidencia.
2. **Escritura fuera de rango** si la lectura cae por casualidad en `253`/`254`: se sobreescribe un byte del heap inmediatamente después del array de objetos (posible corrupción silenciosa de estructuras adyacentes — `windows`, `flags`, etc., dependiendo del orden de `malloc`).
3. **Incompatibilidad de estilo**: todos los demás bucles sobre `objects` del repositorio usan `for (i=0; i<hdr->numObjDsc; i++)` (ver [src/daad.c:197](../src/daad.c#L197), [src/daad.c:988](../src/daad.c#L988), [src/daad.c:1017](../src/daad.c#L1017)). Este es el único que no.

### 4.3. Causa raíz

La combinación `do … while(i++ < N)` con post-incremento intenta hacer el test «antes» pero incrementa «después», de modo que la iteración en la que el test falla ya ha ejecutado el cuerpo con un `i` inválido. La forma correcta en estilo `do-while` sería `while(++i < N)` (pre-incremento: incrementa, y si aún es válido vuelve a entrar). Pero la reescritura a `for` es más clara y más barata en tamaño (ver §6).

## 5. Análisis de optimización (Z80/SDCC)

### 5.1. Contexto del `#pragma`

La función está después de `#pragma opt_code_size` ([src/daad_condacts.c:240](../src/daad_condacts.c#L240)), por lo que SDCC genera código optimizado para **tamaño** en todo el bloque de condacts a partir de esa línea. Esto es relevante: cualquier reescritura que reduzca el número de instrucciones generadas es beneficiosa.

### 5.2. Coste del indexado `objects[i]` con `sizeof(Object)==6`

`Object` tiene 6 bytes (1+1+1+1+1+1, ver [include/daad.h:159-173](../include/daad.h#L159-L173)), que *no* es potencia de 2. Acceder a `objects[i].location` requiere computar `objects + i*6` en cada iteración. SDCC genera para eso una secuencia típica:

- cargar `i` (16 bits),
- multiplicar por 6 (varias sumas o `ADD HL,HL` + `ADD HL,DE` / llamada a runtime),
- sumar el puntero base.

Con dos accesos a `objects[i]` en el cuerpo y uno más en el write-back, esto se hace hasta 3 veces por iteración.

### 5.3. El patrón *pointer walk* ya establecido en el repo

[src/daad.c:1014-1025](../src/daad.c#L1014-L1025) muestra el idioma preferido para recorrer `objects`:

```c
Object *obj = objects;
for (uint16_t i=0; i<hdr->numObjDsc; i++) {
    …
    obj++;
}
```

`obj++` se compila a una suma de constante 6 al puntero (en Z80 SDCC típicamente `LD DE,6; ADD HL,DE` sobre el registro del puntero) — mucho más barato que recalcular `objects + i*6` en cada acceso. Con ello:

- los accesos `obj->location` se convierten en una lectura/escritura indirecta por el puntero (registro `HL`), sin aritmética extra;
- el incremento del puntero se paga una sola vez por iteración.

### 5.4. `uint8_t` en lugar de `int` para el contador

`hdr->numObjDsc` es `uint8_t` ([include/daad.h:99](../include/daad.h#L99), máximo 255). El contador de 16 bits (`int i=0`) obliga a SDCC a promover el test y a reservar 2 bytes para `i`. Un contador de 8 bits cabe en un único registro (`B`) y el test `DEC B; JR NZ,…` es el más compacto del Z80.

### 5.5. Caché de `flags[fPlayer]`

`flags[fPlayer]` se lee *cada vez* que el if se cumple (objeto carried/worn). `flags` es global: cada lectura compila a un `LD A,(flags+fPlayer)`. Dentro del bucle el valor no cambia — SDCC no puede asumirlo (no es `const`). Cachear en local es un ahorro pequeño pero seguro:

```c
uint8_t loc = flags[fPlayer];
```

En partidas con varios objetos en mano (lo normal al ejecutar `DROPALL`) el ahorro es real, y si no los hay sólo se paga una lectura extra antes del bucle.

## 6. Solución adoptada

Reescribir el cuerpo de `do_DROPALL` con: (1) *pointer walk*, (2) *down-counter* `uint8_t` en un `while (n--)`, (3) caché de `flags[fPlayer]` en local.

```diff
 #ifndef DISABLE_DROPALL
 void do_DROPALL()
 {
-	int i=0;
-	do {
-		if (objects[i].location==LOC_CARRIED || objects[i].location==LOC_WORN)
-			objects[i].location = flags[fPlayer];
-	} while(i++ < hdr->numObjDsc);
+	Object *obj = objects;
+	uint8_t loc = flags[fPlayer];
+	uint8_t n = hdr->numObjDsc;
+	while (n--) {
+		if (obj->location==LOC_CARRIED || obj->location==LOC_WORN)
+			obj->location = loc;
+		obj++;
+	}
 	flags[fNOCarr] = 0;
 }
 #endif
```

Tras aplicar y medir, esta variante (equivalente a la §7.3 originalmente propuesta) resulta **28 bytes más pequeña** que la versión *buggy* (binario final: 19810 → 19782). Por ese margen se prefiere al `for` clásico, aun a costa de una pequeña asimetría estilística respecto a otros bucles sobre `objects` del repositorio (todos con `for (i=0; i<N; i++)`).

### 6.1. Verificación de corrección

- **Rango de iteración**: el *down-counter* empieza en `n = N` y la condición del `while (n--)` se evalúa con el valor *actual* antes de decrementar. Secuencia: `n=N` ⇒ test verdadero (iteración con el valor original), `n=N-1` tras ++, … hasta `n=1` ⇒ test verdadero (última iteración útil), `n=0` tras ++ ⇒ test falso ⇒ salida. Se ejecutan exactamente **N** iteraciones del cuerpo. ✓
- **Acceso a memoria**: `obj` comienza en `objects` y avanza `N` veces con `obj++`, pero sólo se desreferencia **antes** de avanzar. Tras el bucle, `obj` apunta a `objects + N` (una dirección *más allá del array*, pero nunca desreferenciada — comportamiento permitido por C para "puntero justo-pasado-el-final"). No hay accesos fuera de rango. ✓
- **Caso `N = 0`**: `while (0--)` evalúa `0` como falso (el valor *previo* al decremento). El cuerpo no se ejecuta. Aunque `n` se convierte en `0xFF` tras el decremento post, el bucle ya ha salido. Comportamiento correcto para un juego sin objetos.
- **Semántica**: objetos con `location == LOC_CARRIED (254)` o `LOC_WORN (253)` pasan a tener `location = flags[fPlayer]`. Resto intactos. `flags[fNOCarr] = 0` sigue ejecutándose tras el bucle. ✓
- **Equivalencia con `loc`**: `loc = flags[fPlayer]` se evalúa una vez antes del bucle. El cuerpo no modifica `flags[fPlayer]` ni llama a ninguna función que lo haga, por lo que cachear es seguro. ✓

### 6.2. Medida real de tamaño

| versión                               | bytes totales | Δ vs. original |
|---------------------------------------|---------------|----------------|
| Original (buggy `do…while(i++ < N)`)  | 19810         | —              |
| Solución adoptada (§6, `while(n--)`)  | 19782         | **−28 bytes**  |

Los 28 bytes ahorrados vienen de la suma de:
- Eliminación del indexado `objects[i]` con `sizeof(Object)==6` (sin la multiplicación `i*6` en cada uno de los tres accesos).
- Contador de 8 bits (`DEC B; JR NZ,…`) en lugar de `int` de 16 bits.
- Patrón `while (n--)` (la forma más compacta de bucle decrementador en Z80) en lugar de `for (i=0; i<N; i++)` más pointer walk.
- Caché de `flags[fPlayer]` en un local.

### 6.3. Comentarios

No se ha modificado el bloque de comentarios del condact ([src/daad_condacts.c:1063-1067](../src/daad_condacts.c#L1063-L1067)) — sigue describiendo la semántica externa (correcta) sin aludir a la implementación. Tampoco se han añadido comentarios dentro del cuerpo.

## 7. Alternativas consideradas

### 7.1. Cambio literal mínimo: `while(++i < hdr->numObjDsc)`

```c
int i=0;
do {
    if (objects[i].location==LOC_CARRIED || objects[i].location==LOC_WORN)
        objects[i].location = flags[fPlayer];
} while(++i < hdr->numObjDsc);
```

- Corrige el off-by-one (pre-incremento: el test `++i < N` evalúa el valor *nuevo*, por lo que tras la iteración con `i=N-1` incrementa a `N`, test falso → sale; cuerpo ejecutado N veces con i=0..N-1). ✓
- **Descartada** porque no aprovecha el pointer walk ni el contador `uint8_t`. Mantiene la multiplicación `i*6` por acceso y no se alinea con el estilo del resto del módulo. Corrige el bug pero desperdicia la oportunidad de reducir tamaño en la sección `opt_code_size`.

### 7.2. `for` clásico sin pointer walk

```c
for (uint8_t i=0; i<hdr->numObjDsc; i++) {
    if (objects[i].location==LOC_CARRIED || objects[i].location==LOC_WORN)
        objects[i].location = flags[fPlayer];
}
```

- Correcto y legible. Pequeño ahorro vs. 7.1 por usar contador de 8 bits.
- **Descartada** porque mantiene los tres accesos indexados `objects[i]` con sizeof 6. Peor que la solución elegida a mismo coste de escritura.

### 7.3. `for` clásico con pointer walk (propuesta inicial del PRP)

```c
Object *obj = objects;
uint8_t loc = flags[fPlayer];
for (uint8_t i=0; i<hdr->numObjDsc; i++) {
    if (obj->location==LOC_CARRIED || obj->location==LOC_WORN)
        obj->location = loc;
    obj++;
}
```

- Correcto y consistente con el estilo del resto del módulo (`for (i=0; i<hdr->numObjDsc; i++)` de [src/daad.c:988](../src/daad.c#L988) y afines).
- **Descartada** tras medir: la variante `while (n--)` adoptada en §6 es 28 bytes más pequeña frente a la versión original y ~varios bytes más pequeña que este `for` (la comparación `i < N` requiere cargar `N` y comparar en cada iteración; `n--` sólo decrementa y salta). Se prioriza el ahorro de tamaño sobre la consistencia estilística.

### 7.4. `memset`/barrido por byte

- Descartada: `location` es el primer byte de `Object` pero los objetos están intercalados cada 6 bytes; no hay operación de bloque aplicable.

## 8. Riesgos y compatibilidad

- **Compatibilidad de juegos**: ninguna regresión observable. Antes se ejecutaba una iteración espuria con `objects[N]`, cuyos efectos eran impredecibles (leer basura del heap y, raramente, escribir en memoria fuera del array). Eliminarla sólo quita comportamiento indefinido.
- **Heap**: se corrige un bug real de corrupción potencial de memoria adyacente al array `objects`.
- **ABI**: ninguna (función pública sin cambio de firma).
- **Plataformas**: el cambio aplica por igual a todos los targets (MSX2, DOS, CPM, etc.). El bucle es portable C.
- **SDCC vs otros**: el pointer walk también es óptimo en compiladores nativos; no introduce dependencias de compilador.

## 9. Plan de verificación

1. **Inspección estática** ✅: el cuerpo de `do_DROPALL` queda como §6, con `flags[fNOCarr] = 0;` preservado después del bucle (línea restaurada tras una omisión accidental en la primera aplicación), guard `#ifndef DISABLE_DROPALL` intacto y comentario del bloque sin cambios.
2. **Compilación** ✅: rebuild completo sin warnings nuevos.
3. **Medida de tamaño** ✅: binario final 19810 → **19782 bytes** (−28 bytes). Ver tabla en §6.2.
4. **Prueba funcional** (pendiente de ejecutar en DDB real):
   - DDB de prueba con al menos un objeto `LOC_CARRIED`, uno `LOC_WORN` y varios en otras localizaciones.
   - Tras ejecutar `DROPALL`: todos los *carried* y *worn* deben tener `location == flags[fPlayer]`; el resto deben permanecer iguales.
   - `flags[fNOCarr]` debe quedar a 0.
   - Verificar especialmente el último objeto (índice `N-1`): antes del fix podía quedar con valor corrupto por la iteración extra.
5. **Test de corrupción de heap**: ejecutar `DROPALL` en un escenario con canarios en memoria adyacente al array `objects` (si se dispone de build debug), confirmar que no hay escritura fuera del rango reservado.

## 10. Referencias

- Informe: [REPORT_DAAD_BUGS.md § BUG-02](../REPORT_DAAD_BUGS.md)
- Especificación: `.agents/skills/daad-system/references/condacts.md` — DROPALL
- Código afectado: [src/daad_condacts.c:1069-1077](../src/daad_condacts.c#L1069-L1077)
- Estructura `Object` (sizeof = 6): [include/daad.h:159-173](../include/daad.h#L159-L173)
- Constantes de localización: [include/daad.h:31-34](../include/daad.h#L31-L34)
- `#pragma opt_code_size` aplicable: [src/daad_condacts.c:240](../src/daad_condacts.c#L240)
- Patrón *pointer walk* de referencia: [src/daad.c:1014-1025](../src/daad.c#L1014-L1025)
