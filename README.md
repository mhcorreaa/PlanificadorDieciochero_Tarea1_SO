# Scheduler de procesos: Planificacón Dieciochero

## 1. Introducción y Requisitos de Ejecución
El presente documento detalla la arquitectura, el funcionamiento y las decisiones de diseño del **Scheduler de Procesos**, un simulador desarrollado en C puro. Bajo la temática de coordinar la logística de un **asado dieciochero**, el sistema gestiona la ejecución concurrente de diversas tareas cotidianas (como comprar carne, prender el carbón o armar choripanes) modeladas algorítmicamente mediante un Grafo Acíclico Dirigido (DAG).

Cada actividad representa un proceso independiente en el sistema. El scheduler garantiza que ninguna tarea inicie hasta que sus dependencias lógicas previas hayan finalizado con éxito. Durante la simulación, el sistema respeta un límite estricto de concurrencia máxima **K** (que representa la capacidad operativa o cantidad de ayudantes simultáneos en el asado)  y sincroniza toda la comunicación a nivel de kernel utilizando pipes.

### 1.1 Compilación
Para garantizar la compatibilidad con los estándares exigidos y habilitar las advertencias de seguridad, el sistema debe compilarse estrictamente con el siguiente comando, el cual enlaza la directiva POSIX de hilos por requerimiento de la pauta:

```bash
gcc -Wall -Wextra -std=c17 main.c lector.c -o planificador

```

### 1.2 Ejecución
El programa requiere la ruta del archivo de texto con la planificación y el límite máximo de actividades concurrentes **K**:

```bash
./planificador plan.txt <k>
```
## 2. Dependencias

### 2.1 Macros de configuración

- **_DEFAULT_SOURCE y _POSIX_C_SOURCE 200809L**: Al compilar bajo el estándar estricto -std=c17, el compilador oculta funciones exclusivas de POSIX. Estas macros se inyectan en la primera línea de los archivos para habilitar explícitamente llamadas al sistema operativo como kill(), usleep(), setrlimit() y strsep().

### 2.2 Librerías

- **<stdio.h>, <stdlib.h>, <string.h>**: Gestión de entrada/salida, asignación dinámica de memoria en el heap (malloc, strdup) y parseo de cadenas de texto.

- **<unistd.h>**: Interfaz principal de llamadas al sistema POSIX para la creación de procesos (fork), comunicación (pipe, read, write) y suspensión de ejecución (usleep).

- **<sys/types.h>, <sys/wait.h>**: Definición de tipos de datos de procesos (pid_t) y mecanismos de sincronización para que el proceso padre recolecte a los procesos hijos terminados (wait).

- **<signal.h>**: Manejo asíncrono de interrupciones de hardware y software (ej. interceptar SIGINT).

- **<time.h>**: Proveedor de la semilla de tiempo real (time(NULL)) para la entropía del generador pseudoaleatorio.

- **<sys/resource.h>**: Manipulación de las cuotas físicas de recursos asignadas por el kernel al proceso.

## 3. Módulos

### 3.1 Lector.h
Este archivo define la interfaz de datos compartida entre los módulos del sistema y las constantes de estado. El núcleo lógico de este encabezado es la definición del struct Actividad, una estructura de datos que encapsula y representa cada tarea individual dentro del grafo de ejecución.   

#### Constantes de Estado
Establece las macros de control READY (2), RUNNING (1), WAITING (0) y FAILED (-1) para estandarizar el ciclo de vida de los procesos a lo largo de la simulación.   

#### Atributos de struct Actividad

- **ID_Actividad**: Puntero a cadena de caracteres (char *) que almacena el identificador alfanumérico único de la tarea.
- **Nombre_Actividad**: Puntero a cadena (char *) con el nombre descriptivo de la acción a ejecutar.
- **tiempo_ms**: Entero (int) que define la duración o latencia de la actividad en milisegundos.
- **depend**: Arreglo dinámico de punteros (char **) que almacena los IDs exactos de las tareas previas de las que depende esta actividad.
- **num_depend**: Entero (int) que indica la cantidad total de dependencias que la actividad debe esperar obligatoriamente antes de iniciar.
- **estado**: Entero (int) que registra en tiempo real la situación de la tarea utilizando las constantes de estado definidas.
- **PID**: Variable de tipo pid_t que guarda el identificador de proceso real asignado por el kernel tras realizar el fork().
- **fd[2]**: Arreglo de dos enteros (int) que almacena los descriptores de archivo para la tubería (pipe) anónima y exclusiva de esta actividad, habilitando la sincronización I/O bloqueante.
- **cantidad_dependientes**: Entero (int) precalculado que almacena el número total de actividades futuras que dependen directamente de esta tarea, dato estricto para saber cuántos mensajes inyectar en la tubería al finalizar.

#### Firma de función procesar_archivo()
Expone la firma pública int procesar_archivo(char *nombre_archivo, struct Actividad *actividadesTotales, int *nroActividades);. Esta declaración permite que el módulo principal invoque la rutina de parseo, parametrizando por referencia el nombre del archivo de entrada, el puntero al arreglo global de estructuras para poblar los datos, y el puntero al contador de memoria que mantendrá el registro total de actividades.

### 3.2 Lector.c
Módulo encargado de leer el archivo **plan.txt** y extraer cada parámetro de las actividades de forma separada, con el objetivo de ingresar dicha actividad como un **struct** en **Lector.h**.

#### procesar_archivo()
Lee el documento línea por línea utilizando un buffer. Emplea strsep para aislar los tokens (ID, nombre, tiempo, dependencias) basándose en el delimitador "**:**".

##### Propiedades

- **Limpieza de datos**: Se implementa una validación para eliminar espacios residuales antes de almacenar el ID, asegurando consistencia en futuras comparaciones de memoria.

- **Parametrización temporal**: Si una actividad no define su duración, el sistema utiliza rand() para asignarle automáticamente un tiempo pseudoaleatorio entre 100 ms y 5000 ms.

- **Estructuración de memoria**: Emplea strdup() para duplicar las cadenas de caracteres dinámicamente en el heap, evitando la pérdida de referencias al sobrescribir el buffer.

- **Mapeo del Grafo (Dependientes)**: Un bucle independiente, ejecutado estrictamente después de cargar todas las tareas, calcula la variable cantidad_dependientes de cada actividad cruzando las dependencias. Necesario para la implementacion de pipes.

### 3.3 Main
Constituye el núcleo operativo del scheduler, orquestando el multiprocesamiento, la sincronización y la recolección de estados.

#### cayoLaSeremi()
Función manejadora de señales diseñada para interceptar la interrupción SIGINT (Ctrl+C). Al ser invocada asíncronamente, recorre el arreglo global de actividades y ejecuta la llamada al sistema kill(PID, SIGKILL) exclusivamente sobre las tareas cuyo estado sea RUNNING. Finalmente, fuerza la salida limpia del programa, previniendo la proliferación de procesos huérfanos.

#### revisarDependencias()
Función de validación de estado. Analiza el arreglo de dependencias de una actividad objetivo. Retorna 1 únicamente si todas sus dependencias tienen estado **READY**. Si detecta que alguna dependencia posee el estado **FAILED**, propaga el error cancelando la actividad actual.

#### Main()
Gestiona el flujo de la ejecución.

##### Propiedades
  
- **Ciclo de Eventos**: Emplea un bucle while que mantiene vivo al scheduler hasta que la suma de actividades listas iguale al total de tareas. Un bucle for anidado recorre la estructura disparando procesos hasta alcanzar el límite **K**.
  
- **Creación y pipe**: Al validar una actividad, se inicializa su tubería (pipe) y se clona el proceso mediante fork().
  
- **Lógica del Proceso Hijo**: El hijo entra en estado de suspensión bloqueante llamando a read() sobre los descriptores de lectura **(fd[0])** de cada una de sus dependencias. El kernel lo despertará solo cuando reciba los mensajes. Tras ejecutar su tarea **(usleep)**, utiliza un ciclo para efectuar múltiples **write()** en su propio descriptor de escritura **(fd[1])**, depositando un mensaje de liberación ("Listo Brother") por cada tarea que dependa de él.

- **Lógica del Proceso Padre**: Utiliza wait(&status) para atrapar al primer hijo que finalice. Mediante las macros **WIFEXITED** y **WEXITSTATUS**, evalúa si el proceso concluyó exitosamente o abortó, actualizando los contadores READY o FAILED para liberar un cupo de concurrencia y reevaluar el grafo en la siguiente iteración.

## 4. Decisiones de Diseño

### Escritura Precalculada en Pipes
Dado que un pipe actúa como una cola FIFO (First In, First Out), un mensaje depositado desaparece al ser leído. Si una actividad tiene 5 dependientes, escribir un solo mensaje causaría que el primer dependiente lo consuma y los otros 4 queden bloqueados permanentemente (Deadlock). El cálculo anticipado de **cantidad_dependientes** en el módulo lector resuelve este problema instruyendo al proceso hijo a inyectar en su tubería la cantidad métricamente exacta de mensajes necesarios.

### Escalabilidad de Descriptores (setrlimit)
Para garantizar la tolerancia a pruebas de estrés con hasta 10000 actividades, se identificó un cuello de botella físico: el límite por defecto de Linux de 1024 archivos abiertos por proceso. Como cada **pipe()** consume dos descriptores, el límite se alcanzaría al instanciar 500 tareas. Se implementó una negociación preventiva utilizando **setrlimit(RLIMIT_NOFILE)** directamente en el código para exigir una cuota de 20000 descriptores en tiempo de ejecución. Si el entorno operativo rechaza la solicitud por políticas de permisos, la función realiza un fallback silencioso, permitiendo la ejecución de simulaciones estándar sin abortar el programa por tecnicismos de administrador.

### Persistencia de Memoria en Estructuras Dinámicas (strdup)
La lectura del archivo de texto (lector.c) se realiza mediante un único arreglo local (char buffer[1024]) que se sobrescribe en cada iteración del ciclo while. Si los punteros extraídos por strsep() se asignaran directamente a los atributos de la estructura **Actividad**, todas las tareas terminarían apuntando a la última línea leída en la memoria. Se utilizó **strdup()** para obligar al sistema a asignar bloques de memoria independientes en el heap (vía malloc implícito) y copiar físicamente el contenido de cada string (ID, Nombre, Dependencias). Esto aísla los datos del ciclo de lectura de fgets y garantiza que la información referenciada sobreviva intacta durante toda la ejecución de la simulación.


