# 🛡️⚔️ Argentum Online — Remake · Manual del Usuario ⚔️🛡️

Bienvenido/a a **Argentum Online Remake**, un MMORPG 2D cliente-servidor desarrollado
en C++. Con las explicaciones paso a paso vas a poder **instalar, compilar, configurar y jugar** sin conocimientos previos de programación.

El proyecto incluye tres programas:

| Programa | Para qué sirve |
|----------|----------------|
| **Servidor** (`taller_server`) | Hostea la partida. Tiene que estar prendido para poder jugar. |
| **Cliente** (`taller_client`) | El juego en sí. Cada jugador abre un cliente y se conecta al servidor. |
| **Editor** (`taller_editor`) | Herramienta gráfica para crear y editar mapas (niveles). |

---

## Índice

1. [Requisitos del sistema](#1-requisitos-del-sistema)
2. [Instalación (la forma fácil)](#2-instalación-la-forma-fácil)
4. [Configuración: mapas, recursos y archivos](#4-configuración-mapas-recursos-y-archivos)
5. [Cómo levantar el servidor](#5-cómo-levantar-el-servidor)
6. [Cómo lanzar el cliente](#6-cómo-lanzar-el-cliente)
7. [Cómo se juega](#7-cómo-se-juega)
8. [Comandos de chat](#8-comandos-de-chat)
9. [Cómo crear y editar mapas con el Editor](#9-cómo-crear-y-editar-mapas-con-el-editor)

---

## 1. Requisitos del sistema

| Requisito | Detalle |
|-----------|---------|
| **Sistema operativo** | Ubuntu 24.04 / Xubuntu 24.04 (cualquier distro basada en Debian/Ubuntu con `apt`). |
| **Compilador** | `g++` con soporte de **C++20** (incluido en `build-essential`). |
| **CMake** | Versión **3.24 o superior** (el instalador la actualiza automáticamente si hace falta). |
| **Dependencias gráficas/audio** | SDL2, SDL2_image, SDL2_mixer, SDL2_ttf (se compilan solas desde fuente). |
| **Editor de mapas** | Qt 6 (`qt6-base-dev`). |
| **Otras librerías** | yaml-cpp, Box2D, GoogleTest, librerías de audio (opus, fluidsynth, etc.). |

> 💡 **No te preocupes por instalar todo esto a mano.** El instalador
> (`install.sh` / `make install`) se encarga de descargar e instalar todas las
> dependencias por vos. La tabla está acá solo a modo informativo.

---

## 2. Instalación (la forma fácil)

Esta es la forma **recomendada**. Sólo necesitás dos pasos.

### Paso 1 — Clonar el repositorio

Abrí una terminal y ejecutá:

```sh
git clone <URL_DEL_REPO>
cd TP_Grupal_Grupo_09_Argentum
```

### Paso 2 — Correr el instalador

```sh
sudo ./install.sh
```

Esto va a:

1. Actualizar el índice de paquetes (`apt-get update`).
2. Instalar **todas** las dependencias del sistema.
3. Verificar la versión de CMake y, si es vieja, actualizarla desde el repositorio oficial de Kitware.
4. **Compilar el proyecto** automáticamente.

Cuando termine vas a ver el mensaje:

```
>>> Compilacion finalizada. Binarios en ./build/
```

A partir de ahí ya tenés los tres ejecutables listos en la carpeta `build/`:

```
build/taller_server
build/taller_client
build/taller_editor
```


## 3. Configuración: mapas, recursos y archivos

La buena noticia: **no hay que editar ningún archivo de configuración a mano para
jugar.** Los recursos (imágenes, sonidos, fuentes) ya vienen en el repositorio y los
programas los encuentran solos, porque al arrancar se posicionan automáticamente en
la raíz del proyecto.

Lo que conviene saber:

- **Imágenes y sprites**: están en `AO_IMGS/` y `common/assets/images/`. No hace
  falta moverlas a ningún lado.
- **Sonidos**: el cliente carga su audio desde la carpeta `client/Audio/`.
- **Mapas del servidor**: están en `server/assets/maps/` como archivos `.yaml`
  (`argentumland.yaml`, `completo.yaml`, etc.).
- **Mapa que carga el servidor**: por defecto el servidor levanta
  `server/assets/maps/argentumland.yaml`. Si querés que cargue otro mapa, podés
  renombrar tu mapa a `argentumland.yaml` (hacé una copia de seguridad del original
  primero), o usar el [Editor](#9-cómo-crear-y-editar-mapas-con-el-editor) para
  guardar tu mapa con ese nombre.

---

## 4. Cómo levantar el servidor

El servidor tiene que estar **prendido antes** de que se conecten los clientes.
Desde la raíz del proyecto, ejecutá:

```sh
./build/taller_server <PUERTO>
```

Por ejemplo, usando el puerto `8080`:

```sh
./build/taller_server 8080
```

- `<PUERTO>` es el número de puerto donde el servidor va a escuchar conexiones.
- La terminal queda "ocupada" mostrando los mensajes del servidor. **Dejala
  abierta** mientras jugás.
- Para apagar el servidor, en la consola del servidor con el comando 'q' se cierra.

> ⚠️ Anotá el puerto que elegiste: lo vas a necesitar para conectar cada cliente.

---

## 5. Cómo lanzar el cliente

Con el servidor ya corriendo, abrí **otra terminal** (o pedile a cada jugador que
abra la suya) y ejecutá:

```sh
./build/taller_client <HOST> <PUERTO>
```

- `<HOST>`: la dirección del servidor.
  - Si el servidor corre en **tu misma computadora**, usá `localhost`.
  - Si corre en **otra máquina de la red**, usá su dirección IP (por ejemplo `192.168.0.15`).
- `<PUERTO>`: **el mismo puerto** que pusiste al levantar el servidor.

Ejemplo (servidor y cliente en la misma máquina, puerto 8080):

```sh
./build/taller_client localhost 8080
```

### Modo pantalla completa (opcional)

Agregá `--fullscreen` al final para abrir el juego en pantalla completa:

```sh
./build/taller_client localhost 8080 --fullscreen
```

Podés abrir **varios clientes a la vez** (cada uno en su terminal) para jugar entre
varias personas contra el mismo servidor.

---

## 6. Cómo se juega

### 6.1 Pantalla de inicio de sesión

Al abrir el cliente vas a ver la pantalla de login. Escribí el **nombre de tu
personaje** y presioná `Enter`.

![Pantalla de login](./Screens/Login.png)

### 6.2 Elegí tu raza

A continuación elegís la **raza** de tu personaje. Usá las flechas o el mouse para
moverte entre las opciones y `Enter` para confirmar.

Razas disponibles: **Humano**, **Elfo**, **Enano**, **Gnomo**.

![Pre-selección de personaje](./Screens/Raza_y_clase.png)

### 6.3 Elegí tu clase

Después elegís la **clase**, que define tu estilo de juego (más fuerza física o más
poder mágico).

Clases disponibles: **Mago**, **Clérigo**, **Campeón**, **Guerrero**.


### 6.4 Elegí la cabeza (aspecto)

Por último elegís el aspecto (cabeza) de tu personaje y entrás al mundo.

![Selección de personaje](./Screens/Seleccion_cabeza.png)

### 6.5 La pantalla de juego (HUD)

Una vez dentro, en el panel lateral derecho vas a ver la información de tu personaje:

- 🟥 **Barra de vida (HP)** — si llega a cero, morís.
- 🟦 **Barra de maná (MP)** — se consume al usar magia.
- 🟡 **Oro** — tu dinero para comprar y vender.
- ⭐ **Experiencia y nivel** — subís de nivel acumulando experiencia.
- 🎒 **Inventario** — los objetos que tenés encima.

![Inventario y estado del personaje](./Screens/Inventario.png)

### 6.6 Controles

| Acción | Control |
|--------|---------|
| **Moverse** | Teclas `W` `A` `S` `D` **o** las flechas ⬆️⬇️⬅️➡️ |
| **Atacar a un enemigo o jugador** | **Clic izquierdo** sobre el enemigo/jugador objetivo |
| **Hablar con un NPC amigo** (comerciante, banquero, sacerdote) | **Clic izquierdo** sobre el NPC para seleccionarlo |
| **Equipar / desequipar un objeto** | **Doble clic izquierdo** sobre el objeto en el inventario |
| **Abrir el chat / enviar mensaje** | `Enter` |
| **Cancelar el chat** | `Escape` |
| **Borrar texto en el chat** | `Backspace` |

> 💬 El **chat** sirve tanto para hablar con otros jugadores como para escribir
> **comandos** (ver la sección siguiente). Mientras el chat está abierto, las teclas
> de movimiento no mueven al personaje.

### 6.7 Combatir, comerciar y banco

- **Atacar**: hacé clic sobre un enemigo. El servidor valida si tenés el arma
  adecuada, si estás a distancia (ataque a distancia) o pegado (cuerpo a cuerpo).
- **Comerciar**: acercate a un **comerciante**, hacé clic para seleccionarlo y usá
  los comandos `/listar`, `/comprar` y `/vender`.
- **Sacerdote**: te cura (`/curar`) o te resucita (`/resucitar`) si moriste.
- **Banquero**: te guarda oro y objetos con `/depositar` y `/retirar`.
- **Recoger objetos del suelo**: parate sobre el objeto y usá `/tomar`.

Si morís, soltás tu equipamiento en el suelo y te convertís en fantasma; buscá un
**sacerdote** y usá `/resucitar`.

---

## 7. Comandos de chat

Abrí el chat con `Enter`, escribí el comando y presioná `Enter` de nuevo para
enviarlo.

### Objetos e inventario

| Comando | Qué hace |
|---------|----------|
| `/tomar` | Levanta el objeto que esté en tu celda. |
| `/tirar <slot>` | Tira al suelo el objeto del slot indicado del inventario. |
| `/equipar <slot>` | Equipa el objeto del slot indicado. |
| `/desequipar <arma\|armor\|casco\|escudo>` | Desequipa la pieza indicada. |
| `/meditar` | Recupera maná meditando (clases mágicas). |

### Comerciante / Sacerdote (requiere tenerlo seleccionado y estar cerca)

| Comando | Qué hace |
|---------|----------|
| `/listar` | Muestra el inventario del comerciante o del banco. |
| `/comprar <objeto>` | Compra un objeto al comerciante o sacerdote. |
| `/vender <objeto>` | Vende un objeto al comerciante. |
| `/curar` | El sacerdote te cura. |
| `/resucitar` | El sacerdote te revive si moriste. |

### Banquero (requiere tenerlo seleccionado y estar cerca)

| Comando | Qué hace |
|---------|----------|
| `/depositar <objeto>` | Deposita un objeto en el banco. |
| `/depositar oro <cant>` | Deposita oro en el banco. |
| `/retirar <objeto>` | Retira un objeto del banco. |
| `/retirar oro <cant>` | Retira oro del banco. |

### Clanes

| Comando | Qué hace |
|---------|----------|
| `/fundar-clan <nombre>` | Funda un clan nuevo. |
| `/unirse <nombre del clan>` | Pide unirte a un clan. |
| `/revisar-clan` | Revisa las solicitudes pendientes (fundador). |
| `/clan-aceptar <nick>` | Acepta a un solicitante. |
| `/clan-rechazar <nick>` | Rechaza a un solicitante. |
| `/clan-kick <nick>` | Expulsa a un miembro. |
| `/clan-ban <nick>` | Banea a un miembro. |
| `/dejar-clan` | Salís del clan. |

### Comandos de prueba (cheats)

Útiles para probar el juego rápidamente:

| Comando | Qué hace |
|---------|----------|
| `/vidainf` | Activa/desactiva vida infinita. |
| `/manainf` | Activa/desactiva maná infinito. |
| `/levelup` | Subís un nivel. |
| `/gold <cantidad>` | Te da oro. |
| `/item <itemId>` | Te da el objeto con ese id. |
| `/suicidio` | Tu personaje muere (para probar la mecánica de muerte). |

---

## 8. Cómo crear y editar mapas con el Editor

El editor de mapas te permite armar los mapas del juego: el **mapa principal** con biomas, ciudades, obstáculos y entradas a dungeons, y los **environments** internos de cada entrada. Al guardar, el mapa se exporta como YAML en `server/assets/maps/` para que el servidor lo cargue.

## Pantalla principal

Al abrir el editor aparecen dos opciones:

- **New map**: crea un mapa en blanco.
- **Open map**: abre un `.yaml` existente.

## Crear un mapa nuevo

1. Elegí **New map**.
2. Completá los campos:
   - **Id**: identificador del archivo. Es el nombre con el que se va a guardar.
   - **Name**: nombre visible del mapa.
   - **Size**: ancho y alto en celdas (entre 1 y 250).
3. Presioná **Create**.

Se abre la pantalla de edición con una grilla vacía.

## Abrir un mapa existente

1. Elegí **Open map**.
2. Seleccioná un archivo `.yaml`.

El editor carga el mapa principal y todos sus environments para seguir editando.

## Pantalla de edición

La interfaz se divide en tres zonas:

| Zona | Para qué sirve |
|---|---|
| Barra superior | Cambiar de herramienta, volver al menú o al mapa principal |
| Panel lateral Izquierdo | Elegir template y ver datos del elemento activo |
| Grilla del mapa | Ver y editar el mapa |

Debajo del mapa hay tres controles:

- **Save map**: guarda el mapa.
- **+** / **–**: acercar y alejar el zoom.

**Exit** vuelve al menú principal (sin guardar).
**Back to map** aparece solo cuando estás dentro de un environment y
te devuelve al mapa principal.

## Herramientas disponibles

En la barra superior, cada botón activa un modo de edición, cambiando el panel lateral según cambia el modo elegido.

### 👤 Player Spawn

Coloca el punto donde aparece el jugador al entrar al mapa principal.

1. Activá **Player Spawn**.
2. Hacé click en una celda libre del mapa.

Solo puede haber un spawn a la vez: si colocás otro, reemplaza al anterior. No se puede poner sobre obstáculos.

### 🪨 Obstacles

Coloca obstáculos con textura (árboles, casas, rocas, etc.).

1. Activá **Obstacles**.
2. Elegí un template en la lista del panel.
3. Hacé click en el mapa para colocarlo.

También podés usar obstáculos dentro de un environment (mismo botón, misma mecánica).

### 🌿 Biomes

Define zonas con una textura de piso y criaturas que spawnean ahí.

1. Activá **Biomes**.
2. Elegí un bioma en la lista.
3. **Arrastrá** con el botón izquierdo para dibujar un rectángulo.
4. Al soltar, se abre un diálogo para configurar la población de cada criatura (sliders de 0 a 50). Las que queden en 0 no spawnean.
5. Las celdas fuera del rectángulo pero cerca reciben la textura del bioma más cercano automáticamente.

**Tips:**

- Pasar el mouse sobre un bioma existente muestra sus criaturas en el panel lateral.
- **Click** sobre un bioma ya colocado (sin arrastrar) abre de nuevo el diálogo de criaturas para editarlo.
- **Click derecho** mientras dibujás cancela el rectángulo en curso.

### 🏙️ Cities

Coloca una ciudad (zona segura) con tamaño fijo según el template.

1. Activá **Cities**.
2. Elegí una ciudad en la lista.
3. Hacé click en el mapa.

La ciudad se coloca con sus NPCs, obstáculos y pisos predefinidos del template. Ocupa el área indicada por el template, y si no entra en el mapa, no se coloca.

### 🟫 Floors

Modifica el piso de una celda individual sin bloquear el paso.

1. Activá **Floors**.
2. Elegí un tile en la lista.
3. Hacé click en la celda deseada.

Si ya había un piso en esa celda, se reemplaza. Podes usarlo para armar caminos.

### 🚪 Environments

Permite crear entradas al overworld que llevan a mazmorras o cuevas, y ver la lista de environments del mapa.

1. Activá **Environments**.
2. Elegí un template de **Entry** en el desplegable.
3. Hacé click en el mapa donde querés la entrada.

Se abren dos diálogos en secuencia:

1. **New environment**: tipo (cueva/mazmorra, según la entrada) y nombre del environment.
2. **Environment creatures**: población de criaturas dentro del environment (mismos sliders que en biomas).

Al confirmar, se crea la entrada visible en el mapa y un environment interno de 30×30 celdas asociado. El environment aparece en la lista **Environments** del panel.
Para editar el interior, hacé **doble click** sobre él en esa lista.

### 📐 Dimensions

Cambia el tamaño del mapa principal (o del environment, si estás editando uno).

1. Activá **Dimensions**.
2. Elegí **Expand** o **Reduce**.
3. Elegí la dirección.
4. Indicá cuántas **Cells** agregar o quitar.
5. Presioná **Apply**.

Al **reducir**, el editor no permite achicar si algún elemento quedaría fuera del nuevo tamaño. El tamaño actual se muestra en **Map Size**.

## Editar un environment

Al entrar a un environment (doble click en la lista), la barra superior cambia, aparecen nuevos botones y se van otros. Dentro podés colocar:

- **Player Spawn**: dónde aparece el jugador al entrar.
- **Obstacles**: obstáculos internos.
- **Walls**: paredes que delimitan el recinto.
- **Exits**: salidas hacia el overworld.

El piso del environment es fijo y se define por su tipo (cueva/mazmorra).

### 🧱 Paredes y salidas

1. Activá **Walls** (el botón que antes decía Environments).
2. Elegí un template de pared y hacé click para colocar.
3. Activá **Exits**, elegí un template y colocá la salida dentro del recinto.


### 🧟‍♂️ Criaturas del environment

Con el botón **Environment creatures** podés reabrir el diálogo de población y cambiar qué criaturas habitan el environment.

### Volver al mapa principal

Presioná **Back to map**. Los cambios del environment se incorporan al
documento antes de volver.

## Eliminar elementos

**Click derecho** sobre un elemento del mapa lo borra.

Para ciudades, biomas y entradas, el editor pide confirmación antes de borrar, y si eliminás una entrada, también se elimina el environment asociado de la lista.

Al borrar una ciudad, se eliminan también los obstáculos y pisos que estaban dentro de su área, incluso los que hayas colocado por tu cuenta.

## Guardar el mapa

1. Presioná **Save map** , abajo del mapa principal.
2. ESi todo está bien, guarda en `server/assets/maps/<id>.yaml`, donde `<id>` es el id que pusiste al principio.

### Requisitos para poder guardar

El editor no guarda si falta algo de esta lista:

| Requisito | Detalle |
|---|---|
| Id y tamaño del mapa | El mapa debe tener id y dimensiones válidas |
| Spawn en el overworld | Debe haber exactamente un **Player Spawn** en el mapa principal |
| Entradas válidas | Cada entrada debe apuntar a un environment existente |
| Tamaño de cada environment | Ancho y alto mayores a cero |
| Spawn en cada environment | Cada environment debe tener su **Player Spawn** |
| Paredes que encierran | Si un environment tiene paredes, deben formar un recinto con interior; el spawn debe quedar adentro |

Si algo falla, aparece un mensaje indicando qué corregir.

---
