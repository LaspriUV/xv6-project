# XV6 Mejorado - Proyecto de SO

Este repositorio contiene un fork del sistema operativo educativo xv6 con algunas mejoras experimentales en planificación de CPU y gestión de memoria.

## Mejoras Implementadas
### 1️⃣ Planificación de CPU

Round-Robin original de xv6 preservado.

Aging simple: los procesos en estado RUNNABLE incrementan su prioridad si esperan mucho tiempo.

Penalización por quantum: los procesos que consumen todo su quantum reciben un aumento en su valor de prioridad (prioridad más baja).

Control de quantum: el tick de CPU se cuenta por proceso y se fuerza yield() cuando alcanza el quantum máximo.

Esto permite una planificación más justa y evita starving de procesos de baja prioridad.

### 2️⃣ Gestión de Memoria

Copy-on-Write (COW) básico:

Durante fork(), las páginas se marcan como COW y solo se copian al escribir.

Reduce el uso de memoria al compartir páginas entre padre e hijo hasta que sean modificadas.

Refcount de páginas: control seguro de referencias de memoria para COW.

Optimización de TLB: se actualizan entradas cuando se cambia la memoria de usuario.

Stack guard page: previene desbordamientos de stack de usuario.

Inicialización de memoria: todas las páginas nuevas se limpian a cero al asignarse (kalloc()).

Liberación correcta de memoria: garantiza que al finalizar un proceso se liberen todas las páginas asignadas.
