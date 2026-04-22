# Fase 3: Gestión de Conexiones y Datos

- Aceptar nuevas conexiones: Registrar los nuevos clientes y sus file descriptors en la estructura de tu función poll().


- Lectura/Escritura segura: Implementar recv y send asegurando que pasen por poll() u otra función equivalente; de lo contrario, tu calificación será 0.


- Agregador de paquetes: Programar un sistema de buffers para acumular datos parciales recibidos hasta reconstruir un comando IRC completo.
