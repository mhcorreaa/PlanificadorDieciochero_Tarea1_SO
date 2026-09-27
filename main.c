#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <unistd.h> // syscalls
#include <string.h>
#include <sys/types.h> //wait
#include <sys/wait.h>
#include <signal.h>

struct Actividad{

    char *ID_Actividad;
    char *Nombre_Actividad;
    int tiempo_ms;
    char **depend;
    int num_depend; //para saber tamano de depend
};

int main(int argc, char *argv[]){

    if(argc != 3){

        printf("Ejecucion incorrecta");
        return -1;
    }

    int k = atoi(argv[2]);

    FILE *read = fopen(argv[1], "r"); //puntero que lee archivo de texto en modo read

    if(read == NULL){

        printf("ERROR: Archivo no encontrado");
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

        struct Actividad nueva_actividad; // se inicializa el struct actividad

        nueva_actividad.tiempo_ms = time;
        nueva_actividad.num_depend = num_deps;
        
        nueva_actividad.depend = malloc(num_deps * sizeof(char *)); // se le asigna memoria con malloc a depend
        
        for(int j = 0; j < num_deps; j++){
            // strdup duplica el texto en memoria heap (malloc + strcpy automatico)
            nueva_actividad.depend[j] = strdup(arrDepend[j]); 
        }

        nueva_actividad.ID_Actividad = strdup(id);
        nueva_actividad.Nombre_Actividad = strdup(nombreAct);

    }

    fclose(read);

    return 0;
}