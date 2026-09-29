#### Scheduler de procesos: Planificacón Dieciochero
### Introducción y Requisitos de Ejecución
El presente documento detalla la arquitectura, el funcionamiento y las decisiones de diseño del
Planificador de Procesos, un simulador desarrollado en C puro que gestiona la ejecución
concurrente de actividades modeladas mediante un Grafo Acíclico Dirigido (DAG). El sistema
respeta un límite máximo de concurrencia ($k$) y sincroniza los procesos a nivel de kernel
mediante pipes).
