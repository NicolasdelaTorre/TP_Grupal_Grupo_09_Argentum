# 🛡️⚔️ Argentum Online - Remake ⚔️🛡️

## Requisitos del sistema

- **Sistema Operativo**: Ubuntu 24.04 / Xubuntu 24.04
- **Compilador**: g++ con soporte C++20
- **CMake**: 3.24 o superior (se instala automáticamente)
- **Dependencias**: SDL2, Qt6, Box2D, GoogleTest (se instalan automáticamente)

## Instalador

El instalador descarga e instala todas las dependencias, por lo cual el usuario solo debe: 

1. **Clonar el repositorio** 

```
git clone <URL_DEL_REPO>
cd TP_Grupal_Grupo_09_Argentum

```

2. **Luego, correr el instalador** 


```sh

sudo make install

```

Una vez completada toda la instalacion, para abrir el servidor se debe de utilizar por consola 

```sh

./build/taller_server <PORT>

#Como ejemplo puede utilizar el 8080

```

Para abrir los clientes, una vez este inicializado el servidor, se debe ejecutar 

```sh

./build/taller_client localhost <PORT>

# El puerto deber ser el mismo que el del servidor.

```

## Documentacion

Se encuentran dentro del proyecto en el apartado documents la documentacion tecnica, el manual de proyecto y el manual de usuario.



## 👥   Integrantes (Grupo 09)

| Nombre | GitHub |
|:--- |:--- |
| **Nicolas de la Torre** | [@NicolasdelaTorre](https://github.com/NicolasdelaTorre) |
| **Oliver Weber** | [@Oli03](https://github.com/Oli03) |
| **Tomás Nahuel Olivera** | [@Tomas-NO](https://github.com/Tomas-NO) |
| **Joaquin Velurtas** | [@joaquinvelurtas](https://github.com/joaquinvelurtas) |