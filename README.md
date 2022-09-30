# Copia de timberman hecho con SDL2 (y sin texturas)

Una copia mal hecha y simplificada del juego Timberman:

<https://store.steampowered.com/app/398710/Timberman/>



Pero funciona en Windows, Web, Raspberry Pi 1, Android y Linux (deberia).


## Jugar

### WEB

Se puede probar aqui:

<https://jorgesc231.github.io/timberman/>


### Windows

Descargar desde la seccion de Releases:

    windows_x64_timberman.zip


Descomprimirlo y ejecutar **Timberman.exe**.


### Android

Descargar desde la seccion de Releases:

    timberman.apk

Pasarlo al telefono de alguna forma e instalarlo. 

**Tienes que tener activado: Origenes desconocidos.**

Seguramente aparezca una alerta de seguridad, ignoralas, la aplicacion no contiene nada raro.


### Raspberry Pi 1

Es bastante dificil instalar...

Para que funcione con aceleracion por hardware hay que activarlo desde el sistema operativo, instalar otros drivers de video y
recompilar SDL2 para que los utilice.

    [Insertar video jugando en la RPI 1]


### Linux

**Por ahora no hay un binario para Linux, mejor seguir la instrucciones para compilarlo**


## Compilar

### Windows

1. Clonar el proyecto.
2. Abrir la solucion con Visual Studio (No el Code...).
3. Compilar y ejecutar


### Web

1. Instalar:

<https://emscripten.org/>

2. Desde una consola de simbolo de sistema de Windows ejecutar:

    emsdk_env.bat

en la carpeta del emsdk.

3. Ir a la carpeta del proyecto y ejecutar desde simbolo de sistema:

    build_web.bat

4. Para ejecutarlo, en el mismo simbolo del sistema:

    emrun web_build/timberman.html

### Linux

**Solo lo he probado en Ubuntu.**

Clonar el repositorio:

    git clone https://github.com/jorgesc231/timberman

Cambiar al directorio del codigo:

    cd timberman

Instalar las dependencias necesarias:

    sudo apt install build-essential libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev

Crear el directorio de salida:

    mkdir build

Darle permisos de ejecucion al build script:

    chmod 0700 ./build_linux.sh

Ejecutar el build script:

    ./build_linux.sh

Esto deberia compilar el juego, para ejecutarlo:

    build/timberman


### Android

TODO


### Raspberry Pi 1

TODO



## TODO

- Agregar Texturas
- Agregar mas efectos de audio
- Varios otros bugs
- Instrucciones para compilar