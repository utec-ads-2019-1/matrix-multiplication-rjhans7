#include <iostream>
#include <time.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#define NUM_HILOS 32

double *mtrzA;
double *mtrzB;
double *mtrzC;

//Declaro una 
struct data{
    int i, j, k; 
    int nFilA, nColAFilB, nColB;
};

void allocateMemory(char select, int filas, int columnas); 
void print(char select, int filas, int columnas); //imprime las matrices
void fillNumbers(char select, int filas, int columnas); //llena las matrices
void execute (char select, int filas, int columnas); //llama a allocateMemory y fillNumbers
int validarProducto(int ca, int fb){ return (ca==fb)? 1: 0;} //Valida el producto matricial
void productoMatricial(int fa, int cafb, int cb); //producto matricial secuencial
void *calcular (void *parametros);
double productWithThreads(int nthreads, int fa, int cafb, int cb);

 
int main(){
    int fa, ca, fb, cb, rc; 
    long t;
    pthread_t hilos[NUM_HILOS];
    FILE *fp;
    fp = fopen ( "fichero.txt", "a+t" ); 
    printf("Multiplicacion de Matrices\n");
  
    printf("Número de filas para la matriz %c:\n", 'A');
    scanf("%d",&fa);
    printf("Número de columnas para la matriz %c:\n", 'A');
    scanf("%d",&ca);
    execute('A', fa, ca);

    printf("Número de filas para la matriz %c:\n", 'B');
    scanf("%d",&fb);
    printf("Número de columnas para la matriz %c:\n", 'B');
    scanf("%d",&cb);
    execute('B', fb, cb);
    allocateMemory('C', fa, cb);
    
    if(validarProducto(ca, fb)){
        for(int i = 1; i <= NUM_HILOS; i++){ //Realizar la misma operación con diferenete num hilos
            printf("----------------------------------------\n");
            double time = productWithThreads(i, fa, ca, cb);
            fprintf(fp,"%f, ", time);
            printf("Tiempo para %d hilos %f s\n",i, time);
            printf("Performance para %d hilos: %f \n",i,1/time);
            printf("----------------------------------------\n\n");
        }
    }else{
        printf("Producto Matricial Inválido!\n");
    }
    
    fclose ( fp );
} //fin de main

void allocateMemory(char select, int filas, int columnas){
switch (select)
{
case 'A':
    mtrzA=(double*)malloc(sizeof(double *)*filas + sizeof(double)*columnas*filas);
    break;
case 'B':
    mtrzB=(double*)malloc(sizeof(double *)*filas + sizeof(double)*columnas*filas); 
    break;
case 'C':
    mtrzC=(double*)malloc(sizeof(double *)*filas + sizeof(double)*columnas*filas);
    break;
default:
    printf("Error! operator is not correct");
}
};
void print(char select, int filas, int columnas){
    
    switch (select)
    {
    case 'A':
    printf("Matriz A: \n");
        for(int i = 0; i < filas; i++){
            for(int j = 0; j < columnas; j++)
                printf("%lf ",*(mtrzA + i*columnas + j));  
        printf("\n");
    }
    break;
    case 'B':
    printf("Matriz B: \n");
        for(int i = 0; i < filas; i++){
            for(int j = 0; j < columnas; j++){
            printf("%lf ",*(mtrzB + i*columnas + j));  
        }
        printf("\n");
    }
        break;
    case 'C':
    printf("Matriz C: \n");
        for(int i = 0; i < filas; i++){
            for(int j = 0; j < columnas; j++){
            printf("%lf ",*(mtrzC + i*columnas + j));  
        }
        printf("\n");
    }
    break;
    default:
        printf("Error! operator is not correcta");
    }
};

void fillNumbers(char select, int filas, int columnas){
    switch (select)
    {
    case 'A':
        for(int i = 0; i < filas; i++)
            for(int j = 0; j < columnas; j++)
                *(mtrzA + i*columnas + j) = rand()%15;
            
        break;
    case 'B':
        for(int i = 0; i < filas; i++)
            for(int j = 0; j < columnas; j++)
                *(mtrzB + i*columnas + j) = rand()%15;
    break;
    default:
        printf("Error! operator is not correct");
    }
};
void execute (char select, int filas, int columnas){
    allocateMemory(select, filas, columnas);
    fillNumbers(select, filas, columnas);
    //print(select, filas, columnas);
};

void productoMatricial(int fa, int cafb, int cb){
        for (int i = 0; i < fa; i++){
            for(int j = 0; j < cb; j++){
                int temp = 0;
                for(int k = 0; k < cafb; k++){
                    int a = *(mtrzA + i*cafb + k); 
                    int b = *(mtrzB + k*cb + j);
                    temp += a*b; 
                    *(mtrzC + i*cb + j) = temp;             
                }
            }
        }
        print('C',fa,cb);
}; //fin de producto matricial secuencial

void *calcular (void *parametros){ //producto matricial con parametros
    int i, j, k, fa, cafb, cb;
    long t;
    struct data *datos = (data *)parametros;

    i = datos->i;
    j = datos->j;
    k = datos->k;
    fa = datos->nFilA;
    cafb = datos->nColAFilB;
    cb = datos->nColB;

    for (; i < fa; i++){
        for(int j = 0; j < cb; j++){
            int temp = 0;
            for(int k = 0; k < cafb; k++){
                int a = *(mtrzA + i*cafb + k);
                int b = *(mtrzB + k*cb + j);
                temp += a*b;
                *(mtrzC + i*cb + j) = temp;
            }
        }
    }
    //print('C',fa,cb);
}
double productWithThreads(int nthreads, int fa, int cafb, int cb){
    double inicio, fin;
    double tiempoTotal;
    struct timeval tiempo;

    printf("Multiplicando matrices con %d hilos. \n", nthreads);
    gettimeofday(&tiempo, NULL);
    inicio = tiempo.tv_sec + tiempo.tv_usec/1000000.0;

    pthread_t hilos[nthreads];

    for (int z = 0; z < nthreads; z++){
        struct data *parametros = (data *)malloc(sizeof(struct data));

        parametros->nColAFilB = cafb;
        parametros->nColB = cb;

        if(fa % nthreads == 0){ //si las filas de A son divisibles por la cantidad de hilos
            parametros->i = (z)*fa/nthreads; //liminf
            parametros->nFilA =(z+1)*fa/nthreads; //limsup
        }else{ //cuando no es divisible
            parametros->i = (z)*(fa/nthreads +1);
            parametros->nFilA = (z+1)*(fa/nthreads+1);
        }
        if(parametros->nFilA > fa || ((z+2)*fa/nthreads)>fa){ //Corregir cuando el limsup supera a fa
            parametros->nFilA = fa;
        }
        pthread_attr_t pt_attr; pthread_attr_init(&pt_attr);
        int resultadoThread = pthread_create(&hilos[z], &pt_attr, calcular, parametros);
        if (resultadoThread  == -1) { printf("Error creando el hilo.\n"); return -1;}

    }
    //union de threads
    for(int z = 0; z < nthreads; z++){ pthread_join(hilos[z],NULL);}

    gettimeofday(&tiempo, NULL);
    fin = tiempo.tv_sec + (tiempo.tv_usec /1000000.0);
    //print('C',fa, cb);

    tiempoTotal = (double)(fin-inicio);
    return tiempoTotal;
}

