#include<sys/types.h>  	//Primitive system data types for abstraction of implementation-dependent data types.
						//POSIX Standard: 2.6 Primitive System Data Types <sys/types.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include <time.h>

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("El uso correcto es: benchmark <número de GB> <archivo> <directorios>.\n");
        printf("Donde del archivo se tomará 1 MB(se recomienda /dev/urandom), el resto hasta completar el GB");
        printf("se tomará copiando este contenido 1024 veces, y una vez obtenido el GB se escribirá o leerá");
        printf("de los archivos de 1 en 1 GB hasta llegar al parámetro indicado.\n");
        printf("La prueba se realizará en cada uno de los directorios pasados como parámetro");
    }

    size_t GB = 1024*1024*1024;		//1GB
    struct timeval start, end, diff;
    FILE* fd_res, *fd_orig, *fd_w, *fd_r, *fd_w2;
    char *buffer = (char*) malloc (GB), *useless = (char*) malloc (GB), *aux = (char*) malloc (GB);
    
    if(buffer == NULL){
        printf("malloc error \n");
        exit(-1);
    }

    fd_res=fopen("./resultados.csv", "w");

    int num_veces = atoi(argv[1]);

    fd_orig=fopen(argv[2], "r");

    int i;
    for(i=0; i<5; i++){
        fprintf(fd_res, "%dª iteración:\n", i+1);

        /*******
         * Inicialización del buffer, pueden ser valores aleatorios de /dev/random en el
         * caso de usar Linux o tomar los valores de un archivo.
         * Aunque no cuenta en la medición del tiempo, por eficiencia, tomo 1MB de este
         * fichero y luego lo copio hasta tener 1GB.
         *******/
        if(fread(buffer, sizeof(char), GB/(sizeof(char)*1024), fd_orig) != GB/1024){
            printf("\nError %d en inicialización buffer",errno);
            perror("\nError en fread");
            exit(-1);
        }
        
        int j;
        for(j = 1; j < 10; j++){
            strcpy(aux, buffer);
            strcat(buffer, aux);
        }



        for(j = 3; j < argc; j++){
            char archivo_prueba[strlen(argv[j]) + 10];
            char archivo_prueba2[strlen(archivo_prueba)];

            fprintf(fd_res, "%s:\n", argv[j]);

            strcpy(archivo_prueba, argv[j]);
            
            if(archivo_prueba[strlen(argv[j])-1] != '/')
                strcat(archivo_prueba, "/");

            strcat(archivo_prueba, "prueba");                       // Creará <directorio dado>/prueba

            strcpy(archivo_prueba2,archivo_prueba);
            strcat(archivo_prueba2, "2");                           // Creará <directorio dado>/prueba2
       
            // Primera prueba(Escritura)
            fd_w=fopen(archivo_prueba, "w");

            gettimeofday(&start, NULL);

            int k;
            for(k = 0; k < num_veces; k++){
                if(fwrite(buffer, sizeof(char), GB/sizeof(char), fd_w) != GB){
                    printf("\nError %d en prueba 1",errno);
                    perror("\nError en fwrite");
                    exit(-1);
                };
            }

            gettimeofday(&end, NULL);
            timersub(&end, &start, &diff);
            fprintf(fd_res, "%ld.%04ld,\t", (long int)diff.tv_sec, (long int)diff.tv_usec);

            fclose(fd_w);

            // Segunda prueba(Lectura)
            fd_r=fopen(archivo_prueba, "r");

            gettimeofday(&start, NULL);

            for(k = 0; k < num_veces; k++){
                if(fread(useless, sizeof(char), GB/sizeof(char), fd_r) != GB){
                    printf("\nError %d en prueba 2",errno);
                    perror("\nError en fread");
                    exit(-1);
                };
            }

            gettimeofday(&end, NULL);
            timersub(&end, &start, &diff);
            fprintf(fd_res, "%ld.%04ld,\t", (long int)diff.tv_sec, (long int)diff.tv_usec);

            fclose(fd_r);

            // Tercera prueba(Lectura y escritura)
            fd_r=fopen(archivo_prueba, "r");
            fd_w2=fopen(archivo_prueba2, "w");

            gettimeofday(&start, NULL);

            for(k = 0; k < num_veces; k++){
                if(fread(aux, sizeof(char), GB/sizeof(char), fd_r) != GB){
                    printf("\nError %d en prueba 3",errno);
                    perror("\nError en fread");
                    exit(-1);
                };

                if (fwrite(aux, sizeof(char), GB/sizeof(char), fd_w2) != GB){
                    printf("\nError %d en prueba 3",errno);
                    perror("\nError en fwrite");
                    exit(-1);
                };
            }

            gettimeofday(&end, NULL);
            timersub(&end, &start, &diff);
            fprintf(fd_res, "%ld.%04ld,\t", (long int)diff.tv_sec, (long int)diff.tv_usec);

            fclose(fd_r);
            fclose(fd_w2);
            
            remove(archivo_prueba);
            remove(archivo_prueba2);

            fprintf(fd_res, "\n");
        }
    }

    fclose(fd_orig);
    fclose(fd_res);

    return 0;
}
