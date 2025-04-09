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

## Problemas con el código

- Legibilidad horrible
- Innecesariamente complicado
- Verbosidad
- Exceso de comentarios
- Capa sobre capa sobre capa de abstracciones: paso de datos innecesario
- FileDatabase es Adhoc, solo sirve como base de datos del cliente:
    - escanea un directorio
    - no puede guardar peers para usarse en NFDirectoryServer
- DirMessage intenta representar todos los tipos de mensajes, haciendo la 
  implementación terrible: encima de que estamos en un lenguaje OOP esto 
  deberia ser resuelto por herencia para cada tipo de mensaje

