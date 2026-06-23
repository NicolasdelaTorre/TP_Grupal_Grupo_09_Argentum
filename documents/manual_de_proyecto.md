# Argentum Online — Manual de Proyecto

**Taller de Programación I — FIUBA — 1er Cuatrimestre 2026 — Grupo 09**

Nicolás de la Torre, Oliver Weber, Joaquín Velurtas, Tomás Nahuel Olivera.

---

## División de trabajo

Cuando arrancamos el TP nos sentamos a charlar cómo íbamos a repartir
el trabajo. La idea fue que cada uno se hiciera cargo de una parte lo
más independiente posible del resto, para que no tuviéramos que
estar esperando que el otro termine ni pisando los cambios que teniamos que hacer. Tuvimos
suerte de que la división en módulos del proyecto calzaba bien con cuatro personas:

- **Nicolás de la Torre**: 

- **Oliver Weber**: 

- **Joaquín Velurtas**: Me enfoque principalmente en todo lo relacionado con el editor de mapas, desde su funcionamiento hasta las herramientas para construir los escenarios del juego. Tambien colabore con parte de la lectura de esos mapas desde el servidor, para integrarlos correctamente al mundo jugable. Ademas, prepare las texturas de los distintos obstaculos y tiles usando Photoshop.

- **Tomás Olivera**: Me hice cargo del protocolo de comunicación entre cliente y servidor: el formato binario que viaja por la red, las clases comunes que lo serializan y la definición de cada uno de los mensajes que cliente y servidor intercambian.

---

## Organización por semana

### Semana 1 — 11 al 17 de mayo

- **Nicolás**: 
- **Oliver**: 
- **Joaquín**: Sin commits, arrancando la version base del editor, investigando un poco mas sobre la herramienta e implementando la logica de las pantallas, elementos de edicion, vistas y el grid del mapa.
- **Tomás**: No tuve commits propios esta semana. Estuve leyendo el
  enunciado y pensando con el resto cómo iba a ser la comunicación
  entre cliente y servidor.

### Semana 2 — 18 al 24 de mayo

- **Nicolás**: 
- **Oliver**: 
- **Joaquín**: Creado y logica de lectura de templates YAML para ciudades y biomas, verificator para asegurar que todos los elementos esten dentro del mapa y guardado del mapa completo en formato YAML.
- **Tomás**: Me metí a fondo con el protocolo. Definimos en grupo el
  formato general y yo lo implementé junto con los primeros mensajes
  de login para probar que cerraba. No era nada complejo pero ya teníamos una conexión entre cliente y servidor. Solo restaba expandir para las nuevas features.

### Semana 3 — 25 al 31 de mayo

- **Nicolás**: 
- **Oliver**: 
- **Joaquín**: Renderizado de obstaculos y tiles con texturas reales, actualizacion del layout de los elementos en pantalla, mostrar criaturas configuradas para cada bioma colocado y permitir editarlas, logica de entradas a environments y los propios environments con obstaculos y paredes, algoritmo de Dijkstra para definir las texturas de las celdas que no estan directamente dentro de un bioma creado.
- **Tomás**: Me ocupé de la integración real, que el servidor parsee
  los mapas que iba armando Joaco con el editor, los mande al cliente
  y distribuya los movimientos entre los jugadores. Lo más difícil de
  la semana no fue programar sino pensar el flujo, quién manda qué y
  en qué momento, para que no se duplicaran mensajes ni quedaran huecos.

### Semana 4 — 1 al 7 de junio

- **Nicolás**: 
- **Oliver**: 
- **Joaquín**: Preparar mas texturas, lectura de los biomas en el YAML desde el servidor y actualizacion en la lectura de los environments, nueva herramienta para poner tiles con texturas especificas en el mapa, pop-up en la creacion de environments para definir las criaturas del mismo, salidas de los environments y flood-fill para definir el exterior de los environments creados, permitir la modificacion de las dimensiones del mapa y los entornos.
- **Tomás**: Sumé el mensaje de stats del jugador para el HUD y armé
  con Oliver la mecánica de ataque (yo el mensaje del wire, él la lógica
  de daño y evasión del servidor). Lo más importante de la semana fue
  un refactor grande del protocolo: cada mensaje pasó a ser su
  propia clase con su propia regla de cómo se serializa, lo que simplificó
  mucho el entendimiento del código.

### Semana 5 — 8 al 14 de junio

- **Nicolás**: 
- **Oliver**: 
- **Joaquín**: Actualizacion de logica en el servidor para mover el cliente del overworld a los environments. Hacer que el cliente use directamente las texturas enviadas por el servidor y renderice los elementos ordenados en el eje Y para evitar solapamientos incorrectos.
- **Tomás**: Me dediqué casi toda la semana al chat libre con sus
  comandos (los del mercader, banquero, sanador, los privados con
  @nick, los cheats). En paralelo trabajé con Oli para cerrar el
  combate real: defensa por armadura, notificaciones al chat, fair
  play, drop al morir y zonas seguras. Lo que más me costó fue la
  predicción de movimiento del cliente, que tenía que sentirse
  inmediata pero conciliar con la palabra final del servidor sin que
  se viera como un tirón. Cerré la semana armando la primera suite
  de unit tests del protocolo con GoogleTest.

### Semana 6 — 15 al 21 de junio

- **Nicolás**: 
- **Oliver**: 
- **Joaquín**: Muchas mas texturas y templates, cambio en la logica de renderizacion de imagenes (usar un anchor distinto a la esquina inferior izquierda si se necesita), actualizaciones visuales del hud del editor.
- **Tomás**: Implementé el sistema de clanes completo con todos
  los comandos, las notificaciones, el bonus de cercanía y la
  persistencia. Después le di una mano a Oliver con unos
  bugs del servidor y armé el hechizo de curación con la
  flauta élfica. Cerré con un refactor de la configuración del 
  toml (teníamos cuatro archivos  sueltos pasaron a uno solo) 
  y la documentación.

## Herramientas usadas
## Problematicas encontradas
## Faltantes

