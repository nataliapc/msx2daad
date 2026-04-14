| Gu´ıa unificada | de los sistemas | de creaci´on | de  |
| --------------- | --------------- | ------------ | --- |
aventuras conversacionales
| The Quill, | The PAWS, | SWAN | y   |
| ---------- | --------- | ---- | --- |
DAAD
|     | Jos´e Manuel Ferrer | Ortiz |     |
| --- | ------------------- | ----- | --- |
|     | 19 de mayo de 2016  |       |     |
´
Indice
| 1. Vocabulario |     |     | 4   |
| -------------- | --- | --- | --- |
| 2. Banderas    |     |     | 4   |
Bandera 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
Bandera 1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
Bandera 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
Bandera 3 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
Bandera 4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
Banderas 5 a 8 . . . . . . . . . . . . . . . . . . . . . . . . . . 6
Bandera 9 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
Bandera 10 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
Banderas 11 a 28 . . . . . . . . . . . . . . . . . . . . . . . . . 7
Bandera 29 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
Bandera 30 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
Banderas 31 y 32 . . . . . . . . . . . . . . . . . . . . . . . . . 7
1

Bandera 33 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8
Bandera 34 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8
Bandera 35 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8
Bandera 36 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8
Bandera 37 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8
Bandera 38 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9
Bandera 39 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9
Bandera 40 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9
Bandera 41 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9
Bandera 42 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9
Bandera 43 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
Bandera 44 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
Bandera 45 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
Banderas 46 y 47 . . . . . . . . . . . . . . . . . . . . . . . . . 10
Banderas 48 y 49 . . . . . . . . . . . . . . . . . . . . . . . . . 10
Bandera 50 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
Bandera 51 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
Bandera 52 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
Bandera 53 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
Bandera 54 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
Bandera 55 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
Bandera 56 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
Bandera 57 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
Bandera 58 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
Bandera 59 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
Banderas 60 a 255 . . . . . . . . . . . . . . . . . . . . . . . . 12
3. Mensajes de sistema 12
Mensaje 33 . . . . . . . . . . . . . . . . . . . . . . . . . . . . 13
4. Condiciones 13
2

CHANCE porcentaje . . . . . . . . . . . . . . . . . . . . . . . 14
5. Acciones 14
ANYKEY . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
PROCESS num proceso . . . . . . . . . . . . . . . . . . . . . 14
TIME . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
WINSEL num ventana . . . . . . . . . . . . . . . . . . . . . . 15
3

1. Vocabulario
La longitud ma´xima de las palabras de vocabulario es de 4 letras en
The Quill, y de 5 letras en los dema´s sistemas. Esto supone que cualquier
palabra introducida en una orden desde el teclado, ser´a reducida por
el int´erprete a sus 4 ´o 5 primeras letras, para su comparaci´on con las
palabras del vocabulario de la aventura.
El sistema The PAWS, introdujo la distinci´on de las palabras de
vocabulario por tipo, permitiendo (y obligando) que el programador
indique el tipo de cada palabra que an˜ada al vocabulario, eligiendo uno
de entre los siguientes 7 tipos: verbo, adverbio, nombre (sustantivo),
adjetivo, preposici´on, conjunci´on y pronombre.
Esta caracter´ıstica fue heredada por los sucesores de The PAWS:
SWAN y DAAD. Mientras que The Quill, no hace distinci´on entre las
palabras de vocabulario por su tipo (podr´ıamos decir, que The Quill
so´lo soporta un u´nico tipo de palabras de vocabulario).
2. Banderas
Las banderas son posiciones de memoria en las que se almacena,
en cada una de ellas, un valor num´erico entero entre 0 y 255 (ambos
incluidos). Estos valores se pueden consultar y modificar, y su funcio´n
es dotar de vida a las aventuras, al mantener la mayor parte del estado
de ejecuci´on de las mismas.
The Quill proporciona 33 de estas banderas (numeradas de 0 a 32),
mientras que sus sucesores (The PAWS, SWAN y DAAD) proporcionan
256 banderas (que van desde la 0 hasta la 255).
Sin embargo, no todas las banderas esta´n disponibles para cualquier
propo´sito, ya que algunas de ellas tienen un significado predeterminado,
y nos permitira´n intercambiar con el int´erprete informacio´n importante
para la aventura.
Al inicio de la ejecuci´on de la aventura (y cada vez que esta se
reinicia), el int´erprete pone todas las banderas a su valor inicial. Este
valor es cero para todas las banderas, salvo la bandera 1 (que so´lo sera´
cero si el jugador comienza la aventura con un inventario vac´ıo) y, en los
sistemas posteriores a The Quill, tambi´en algunas otras de las banderas
que tienen una utilidad predefinida; que podra´n tomar valores iniciales
distintos de cero.
4

A continuaci´on, detallaremos el prop´osito que tiene cada bandera,
de qu´e modo se utilizan y c´omo afectar´an a la aventura. Adem´as,
indicaremos su valor inicial, en caso de ser distinto de cero; y los
sistemas (The Quill, The PAWS, SWAN o DAAD) a los que se refieren
las explicaciones.
| Bandera   | 0              |             |     |     |
| --------- | -------------- | ----------- | --- | --- |
| Sistemas: | The Quill, The | PAWS y DAAD |     |     |
Indica al int´erprete si el jugador esta´ en un sitio iluminado o, por el
contrario, est´a en un lugar a oscuras. Un valor de cero significa luz, y
| cualquier valor | distinto | de cero significa | oscuridad. |     |
| --------------- | -------- | ----------------- | ---------- | --- |
En el momento de describir la localidad, si esta bandera tiene un
valor distinto de cero (oscuridad), se decrementar´a la bandera 3 en
una unidad. Si, adema´s, no esta´ presente el objeto 0 (que el int´erprete
considera como una fuente de luz), entonces se decrementar´a tambi´en
en una unidad la bandera 4 y, en lugar de mostrarse la descripci´on de
la localidad, se imprimira´ el mensaje de sistema 0, que avisa al jugador
| de que se encuentra | a   | oscuras. |     |     |
| ------------------- | --- | -------- | --- | --- |
Cada vez que transcurre un turno de juego, el int´erprete comprueba
si esta bandera indica oscuridad. Si es as´ı, entonces decrementar´a en
una unidad la bandera 9; y, si adema´s, el objeto 0 esta´ ausente, entonces
| decrementar´a | tambi´en       | en una unidad | la bandera | 10. |
| ------------- | -------------- | ------------- | ---------- | --- |
| Bandera       | 1              |               |            |     |
| Sistemas:     | The Quill, The | PAWS y DAAD   |            |     |
Contiene el nu´mero de objetos que el jugador lleva en su inventario.
El int´erprete inicializa esta bandera al valor correcto (el nu´mero de
objetos que el jugador lleva al inicio de la aventura), y lo modificar´a
autom´aticamente cada vez que el jugador coja o deje objetos.
| Bandera   | 2              |             |     |     |
| --------- | -------------- | ----------- | --- | --- |
| Sistemas: | The Quill, The | PAWS y DAAD |     |     |
Esta bandera es decrementada en una unidad por el int´erprete cada
| vez que describe | alguna | localidad. |     |     |
| ---------------- | ------ | ---------- | --- | --- |
5

| Bandera   | 3          |          |     |      |
| --------- | ---------- | -------- | --- | ---- |
| Sistemas: | The Quill, | The PAWS | y   | DAAD |
Esta bandera es decrementada en una unidad por el int´erprete cada
vez que describe alguna localidad en un sitio no iluminado (cuando la
| bandera   | 0 est´a a un | valor distinto |     | de cero). |
| --------- | ------------ | -------------- | --- | --------- |
| Bandera   | 4            |                |     |           |
| Sistemas: | The Quill,   | The PAWS       | y   | DAAD      |
Esta bandera es decrementada en una unidad por el int´erprete cada
vez que describe alguna localidad en un sitio no iluminado (cuando la
bandera 0 est´a a un valor distinto de cero), pero s´olo si tampoco est´a
| presente  | el objeto 0 | (fuente  | de luz). |      |
| --------- | ----------- | -------- | -------- | ---- |
| Banderas  | 5 a 8       |          |          |      |
| Sistemas: | The Quill,  | The PAWS | y        | DAAD |
Estas banderas son decrementadas en una unidad por el int´erprete
| cada vez  | que transcurre | un       | turno | de juego. |
| --------- | -------------- | -------- | ----- | --------- |
| Bandera   | 9              |          |       |           |
| Sistemas: | The Quill,     | The PAWS | y     | DAAD      |
Esta bandera es decrementada en una unidad por el int´erprete cada
vez que transcurre un turno de juego, cuando el jugador esta´ en un sitio
no iluminado (cuando la bandera 0 est´a a un valor distinto de cero).
| Bandera   | 10         |          |     |      |
| --------- | ---------- | -------- | --- | ---- |
| Sistemas: | The Quill, | The PAWS | y   | DAAD |
Esta bandera es decrementada en una unidad por el int´erprete cada
vez que transcurre un turno de juego, cuando el jugador esta´ en un sitio
no iluminado (cuando la bandera 0 est´a a un valor distinto de cero) y
| adem´as | el objeto 0 (fuente | de  | luz) | est´a ausente. |
| ------- | ------------------- | --- | ---- | -------------- |
6

| Banderas  | 11         | a 28 |      |     |      |     |
| --------- | ---------- | ---- | ---- | --- | ---- | --- |
| Sistemas: | The Quill, | The  | PAWS | y   | DAAD |     |
Estas banderas no tienen ningu´n propo´sito predefinido y, por tanto,
| pueden   | ser utilizadas |     | libremente | por | el programador. |     |
| -------- | -------------- | --- | ---------- | --- | --------------- | --- |
| Bandera  | 29             |     |            |     |                 |     |
| Sistema: | The Quill      |     |            |     |                 |     |
Esta bandera no tiene ningu´n prop´osito predefinido y, por tanto,
| puede ser | utilizada | libremente |     | por | el programador. |     |
| --------- | --------- | ---------- | --- | --- | --------------- | --- |
| Sistema:  | The PAWS  |            |     |     |                 |     |
El int´erprete comprueba el valor de esta bandera para saber cua´ndo
| debe dibujar | los  | gr´aficos | en  | pantalla. | FIXME: | M´as detalle. |
| ------------ | ---- | --------- | --- | --------- | ------ | ------------- |
| Sistema:     | DAAD |           |     |           |        |               |
El int´erprete inicializa esta bandera a 128 si est´a ejecut´andose con
un modo gra´fico de 320 p´ıxeles de ancho, con lo cual se dispondra´ de 53
columnas para gra´ficos (de 6 p´ıxeles de anchura) o texto. Si, en cambio,
el int´erprete se ejecuta en modo texto;entonces inicializara´estabandera
a 0, indicando con ello que se dispondr´a de 80 columnas para texto.
| Bandera   | 30         |      |          |     |             |              |
| --------- | ---------- | ---- | -------- | --- | ----------- | ------------ |
| Sistemas: | The Quill, | The  | PAWS     | y   | DAAD        |              |
| En esta   | bandera    | se   | almacena | la  | puntuaci´on | del jugador. |
| Banderas  | 31         | y 32 |          |     |             |              |
| Sistemas: | The Quill, | The  | PAWS     | y   | DAAD        |              |
Estas dos banderas contienen el nu´mero de turnos que han transcu-
| rrido desde | el inicio | del | juego. |     |     |     |
| ----------- | --------- | --- | ------ | --- | --- | --- |
La bandera 31 contiene la parte menos significativa (LSB) y la 32
contiene la ma´s significativa (MSB). Es decir, que cuando hayan pasado
255 turnos, la bandera 31 tendra´ un valor de 255 y la 32 un valor de 0;
y cuando se pase a 256, la bandera´ 31 guardara´ un 0 y la 32 un 1. Dicho
7

de otra forma au´n, el nu´mero total de turnos sera´ 256 multiplicado por
| el valor  | de la bandera | 32,    | m´as el valor   | de la | bandera 31.     |
| --------- | ------------- | ------ | --------------- | ----- | --------------- |
| Bandera   | 33            |        |                 |       |                 |
| Sistemas: | The PAWS      | y DAAD |                 |       |                 |
| Contiene  | el primer     | verbo  | de la sentencia |       | l´ogica actual. |
Si se modifica desde la tabla de respuestas (o desde cualquier sub-
proceso llamado desde esta), las entradas siguientes tendra´n que encajar
| con esta  | nueva sentencia |        | l´ogica reci´en | modificada. |                 |
| --------- | --------------- | ------ | --------------- | ----------- | --------------- |
| Bandera   | 34              |        |                 |             |                 |
| Sistemas: | The PAWS        | y DAAD |                 |             |                 |
| Contiene  | el primer       | nombre | de la           | sentencia   | l´ogica actual. |
Si se modifica desde la tabla de respuestas (o desde cualquier sub-
proceso llamado desde esta), las entradas siguientes tendra´n que encajar
| con esta  | nueva sentencia |        | l´ogica reci´en | modificada. |     |
| --------- | --------------- | ------ | --------------- | ----------- | --- |
| Bandera   | 35              |        |                 |             |     |
| Sistemas: | The PAWS        | y DAAD |                 |             |     |
Contiene el adjetivo del primer nombre de la sentencia lo´gica actual.
| Bandera   | 36          |        |                 |         |         |
| --------- | ----------- | ------ | --------------- | ------- | ------- |
| Sistemas: | The PAWS    | y DAAD |                 |         |         |
| Contiene  | el adverbio |        | de la sentencia | l´ogica | actual. |
| Bandera   | 37          |        |                 |         |         |
| Sistemas: | The PAWS    | y DAAD |                 |         |         |
Almacena el nu´mero m´aximo de objetos que el jugador es capaz
de llevar (en su inventario) a la vez. Su valor inicial es 4, y puede ser
| modificado | con el condacto |     | ABILITY. |     |     |
| ---------- | --------------- | --- | -------- | --- | --- |
8

| Bandera   | 38       |        |     |     |     |     |
| --------- | -------- | ------ | --- | --- | --- | --- |
| Sistemas: | The PAWS | y DAAD |     |     |     |     |
En esta bandera se mantiene el nu´mero de la localidad actual del
jugador.
| Bandera   | 39       |        |     |     |     |     |
| --------- | -------- | ------ | --- | --- | --- | --- |
| Sistemas: | The PAWS | y DAAD |     |     |     |     |
En esta bandera, se indica el nu´mero de la primera l´ınea en la que
se podra´ escribir texto. Se puede modificar a trav´es del condacto LINE.
TODO: Comprobar qu´e ocurre usando LINE junto con el condacto
| MODE, | ver la p´ag. | 34 de | la Gu´ıa T´ecnica. |     |     |     |
| ----- | ------------ | ----- | ------------------ | --- | --- | --- |
DAAD le asigna un valor inicial de 13. Y au´n as´ı... ¡imprime desde
| la primera | l´ınea! FIXME |        |     |     |     |     |
| ---------- | ------------- | ------ | --- | --- | --- | --- |
| Bandera    | 40            |        |     |     |     |     |
| Sistemas:  | The PAWS      | y DAAD |     |     |     |     |
AlgorelacionadoconMODE...¿elmododepantalla?FIXMEAfecta
| a la parte | en que      | el int´erprete | describe   | la localidad. |                |     |
| ---------- | ----------- | -------------- | ---------- | ------------- | -------------- | --- |
| Bandera    | 41          |                |            |               |                |     |
| Sistemas:  | The PAWS    | y DAAD         |            |               |                |     |
| Algo       | relacionado | con            | PROTECT... | FIXME ¿Valor  | inicial en     | The |
| PAWS !=    | 0? FIXME    |                |            |               |                |     |
| Bandera    | 42          |                |            |               |                |     |
| Sistemas:  | The PAWS    | y DAAD         |            |               |                |     |
| Esta       | bandera     | guarda         | el nu´mero | del mensaje   | de sistema que | se  |
imprimir´a para alentar o guiar al jugador a que introduzca una nueva
| orden. Se | puede cambiar |     | con el condacto | PROMPT. |     |     |
| --------- | ------------- | --- | --------------- | ------- | --- | --- |
El valor 0 (que es el valor por defecto) hace que el int´erprete elija
un mensaje aleatoriamente entre los mensajes de sistema 2, 3, 4 y 5;
con una probabilidad de 3/10, 3/10, 3/10 y 1/10, respectivamente.
9

| Bandera   | 43              |        |           |              |                 |
| --------- | --------------- | ------ | --------- | ------------ | --------------- |
| Sistemas: | The PAWS        | y DAAD |           |              |                 |
| Contiene  | la preposici´on |        | de la     | sentencia    | l´ogica actual. |
| Bandera   | 44              |        |           |              |                 |
| Sistemas: | The PAWS        | y DAAD |           |              |                 |
| Contiene  | el segundo      |        | nombre de | la sentencia | l´ogica actual. |
| Bandera   | 45              |        |           |              |                 |
| Sistemas: | The PAWS        | y DAAD |           |              |                 |
Contiene el adjetivo del segundo nombre de la sentencia l´ogica
actual.
FIXME: ¿el adjetivo del segundo nombre, o simplemente el segundo
| adjetivo  | (el que se | escriba | en seguna | posici´on)? |     |
| --------- | ---------- | ------- | --------- | ----------- | --- |
| Banderas  | 46 y       | 47      |           |             |     |
| Sistemas: | The PAWS   | y DAAD  |           |             |     |
Algo de los pronombres: FIXME. El int´erprete los inicializa con un
valor de 255, lo cual simboliza que no se refiere a ningu´n pronombre.
| Banderas  | 48 y     | 49     |     |     |     |
| --------- | -------- | ------ | --- | --- | --- |
| Sistemas: | The PAWS | y DAAD |     |     |     |
Algo relacionado con el tiempo muerto (condacto TIMEOUT): FIX-
ME
| Bandera   | 50          |        |             |        |       |
| --------- | ----------- | ------ | ----------- | ------ | ----- |
| Sistemas: | The PAWS    | y DAAD |             |        |       |
| Algo      | relacionado | con    | el condacto | DOALL: | FIXME |
10

| Bandera   | 51              |     |     |
| --------- | --------------- | --- | --- |
| Sistemas: | The PAWS y DAAD |     |     |
El int´erprete guarda en esta bandera el co´digo del u´ltimo objeto al
que se ha referido... ¿? ...y poder ser imprimido en lugar de en los
| mensajes. | FIXME           |     |     |
| --------- | --------------- | --- | --- |
| Bandera   | 52              |     |     |
| Sistemas: | The PAWS y DAAD |     |     |
Almacena el peso m´aximo que el jugador es capaz de cargar en su
inventario (contando solamente el peso los objetos que lleve ah´ı, no los
que lleve puestos) a la vez. Su valor inicial es 10, y puede ser modificado
| con el condacto | ABILITY.        |     |     |
| --------------- | --------------- | --- | --- |
| Bandera         | 53              |     |     |
| Sistemas:       | The PAWS y DAAD |     |     |
Con esta bandera, se le indica al int´erprete el modo en que deseamos
que liste los objetos que esta´n en el inventario del jugador, ¿cuando use-
mos el condacto INVEN?. FIXME: Ma´s detalle. Afecta a los condactos
| LISTOBJ   | y LISTAT.       |     |     |
| --------- | --------------- | --- | --- |
| Bandera   | 54              |     |     |
| Sistemas: | The PAWS y DAAD |     |     |
El valor de esta bandera es el c´odigo de la localidad en que se
encuentra el objeto actualmente referido (el objeto guardado en la
| bandera   | 51).            |     |     |
| --------- | --------------- | --- | --- |
| Bandera   | 55              |     |     |
| Sistemas: | The PAWS y DAAD |     |     |
Esta bandera proporciona el peso que tiene el objeto actualmente
| referido | (el objeto guardado | en la bandera | 51). |
| -------- | ------------------- | ------------- | ---- |
11

| Bandera   | 56       |        |     |     |     |
| --------- | -------- | ------ | --- | --- | --- |
| Sistemas: | The PAWS | y DAAD |     |     |     |
Algo relativo a si el objeto referido es un contenedor... FIXME
| Bandera   | 57            |           |          |                  |       |
| --------- | ------------- | --------- | -------- | ---------------- | ----- |
| Sistemas: | The PAWS      | y DAAD    |          |                  |       |
| Algo      | relativo a si | el objeto | referido | es una prenda... | FIXME |
| Bandera   | 58            |           |          |                  |       |
| Sistema:  | The PAWS      |           |          |                  |       |
[3]
| Bandera  | 59       |     |     |     |     |
| -------- | -------- | --- | --- | --- | --- |
| Sistema: | The PAWS |     |     |     |     |
La Gu´ıa T´ecnica de The PAWS [2] recomienda no utilizar esta
bandera, puesto que iban a utilizarse para ampliaciones futuras.
| Banderas  | 60 a 255 |        |     |     |     |
| --------- | -------- | ------ | --- | --- | --- |
| Sistemas: | The PAWS | y DAAD |     |     |     |
Estas banderas no tienen ningu´n propo´sito predefinido y, por tanto,
| pueden      | ser utilizadas | libremente | por el | programador. |     |
| ----------- | -------------- | ---------- | ------ | ------------ | --- |
| 3. Mensajes |                | de sistema |        |              |     |
Los mensajes de sistema son mensajes de informaci´on que se le
ofrecen al jugador, en distintos contextos. A diferencia de los mensajes
de usuario, algunos de estos mensajes son mostrados automa´ticamente
por el int´erprete, aunque las aventuras de The PAWS y DAAD pueden
mostrarlos tambi´en en cualquier momento, haciendo uso de la acci´on
| SYSMESS | (no existente | en The | Quill). |     |     |
| ------- | ------------- | ------ | ------- | --- | --- |
12

The Quill tiene 32 mensajes de sistema, todos ellos con funcionali-
dades determinadas de antemano. Pero The PAWS permite hasta 256
mensajes de sistema, de los cuales menos de una cuarta parte tiene
finalidades predeterminadas, pudiendo los dem´as utilizarse libremente.
Esto, junto con los mensajes de usuario, deja al programador de aventu-
ras The PAWS con ma´s de 440 mensajes de propo´sito general, mientras
que The Quill s´olo proporciona los 256 mensajes de usuario.
Con lo que el autor de este documento sabe de momento de DAAD,
este sistema proporciona al menos el mismo nu´mero total de mensajes
de prop´osito general (todos los de usuario y los libres de sistema).
Aunque, Javier San Jos´e en un mensaje [5] de la lista de correo de
CAAD, hablaba del soporte de ¡hasta 256 tablas de mensajes! (supongo,
contando la de sistema):
(cid:40)(cid:40)Adem´as se ampliaba el nu´mero de tablas de mensajes.
Recuerdo que en PAWS s´olo hab´ıa dos tablas de mensajes
(la de usuario y la del sistema) con un m´aximo de 256
mensajes cada una. En DAAD se pod´ıa crear hasta 256
tablas de mensajes con 256 mensajes cada una.(cid:41)(cid:41)
Mensaje 33
Sistemas: The PAWS y DAAD
A pesar de no verse afectado por el uso del condacto PROMPT,´este
es el verdadero prompt en el sentido original del t´ermino (consu´ltese
http://es.wikipedia.org/wiki/Prompt). Como tal, ser´a mostrado
por el int´erprete siempre que quede a la espera de recibir cualquier l´ınea
de entrada por parte del jugador, a trav´es del teclado.
4. Condiciones
Haydostiposcondactos:condicionesyacciones.Dehecho,lapalabra
condacto (en ingl´es, condact) proviene de la unio´n de esas dos palabras
en ingl´es: condition (condici´on) y action (acci´on).
Las condiciones de cada sistema y sus finalidades son:
13

| CHANCE    | porcentaje |          |        |     |
| --------- | ---------- | -------- | ------ | --- |
| Sistemas: | The Quill, | The PAWS | y DAAD |     |
Siendo porcentaje un valor entre 1 y 99, ambos incluidos, esta
condici´on se cumple si este nu´mero es mayor o igual1 que un nu´mero
| aleatorio | entre 1 y | 100, tambi´en | inclusive. |     |
| --------- | --------- | ------------- | ---------- | --- |
Es decir, que una condicio´n CHANCE 25 sera´ satisfactoria un 25% de
las veces, y saltar´a en las dem´as ocasiones (el 75% de veces restante).
5. Acciones
| Las | acciones de | cada sistema | y sus finalidades | son: |
| --- | ----------- | ------------ | ----------------- | ---- |
ANYKEY
Al menos en DAAD, este condacto no tiene en cuenta pulsaciones
de teclas modificadoras, al menos no tiene en cuenta: Ctrl, Shift, Alt,
Teclas de Windows de abajo (comprobado bajo DOSBox, habr´ıa que
| ver desde | Windows). |         |     |     |
| --------- | --------- | ------- | --- | --- |
| PROCESS   | num       | proceso |     |     |
| Sistemas: | The PAWS  | y DAAD  |     |     |
Comprobado con DAAD, desde la tabla de proceso 1 no se permite
llamar ni al proceso 0 ni al 2, ni a un nu´mero de proceso inexistente
| (en todos | estos casos | da error | 6). |     |
| --------- | ----------- | -------- | --- | --- |
TIME
| Sistema: | The PAWS |     |     |     |
| -------- | -------- | --- | --- | --- |
Enlaversio´nSpectrumdeThePAWS,laduracio´nesenintervalosde
1,28 segundos [2], mientras que en las dema´s versiones, es en intervalos
| de 1 segundo | [4]. |     |     |     |
| ------------ | ---- | --- | --- | --- |
1
Los manuales de The Quill y The PAWS dicen aqu´ı “menor o igual”, pero se
| trata de | una errata que | ha pasado | desapercibida. |     |
| -------- | -------------- | --------- | -------------- | --- |
14

| WINSEL   |     | num ventana |     |     |     |     |     |     |
| -------- | --- | ----------- | --- | --- | --- | --- | --- | --- |
| Sistema: |     | DAAD        |     |     |     |     |     |     |
Selecciona la subventana de impresio´n de nu´mero num ventana para
trabajar sobre ella. Cualquier tipo de impresio´n tanto de texto como de
gr´aficos, se realiza siempre sobre la subventana seleccionada.
DAAD proporciona ocho subventanas de impresi´on, numeradas de
la 0 a la 7, siendo la nu´mero 1 la seleccionada inicialmente. Para cada
| una | de estas | subventanas, | guarda | la siguiente |     | informaci´on: |     |     |
| --- | -------- | ------------ | ------ | ------------ | --- | ------------- | --- | --- |
Coordenadas de origen Por defecto: [0, 0]. Se cambian a trav´es del
|     | condacto | WINPOS. |     |     |     |     |     |     |
| --- | -------- | ------- | --- | --- | --- | --- | --- | --- |
Taman˜o de la subventana Por defecto: [25, 53] en modo gr´afico y
[25, 80] en modo texto (v´ease la informaci´on de la bandera 29).
|     | Se cambia | a trav´es | del condacto |     | WINSIZE. |     |     |     |
| --- | --------- | --------- | ------------ | --- | -------- | --- | --- | --- |
Coordenadas del cursor Por defecto: [0, 0]. Se cambian a trav´es del
|     | condacto      | PRINTAT.      |               |     |               |     |           |          |
| --- | ------------- | ------------- | ------------- | --- | ------------- | --- | --------- | -------- |
|     | Los condactos | PRINTAT,      | WINPOS        |     | y WINSIZE     |     | modifican | solamen- |
| te  | la subventana | seleccionada. |               |     |               |     |           |          |
|     | FIXME:        | ¿Se puede     | redimensionar |     | la subventana |     | 0?        |          |
Referencias
[1] Graeme Yeandle, “The Quill: An Adventure System for the 48K
|     | Spectrum”. | Gilsoft | International | Ltd., | Serial | C,  | 1983. |     |
| --- | ---------- | ------- | ------------- | ----- | ------ | --- | ----- | --- |
ftp://ftp.worldofspectrum.org/pub/sinclair/games-info/
q/QuillAdventureSystemThe.txt
[2] “The Quill Adventure Writing System for the BBC B & Electron”.
|     | Gilsoft | International | Ltd., 1984. |     |     |     |     |     |
| --- | ------- | ------------- | ----------- | --- | --- | --- | --- | --- |
http://www.nvg.ntnu.no/bbc/sw/Various/Quill.doc
[3] Tim J. Gilberts, “A Technical guide to: The Professional Adventure
|     | Writer.  | A graphic   | adventure | writing       |     | system | for the | Sinclair |
| --- | -------- | ----------- | --------- | ------------- | --- | ------ | ------- | -------- |
|     | Spectrum | computers”. | Gilsoft   | International |     | Ltd.,  | 1986.   |          |
ftp://ftp.worldofspectrum.org/pub/sinclair/games-info/
p/ProfessionalAdventureWriter_TechnicalGuide.zip
15

| [4] “THE   | PROFESSIONAL | ADVENTURE | WRITING          | SYSTEM. | A   |
| ---------- | ------------ | --------- | ---------------- | ------- | --- |
| supplement | for the      | Spectrum  | – Version A 16”. |         |     |
ftp://ftp.worldofspectrum.org/pub/sinclair/games-info/
p/ProfessionalAdventureWriter_A16Supplement.txt
[5] Tim J. Gilberts & Graeme Yeandle, “PC Adventure Writer – Tech-
| nical | Guide”. |     |     |     |     |
| ----- | ------- | --- | --- | --- | --- |
http://www.yeandle.plus.com/advent/pawtech.html
[6] Javier San Jos´e, “RE: el DAAD”. Lista de correo del Club de
| Aventuras | AD, 29 | de noviembre | de 2002. |     |     |
| --------- | ------ | ------------ | -------- | --- | --- |
http://www.caad.es/listas/caad/msg12353.html
16
