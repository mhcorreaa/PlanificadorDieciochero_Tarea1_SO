#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <unistd.h> // syscalls
#include <sys/types.h> //wait
#include <sys/wait.h>
#include <signal.h> // ctrl + c seremi
#include "lector.h"
#include <string.h>


int revisarDependencias(struct Actividad *actividadesTotales, int id, int nroActividades, int *contadorActReady){  //revisa si puede ser ejecutada dicha actividad

    char **cDependencias = actividadesTotales[id].depend;
    int limite = actividadesTotales[id].num_depend;

    for(int i=0; i<limite; i++){

        char *idDependencia = cDependencias[i]; 

        for(int j=0; j<nroActividades; j++){

            if(strcmp(idDependencia, actividadesTotales[j].ID_Actividad) == 0 && actividadesTotales[j].estado == FAILED){ 

                actividadesTotales[j].estado = FAILED;  //si su dependencia fallo, pasa a failed tmb
                contadorActReady++;
                return 0;
            }

            if(strcmp(idDependencia, actividadesTotales[j].ID_Actividad) == 0 && actividadesTotales[j].estado != READY) return 0;
            
        }
    }

    return 1;
}

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Ejecucion incorrecta\n"); //revisa que la invocacion sea con el nro de argumentos correctos
        return -1;
    }

    int k = atoi(argv[2]);

    struct Actividad actividadesTotales[10000]; // arreglo de actividades
    int nroActividades = 0;

    if(procesar_archivo(argv[1], actividadesTotales, &nroActividades) == -1){ //se lee plan.txt en lector.c para guardar la lista de actividades
        return -1;
    }

    int contadorActReady = 0;
    int contadorActRunning = 0;

    while(contadorActReady < nroActividades){ //hasta que se ejecuten todas las actividades

        for(int i=0; i<nroActividades && contadorActRunning < k; i++){ //recorre el total de act

            if(actividadesTotales[i].estado == WAITING){ //se puede crear una nueva act?

                if(revisarDependencias(actividadesTotales, i, nroActividades, &contadorActReady)){ // revisa si todas sus dependencias estan listas, o si no tiene

                    printf("Iniciando actividad ID: %s\n", actividadesTotales[i].ID_Actividad);
                    printf("Accion:  %s\n\n", actividadesTotales[i].Nombre_Actividad);

                    actividadesTotales[i].estado = RUNNING;
                    contadorActRunning++;

                    pipe(actividadesTotales[i].fd); //inicializa el pipe 

                    pid_t pid = fork();

                    if(pid == 0){ 
                        
                        char buffer[10];
                        char mensaje[] = "Listo Brother";

                        for(int d = 0; d < actividadesTotales[i].num_depend; d++){ //lee el mensaje del pipe que le dejo cada dependencia
                            
                            char *id_buscado = actividadesTotales[i].depend[d];
                            
                            for(int j = 0; j < nroActividades; j++){
                                
                                if(strcmp(id_buscado, actividadesTotales[j].ID_Actividad) == 0){
                                    
                                    read(actividadesTotales[j].fd[0], buffer, sizeof(mensaje));
                                    break; 
                                }
                            }
                        }

                        close(actividadesTotales[i].fd[0]);  
                        
                        for(int j = 0; j < actividadesTotales[i].cantidad_dependientes; j++){ //le deja el mensaje a sus dependientes
                            
                            write(actividadesTotales[i].fd[1], mensaje, sizeof(mensaje));
                        }

                        close(actividadesTotales[i].fd[1]); 

                        usleep(actividadesTotales[i].tiempo_ms * 1000); //realiza su actividad
                        exit(0);
                    }

                    actividadesTotales[i].PID = pid;
                    close(actividadesTotales[i].fd[1]);
                }
            }
        }

        if(contadorActRunning > 0){
            
            int status;
            pid_t pidListo = wait(&status); // pid del primer hijo que termine
        
            for(int i=0; i<nroActividades; i++){
                
                if(actividadesTotales[i].PID == pidListo){
                    
                    if(WIFEXITED(status)){
                        
                        int exit_code = WEXITSTATUS(status);
                        
                        if(exit_code == 0) actividadesTotales[i].estado = READY; 
                               
                        else actividadesTotales[i].estado = FAILED;    
                    }
                    
                    contadorActRunning--;
                    contadorActReady++; 
                    break;
                }
            }
        }
    }

    return 0;
}