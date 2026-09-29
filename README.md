# Scheduler de procesos: Planificacón Dieciochero
## Introducción y Requisitos de Ejecución
El presente documento detalla la arquitectura, el funcionamiento y las decisiones de diseño del **Scheduler de Procesos**, un simulador desarrollado en C puro. Bajo la temática de coordinar la logística de un **asado dieciochero**, el sistema gestiona la ejecución concurrente de diversas tareas cotidianas (como comprar carne, prender el carbón o armar choripanes) modeladas algorítmicamente mediante un Grafo Acíclico Dirigido (DAG).

Cada actividad representa un proceso independiente en el sistema. El planificador garantiza que ninguna tarea inicie hasta que sus dependencias lógicas previas hayan finalizado con éxito. Durante la simulación, el sistema respeta un límite estricto de concurrencia máxima **K** (que representa la capacidad operativa o cantidad de ayudantes simultáneos en el asado)  y sincroniza toda la comunicación a nivel de kernel utilizando pipes.

### Compilación
Para garantizar la compatibilidad con los estándares exigidos y habilitar las advertencias de seguridad, el sistema debe compilarse estrictamente con el siguiente comando, el cual enlaza la directiva POSIX de hilos por requerimiento de la pauta:

```bash
gcc -Wall -Wextra -std=c17 main.c lector.c -o planificador

```

### Ejecución
El programa requiere la ruta del archivo de texto con la planificación y el límite máximo de actividades concurrentes **K**:

```bash
./planificador plan.txt <k>
```
