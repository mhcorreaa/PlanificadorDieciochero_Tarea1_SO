#ifndef LECTOR_H
#define LECTOR_H

#define READY 2
#define RUNNING 1
#define WAITING 0
#define FAILED -1

struct Actividad{
    char *ID_Actividad;
    char *Nombre_Actividad;
    int tiempo_ms;
    char **depend;
    int num_depend; // nro total de actividades que este depende 
    int estado;
    pid_t PID;
    int fd[2]; //pipe
    int cantidad_dependientes; //nro total de actividades que dependen de esta actividad
};

int procesar_archivo(char *nombre_archivo, struct Actividad *actividadesTotales, int *nroActividades);

#endif