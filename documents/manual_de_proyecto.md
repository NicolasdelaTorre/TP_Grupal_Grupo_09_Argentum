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

- **Nicolás de la Torre**: En mi caso me hice cargo de la renderizacion en pantalla de todo lo relacionado al juego. Una vez la informacion enviada por el server es decodificada se imprime en pantalla.

- **Oliver Weber**: Me encargue del apartado logico del juego. Es decir, de las reglas del mundo, de lo que podia hacer una entidad o no, recompenas y castigos a jugadores y la interacción con el mundo. 

- **Joaquín Velurtas**: Me enfoque principalmente en todo lo relacionado con el editor de mapas, desde su funcionamiento hasta las herramientas para construir los escenarios del juego. Tambien colabore con parte de la lectura de esos mapas desde el servidor, para integrarlos correctamente al mundo jugable. Ademas, prepare las texturas de los distintos obstaculos y tiles usando Photoshop.

- **Tomás Olivera**: Me hice cargo del protocolo de comunicación entre cliente y servidor: el formato binario que viaja por la red, las clases comunes que lo serializan y la definición de cada uno de los mensajes que cliente y servidor intercambian.

---

## Organización por semana

### Semana 1 — 11 al 17 de mayo

- **Nicolás**: Subi el template y se dividieron distintas branches de trabajo
- **Oliver**: Creación de las clases principales de la logica (Game, Player y Map) e inicio del juego a nivel logico.
- **Joaquín**: Sin commits, arrancando la version base del editor, investigando un poco mas sobre la herramienta e implementando la logica de las pantallas, elementos de edicion, vistas y el grid del mapa.
- **Tomás**: No tuve commits propios esta semana. Estuve leyendo el
  enunciado y pensando con el resto cómo iba a ser la comunicación
  entre cliente y servidor.

### Semana 2 — 18 al 24 de mayo

- **Nicolás**: Creo la base de protocolo y primeras conexiones.
- **Oliver**: Inicio de sesión con los datos proporcionados del cliente.
- **Joaquín**: Creado y logica de lectura de templates YAML para ciudades y biomas, verificator para asegurar que todos los elementos esten dentro del mapa y guardado del mapa completo en formato YAML.
- **Tomás**: Me metí a fondo con el protocolo. Definimos en grupo el
  formato general y yo lo implementé junto con los primeros mensajes
  de login para probar que cerraba. No era nada complejo pero ya teníamos una conexión entre cliente y servidor. Solo restaba expandir para las nuevas features.

### Semana 3 — 25 al 31 de mayo

- **Nicolás**: Primer funcionamiento real del juego pudiendose ver las primeras texturas y personajes con sus respectivas armas y animaciones. Tambien se implementa el login y el fullscreen. Como asi tambien las colisiones con objetos.
- **Oliver**: Envio de datos del jugador y del mapa a todos los clientes. Vida agregada y enviada al cliente con su correspondiente jugador.
- **Joaquín**: Renderizado de obstaculos y tiles con texturas reales, actualizacion del layout de los elementos en pantalla, mostrar criaturas configuradas para cada bioma colocado y permitir editarlas, logica de entradas a environments y los propios environments con obstaculos y paredes, algoritmo de Dijkstra para definir las texturas de las celdas que no estan directamente dentro de un bioma creado.
- **Tomás**: Me ocupé de la integración real, que el servidor parsee
  los mapas que iba armando Joaco con el editor, los mande al cliente
  y distribuya los movimientos entre los jugadores. Lo más difícil de
  la semana no fue programar sino pensar el flujo, quién manda qué y
  en qué momento, para que no se duplicaran mensajes ni quedaran huecos.

### Semana 4 — 1 al 7 de junio

- **Nicolás**: Implemento el feature de convertirse en fantasma una vez muerto y las diferentes skins. Tambien se renderizan los objetos en el suelo como asi la sangre cuando un personaje es atacado.
- **Oliver**: Persistenca, mecanica del ataque, curación, npcs y su comportamiento y dungeons añadidos.
- **Joaquín**: Preparar mas texturas, lectura de los biomas en el YAML desde el servidor y actualizacion en la lectura de los environments, nueva herramienta para poner tiles con texturas especificas en el mapa, pop-up en la creacion de environments para definir las criaturas del mismo, salidas de los environments y flood-fill para definir el exterior de los environments creados, permitir la modificacion de las dimensiones del mapa y los entornos.
- **Tomás**: Sumé el mensaje de stats del jugador para el HUD y armé
  con Oliver la mecánica de ataque (yo el mensaje del wire, él la lógica
  de daño y evasión del servidor). Lo más importante de la semana fue
  un refactor grande del protocolo: cada mensaje pasó a ser su
  propia clase con su propia regla de cómo se serializa, lo que simplificó
  mucho el entendimiento del código.

### Semana 5 — 8 al 14 de junio

- **Nicolás**: Implemente el HUD del inventario y la visualizacion del chat. Utilizo los mismos items en inventario que para renderizar en el suelo. Se establecen skins default. Se agregan las stats en la HUD de inventario. Tambien se implementan los hechizos con los baculos. Agrego pantalla de seleccion de raza y clase.
- **Oliver**: Meditación, resurrección, curación de salud y mana durante el tiempo, consumo de pociones y algunas soluciones de errores o de mal comportamiento de npcs.
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

- **Nicolás**: fix de las texturas de los NPC, paths y agrego audio al juego.
- **Oliver**: Solución a muchos errores y mejoras en la calidad del vida del juego.
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
## Trabajo Futuro y Limitaciones Conocidas (Faltantes)

### 1. Arquitectura y Deuda Técnica (Lógica del Servidor)
- **Desacoplamiento de `Game` y `GameLoop`:** Actualmente ambas actúan como *God Classes* (clases dios), concentrando la mayor parte de las operaciones de la lógica del juego. Se requiere refactorizar y delegar responsabilidades en sistemas más pequeños y cohesivos para reducir el acoplamiento.
- **Implementación de Identificadores Únicos (UUID):** El sistema actual utiliza el nombre del jugador como clave primaria para evitar el doble inicio de sesión y gestionar la persistencia. Lo óptimo sería implementar un sistema de IDs únicos e inmutables que persistan a lo largo del tiempo.
- **Optimización de la persistencia bancaria:** Actualmente se utiliza un único archivo para las cuentas bancarias. Si este archivo debe cargarse íntegramente para operar, resulta ineficiente a nivel de consumo de memoria RAM.
- **Encapsulamiento del `yaml_map_loader`:** Se requiere refactorizar el módulo de carga de mapas para convertirlo en una clase independiente, reduciendo así dependencias globales.
- **Tipado estricto en coordenadas:** Dado que no existen coordenadas negativas en el mapa, se debe migrar la estructura de posición `(x, y)` para utilizar enteros sin signo (`uint16_t` en lugar de enteros con signo), optimizando el uso de memoria y previniendo errores lógicos.

### 2. Red y Sincronización (Protocolo y Cliente-Servidor)
- **Interpolación de movimiento de entidades remotas:** Mientras que el jugador local percibe su movimiento de forma fluida dentro de un mismo *tile*, las entidades remotas (otros jugadores y NPCs) actualizan su posición saltando directamente del centro de un *tile* a otro.
- **Parametrización de comandos de depuración (Trucos):** Permitir al servidor activar o desactivar el uso de trucos.
- **Consistencia del idioma:** Actualmente existe una mezcla de idiomas (Spanglish) en los comandos de usuario y la lógica interna (ej. `/comprar axe`). Se debe unificar el idioma.

### 3. Mecánicas y Jugabilidad
- **Sistema de Línea de Visión (Line of Sight):** Implementar detección de colisiones estáticas para los ataques a distancia. Si existe un obstáculo sólido entre una entidad atacante y su objetivo, el proyectil debería impactar contra el entorno, anulando el daño.
- **Manejo de estados límite en comandos:** Deshabilitar los trucos para los jugadores muertos (Fantasma).
- **Profundidad del bestiario:** Aumentar la variedad de atributos y comportamientos de las criaturas. Actualmente, la única variable que diferencia a dos criaturas del mismo tipo es su nivel.
- **Escalado dinámico de daño:** Implementar una mecánica que incremente el rango de daño base de las armas ofensivas en proporción directa al nivel del jugador que las porta.
- **Refinamiento del balance general:** Realizar ajustes iterativos sobre las fórmulas de daño, evasión, experiencia, etc para mejorar la curva de dificultad.

### 4. Interfaz Gráfica y Herramientas (Cliente y Editor)
- *(Espacio reservado para agregar faltantes de UI/UX, renderizado gráfico en SDL, o funcionalidades del Editor de Mapas)*
