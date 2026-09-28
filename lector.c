#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L //habilita funciones posix que no estan en c17, como strsep

#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <string.h>
#include "lector.h"

int procesar_archivo(char *nombre_archivo, struct Actividad *actividadesTotales, int *nroActividades){

    FILE *read = fopen(nombre_archivo, "r"); //puntero que lee archivo de texto en modo read

    if(read == NULL){
        printf("ERROR: Archivo no encontrado\n");
        return -1;
    }

    char buffer[1024];

    while(fgets(buffer, sizeof(buffer), read) != NULL){
        
        char *linea = buffer; // puntero auxiliar posicion strsep
        char *salto = ":"; 
        
        char *id = strsep(&linea, salto);
        char *nombreAct = strsep(&linea, salto);
        char *tiempoToken = strsep(&linea, salto);
        char *depend = strsep(&linea, salto); 
        
        int time;

        if(tiempoToken != NULL && atoi(tiempoToken) == 0){ //si no ingresaron tiempo, se randomiza
            time = 100 + rand() % (5000 - 100 + 1); 
        } else {
            time = atoi(tiempoToken);
        }

        int num_deps = 0;
        char *arrDepend[100]; // arreglo de dependencias

        if(depend != NULL){
            depend[strcspn(depend, "\n")] = '\0'; 
            
            char *coma = " ,";
            char *token_dep;
            
            while((token_dep = strsep(&depend, coma)) != NULL){
                if(token_dep[0] == ' '){
                    token_dep++; 
                }
                
                if(strlen(token_dep) > 0){ //se guarda cada dependencia en el arreglo
                    arrDepend[num_deps] = token_dep;
                    num_deps++;
                }
            }
        }

        struct Actividad actividad; // se inicializa el struct actividad

        actividad.estado = WAITING;
        actividad.tiempo_ms = time;
        actividad.num_depend = num_deps;
        actividad.cantidad_dependientes = 0;
        
        actividad.depend = malloc(num_deps * sizeof(char *)); // se le asigna memoria con malloc a depend
        
        for(int j = 0; j < num_deps; j++){
            actividad.depend[j] = strdup(arrDepend[j]); // strdup duplica el texto en memoria heap, para no perder referencia
        }

        actividad.ID_Actividad = strdup(id);
        actividad.Nombre_Actividad = strdup(nombreAct);

        actividadesTotales[*nroActividades] = actividad;
        (*nroActividades)++;    

        
    }

    for(int i = 0; i < *nroActividades; i++){                       //registrar nro de actividades dependientes
            
        for(int d = 0; d < actividadesTotales[i].num_depend; d++){      
                
            char *nombre_dep = actividadesTotales[i].depend[d];     //se posiciona en cada dependencia de la act i

            for(int j = 0; j < *nroActividades; j++){       

                if(strcmp(actividadesTotales[j].ID_Actividad, nombre_dep) == 0){    //busca la coincidencia

                    actividadesTotales[j].cantidad_dependientes++;
                    break; 
                }
            }
        }
    }
    
    fclose(read);
    return 0;
}