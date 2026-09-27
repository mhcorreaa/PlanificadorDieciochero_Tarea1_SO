#ifndef LECTOR_H
#define LECTOR_H

#define READY 2
#define RUNNING 1
#define WAITING 0

struct Actividad{
    char *ID_Actividad;
    char *Nombre_Actividad;
    int tiempo_ms;
    char **depend;
    int num_depend; //para saber tamano de depend
    int estado;
};

int procesar_archivo(char *nombre_archivo, struct Actividad *actividadesTotales, int *nroActividades);

#endif