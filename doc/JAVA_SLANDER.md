# Java slander

Por qué el proyecto base de Nanofiles es malo: Un analisis

## Elección del lenguaje

Java era un lenguaje ampliamente usado para escribir software nuevo... hace 15
años. Ahora se considera como mala opción, como deuda tecnica (por razones fuera
del alcance de este documento) no se escribe nuevo software en el, solo
se mantiene el antiguo porque es mas barato que reescribirlo.

Especificamente, Java es una opción terrible para este caso porque es un
obstaculo para el objetivo del proyecto: aprender a escribir aplicaciones con
sockets.

Java, siendo OOP puro, está sujeto a limitaciones que le impiden representar fielmente
realmente como funcionan las cosas. La API tiene una serie de clases para representar
datagramas UDP por ejemplo: los datagramas existen, pero la clase DatagramPacket
es algo muy abstracto que en ningún momento representa el paquete real, sino juntar
datos en una unidad porque sí. Java no sabe lo que es un datagrama realmente.

Que queremos enviar un cacho de datos a varias direcciones: tenemos que construir
un DatagramPacket para cada vez, con los mismos datos, pero con una dirección de
destino distinta. Java está lleno de estas construcciones, copias copias y copias
y destrucciones completamente innecesarias.

Cada día nos alejamos mas de C: La API de sockets de Java está demasiado alejada
de la verdad: la API de sockets POSIX. Esa que todos los sitemas operativos reales
implementan de una forma u otra desde BSD 4.2 (incluso Windows), que es lo que la
JVM realmente usa por debajo para hablar con la red.

Esta API POSIX, tenemos unos datos, y los enviamos a una direccion, en una sola
llamada, y sin ningún tipo de construcción o copia de nada. Es el kernel el que
coje el búfer de memoria, construye el paquete UDP, lo encapsula en un frame Ethernet
y se lo pasa al driver de la tarjeta de red (generalmente).

- Java no esta diseñado para trabajar con datos binarios facilmente, teniendo
  que serializar y deserializar todo, y ni si quiera soporta tipos enteros sin signo
  explicitamente.

- Aparentemente no hay forma de coger un trozo de un archivo y mandarlo por un socket
  sin copiar el bufer, lenguaje inutilizable, como se permite esto en 2025?

- Java es muy inflexible en cuanto a operaciones no bloqueantes. POSIX tiene la syscall
  poll() para monitorear una lista de sockets en un solo thread, y Java tiene algo parecido
  pero crucialmente, en C puedes responder con llamadas bloqueantes sabiendo que no se van
  a bloquear, mientras que en Java solo puedes usar cosas no bloqueantes sobre un canal
  con select, lo cual es pedantico y molesto.
  

## Problemas con el código

### General

- Legibilidad horrible
- Nombres confusos, ambiguos
- Inconsistencias de patrones
- Innecesariamente complicado
- Verbosidad (mas culpa de Java supongo)
- Exceso de comentarios, y además confusos
- Capa sobre capa sobre capa de abstracciones: paso de datos entre clases que
  no hacen nada innecesariamente
- Enumeraciones, arrays, estructuras innecesarias, como los comandos.

### Especifico

- Nos dan el proyecto para Eclipse, usando el build system de eclipse. Con lo
  cual nos obligan a usar Eclipse. Los IDEs son un obstaculo para el aprendizaje.
  Tienen una cantidad de ruido visual increibles, son enormemente complicados de usar
  y suelen tener bugs y molestias.
  La alternativa es "convertirlo a gradle" y usar gradle que es otro build system
  terrible y innecesariamente complejo.

- Ejemplo de TODO ambiguo: En cual se crea el hilo?
    - NFControllerP2P: "Arrancar servidor en segundo plano creando un nuevo hilo"
    - NFServer: "Añadir métodos a esta clase para: 1) Arrancar el servidor en
      un hilo nuevo que se ejecutará en segundo plano"
    La verdadera respuesta es que hay que fijarse que NFServer implementa Runnable,
    con lo cual es esta clase la que hay que pasar a un Thread, y run() el metodo que
    ejecuta el Thread, siendo este declarado efectivamente en NFControllerP2P.

- Ejemplo de TODO confuso e incongruente:
    - NFServer implements Runnable: "Añadir métodos a esta clase para: [...] 2) Detener el servidor (stopserver)"
    Como vas a parar el servidor dentro del mismo thread, si el thread está bloqueado en accept()?
    Si no se usan sockets no bloqueantes (no los hemos visto) la unica forma de detener
    el thread es terminandolo a la fuerza desde el thread Main.

- Ejemplo de TODO directamente incorrecto:
    - NFServer.serveFilesToClient: "Los métodos lookupHashSubstring y lookupFilenameSubstring de la clase FileInfo"
    lookupHashSubstring no existe en ningún sitio.
    Además, estas utilidades deberían estar en la base de datos FileDatabase, literalmente son queries a la base de datos.
    Sin embargo, lookupFilePath sí que está en la FileDatabase, por qué? Es como si se hubieran puesto los metodos
    de forma aparentemente aleatoria por el codigo.
    Nota: esta función tampoco no sirve un proposito porque en el servidor se necesita el resto de campos
    del objeto FileInfo, no solo su path. Debería de haber un solo metodo para buscar en la db por hash o por nombre y devolver
    el FileInfo, solo que por alguna razón divinamente desconocida, a FileInfo le falta el getter del path.
    (Nota: solo tiene sentido guardar el path entero en FileInfo, ya que el nombre es parte del path y por tanto redundante)

- FileDatabase es AdHoc, solo sirve como base de datos del cliente:
    - escanea un directorio
    - no puede guardar peers para usarse en NFDirectoryServer

- DirMessage intenta representar todos los tipos de mensajes, haciendo la 
  implementación terrible: encima de que estamos en un lenguaje OOP esto 
  deberia ser resuelto por herencia para cada tipo de mensaje

- NFDirectoryServer termina inmediatamente al recibir un mensaje mal formado,
  esto es una vulnerabilidad de Denial of Service

- NanoFiles (cliente) termina inmediatamente en cuanto un comando falla.
  Cuando haces un ping y el directorio no responde, termina, en vez de dejarte
  volver a intentar

- NFController.processCommand()[COM\_DOWNLOAD]: A dos metodos aislados se le pasa la misma
  información inutil (una substring del nombre del archivo). Sin modificar estos,
  los dos metodos necesitan el hash del archivo -> tendrian que hacer la misma request al directorio
  para ver a que hash se corresponde (información util) para poder preguntar por
  A. los servidores y B. a los peers, los chunks del archivo. Diseño fundamentalmente defectuoso.
  Lo primero que se debería hacer es obtener la información util, el hash, que es con lo que
  se debe trabajar.

