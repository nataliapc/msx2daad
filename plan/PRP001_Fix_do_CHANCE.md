# PRP001 — BUG-01: `do_CHANCE` off-by-one en el cálculo de probabilidad

- **ID**: PRP001
- **Bug ref**: [REPORT_DAAD_BUGS.md § BUG-01](../REPORT_DAAD_BUGS.md)
- **Fichero afectado**: [src/daad_condacts.c:548](../src/daad_condacts.c#L548)
- **Severidad**: BUG (crítico funcionalmente, trivial de corregir)
- **Fecha**: 2026-04-12

---

## 1. Resumen

El condact `CHANCE percent` debe evaluarse a verdadero con una probabilidad exacta del `percent`% (rango 1-100). La implementación actual usa una comparación estricta `<` en lugar de `<=`, lo que produce un desfase sistemático de -1% en todas las probabilidades:

- `CHANCE 100` falla el 1% de las veces (debería ser siempre verdadero).
- `CHANCE 50` acierta el 49% de las veces (debería ser 50%).
- `CHANCE 1` nunca acierta (debería acertar el 1% de las veces).

## 2. Código actual

[src/daad_condacts.c:541-550](../src/daad_condacts.c#L541-L550):

```c
/*	Succeeds if percent is less than or equal to a random number in the range
	1-100 (inclusive). Thus a CHANCE 50 condition would allow PAW to look at the
	next CondAct only if the random number generated was between 1 and 50, a 50%
	chance of success. */
#ifndef DISABLE_CHANCE
void do_CHANCE()	// percent
{
	checkEntry = (rand()%100)+1 < getValueOrIndirection();
}
#endif
```

## 3. Especificación de referencia

De `.agents/skills/daad-system/references/condacts.md:28`:

> `| 10 | CHANCE | 1 | C | True with percent% probability (1-100) |`

El comentario del propio fichero fuente precisa el comportamiento esperado con un ejemplo concreto:

> *"CHANCE 50 condition would allow PAW to look at the next CondAct only if the random number generated was between 1 and 50, a 50% chance of success."*

Es decir, el condact es cierto cuando el número aleatorio sorteado (en el rango 1..100) está dentro del intervalo `[1, percent]`. La probabilidad exacta de éxito debe ser `percent/100`.

## 4. Análisis detallado del comportamiento actual

### 4.1. Dominio del valor aleatorio

- `rand()%100` produce un entero uniformemente distribuido en el rango `[0, 99]`.
- `(rand()%100)+1` desplaza ese rango a `[1, 100]` (100 valores posibles, equiprobables).

Este rango `1..100` es correcto y coincide con el usado en `do_RANDOM` ([src/daad_condacts.c:1500](../src/daad_condacts.c#L1500)), que implementa la misma fuente pseudoaleatoria del condact `RANDOM flagno` (*"set to a number from the Pseudo-random sequence from 1 to 100"*).

### 4.2. Evaluación de la comparación

Sea `r = (rand()%100)+1`, con `r ∈ {1, 2, …, 100}`, y `p = getValueOrIndirection()` (el valor de `percent`). La implementación actual evalúa:

```
checkEntry = (r < p)
```

`checkEntry` es verdadero sólo cuando `r` toma uno de los valores `{1, 2, …, p-1}`. Hay `p - 1` valores favorables sobre 100 posibles, luego:

```
P(éxito) = (p - 1) / 100
```

### 4.3. Tabla de probabilidades reales vs esperadas

| `percent` | Valores de `r` favorables | Casos favorables | P real (actual) | P esperada | Desviación |
|-----------|---------------------------|------------------|-----------------|------------|------------|
| 1         | ∅                         | 0                | 0%              | 1%         | −1%        |
| 2         | {1}                       | 1                | 1%              | 2%         | −1%        |
| 50        | {1..49}                   | 49               | 49%             | 50%        | −1%        |
| 99        | {1..98}                   | 98               | 98%             | 99%        | −1%        |
| 100       | {1..99}                   | 99               | 99%             | 100%       | −1%        |

### 4.4. Consecuencias observables en juego

1. **`CHANCE 100` no es determinista**: el autor del juego usa a menudo valores 100 como «rama siempre tomada» dentro de un patrón condicional. Con el bug, uno de cada 100 turnos esa rama no se ejecuta, introduciendo fallos aleatorios muy difíciles de reproducir.
2. **`CHANCE 1` es código muerto**: cualquier evento «muy raro» modelado con `CHANCE 1` jamás ocurrirá.
3. **Sesgo sistemático**: aunque −1% es pequeño, afecta a *todos* los valores y se acumula en secuencias largas (por ejemplo, tiradas repetidas en combate).

## 5. Causa raíz

El operador relacional `<` excluye el caso de igualdad. Para que la probabilidad sea exactamente `p/100` con `r ∈ [1, 100]`, necesitamos que `r = p` sea un caso favorable (de hecho, es el caso que falta). Equivalente: el intervalo favorable debe ser `[1, p]` y no `[1, p-1]`.

## 6. Solución propuesta

Sustituir la expresión completa por su forma simplificada equivalente, eliminando el `+1` redundante en [src/daad_condacts.c:548](../src/daad_condacts.c#L548):

```diff
 void do_CHANCE()	// percent
 {
-	checkEntry = (rand()%100)+1 < getValueOrIndirection();
+	checkEntry = (rand()%100) < getValueOrIndirection();
 }
```

Adicionalmente, se recomienda actualizar el comentario del bloque para reflejar la nueva formulación (rango interno `0..99`) sin cambiar el contrato externo.

### 6.1. Demostración de equivalencia con la corrección «ingenua» (`<=`)

La corrección mínima literal del bug sería cambiar `<` por `<=`:

```c
checkEntry = (rand()%100)+1 <= getValueOrIndirection();   // opción A
```

Sea `x = rand()%100`, `x ∈ {0, 1, …, 99}`, y `p = getValueOrIndirection()`:

- **Opción A** (sólo cambiar `<` por `<=`):  `(x + 1) <= p`  ⟺  `x <= p − 1`  ⟺  `x < p`  (aritmética entera).
- **Opción elegida** (eliminar el `+1`):     `x < p`.

Ambas expresiones son **idénticas a nivel booleano para toda semilla**. No se trata sólo de equivalencia estadística: producen el mismo resultado en cada invocación individual. La opción elegida ahorra una suma (`+1`) por llamada.

### 6.2. Verificación de la corrección

Con `x ∈ {0, …, 99}` y la condición `x < p`:

- Casos favorables: `{0, 1, …, p − 1}` → exactamente `p` valores.
- `P(éxito) = p / 100` ✓ (coincide con la especificación).

Casos límite:

| `percent` | x favorables       | nº favorables | P real | Correcto |
|-----------|--------------------|---------------|--------|----------|
| 1         | {0}                | 1             | 1%     | ✓        |
| 50        | {0..49}            | 50            | 50%    | ✓        |
| 99        | {0..98}            | 99            | 99%    | ✓        |
| 100       | {0..99} (todos)    | 100           | 100%   | ✓        |

`CHANCE 100` pasa a ser siempre verdadero como exige la especificación.

### 6.3. Coste y alcance

- Expresión más simple y un operador aritmético menos.
- En Z80/SDCC se ahorra la instrucción de `+1` sobre el valor de 16 bits que devuelve `rand()%100` (unos pocos bytes y ciclos por ejecución). Es un ahorro marginal pero real, y no tiene contraparte negativa.
- Sin cambios en API, sin cambios en el binario DDB, sin cambios en la semántica externa del condact.
- No afecta a `do_RANDOM` ([src/daad_condacts.c:1500](../src/daad_condacts.c#L1500)), que mantiene su expresión `(rand()%100)+1` porque ahí el rango `1..100` *sí* forma parte del contrato (el valor se escribe en un flag legible por el juego: *"set to a number from the Pseudo-random sequence from 1 to 100"*). En `do_CHANCE` el valor aleatorio es interno y nunca se expone, por lo que el rango es un detalle de implementación.

## 7. Alternativas consideradas

### 7.1. Corrección literal: cambiar sólo `<` por `<=`

```c
checkEntry = (rand()%100)+1 <= getValueOrIndirection();
```

- Funcionalmente correcta: `P = p/100` ✓.
- Booleanamente idéntica a la elegida (ver §6.1), pero conserva un `+1` innecesario.
- **Descartada** en favor de la forma simplificada, que es equivalente pero más barata y más clara.

### 7.2. Usar `rand() % 101`

- Introduce un rango `0..100` (101 valores equiprobables) y rompe la uniformidad sobre 100. Produciría `P = p/101`, que no coincide con la especificación. Descartada.

### 7.3. Reescribir con distribución re-sesgada

- Cualquier reescritura más elaborada añade complejidad sin beneficio. El cambio de una línea es suficiente y óptimo.

## 8. Riesgos y compatibilidad

- **Compatibilidad de juegos**: aumenta la probabilidad de éxito en +1% respecto al comportamiento previo. Juegos que *dependieran involuntariamente* del sesgo podrían mostrar diferencias estadísticas, pero:
  - Ningún juego puede depender legítimamente de un bug de off-by-one indocumentado.
  - El nuevo comportamiento es el que los autores esperan al leer la especificación oficial de DAAD.
- **Riesgo de regresión**: nulo. El cambio es local a una sola línea con semántica claramente definida.
- **Plataformas**: el fix aplica por igual a MSX2 y a cualquier *target* soportado por el repositorio (no depende de arquitectura).

## 9. Plan de verificación

1. **Inspección estática**: confirmar que la línea queda como `checkEntry = (rand()%100) < getValueOrIndirection();` (sin el `+1`), que el comentario del bloque se actualiza para reflejar el nuevo contrato, y que el guard `#ifndef DISABLE_CHANCE` se conserva.
2. **Compilación**: rebuild completo con `make` (Docker SDCC) para asegurar que no hay warnings nuevos.
3. **Test funcional en intérprete** (manual o en un DDB de prueba):
   - Proceso con `CHANCE 100` → debe ejecutar la acción siguiente en el 100% de los turnos (muestreo sugerido ≥ 1000 turnos → 0 fallos).
   - Proceso con `CHANCE 1` → debe ejecutar al menos una vez en una muestra larga (≥ 1000 turnos ⇒ ~10 éxitos esperados; antes del fix: 0).
   - Proceso con `CHANCE 50` → frecuencia observada ≈ 50% (±3σ con n=1000 ⇒ rango aceptable 45–55%).
4. **Comparación cruzada con `do_RANDOM`**: verificar que `RANDOM` sobre un flag y `CHANCE` sobre el mismo umbral producen distribuciones equivalentes.

## 10. Referencias

- Informe: [REPORT_DAAD_BUGS.md § BUG-01](../REPORT_DAAD_BUGS.md)
- Especificación: `.agents/skills/daad-system/references/condacts.md:28`
- Código: [src/daad_condacts.c:541-550](../src/daad_condacts.c#L541-L550)
- Condact hermano (misma fuente aleatoria): [src/daad_condacts.c:1495-1502](../src/daad_condacts.c#L1495-L1502) (`do_RANDOM`)
