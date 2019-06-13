#include <iostream>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
using namespace std;

double *mtrzA;
double *mtrzB;
double *mtrzC;

struct Data{
    unsigned i, j, k, nFilA, nColAFilB, nColB;
};

bool validarProducto(unsigned ca, unsigned fb){ //Valida el producto matricial
    return ca == fb;
}

void allocateMemory(char select, unsigned filas, unsigned columnas){ //Alocar memoria
    switch (select)
    {
        case 'A':
            mtrzA = new double[filas*columnas];
            break;
        case 'B':
            mtrzB = new double[filas*columnas];
            break;
        case 'C':
            mtrzC = new double[filas*columnas];
            break;
        default:
            cout <<"¡Selección inválida!";
    }
}

void print(char select, unsigned filas, unsigned columnas){
    switch (select)
    {
        case 'A':
            printf("Matriz A: \n");
            for(int i = 0; i < filas; i++){
                for(int j = 0; j < columnas; j++)
                    cout <<*(mtrzA + i*columnas + j) << " ";
                cout << endl;
            }
            break;
        case 'B':
            printf("Matriz B: \n");
            for(int i = 0; i < filas; i++){
                for(int j = 0; j < columnas; j++){
                    cout <<*(mtrzB + i*columnas + j) << " ";
                }
                cout << endl;
            }
            break;
        case 'C':
            printf("Matriz C: \n");
            for(int i = 0; i < filas; i++){
                for(int j = 0; j < columnas; j++){
                    cout <<*(mtrzC + i*columnas + j) << " ";
                }
                cout << endl;
            }
            break;
        default:
            cout <<"¡Selección inválida!";
    }
}

void fillNumbers(char select, unsigned filas, unsigned columnas){
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
            cout <<"¡Selección inválida!";
    }
}

void manualFill(char select, unsigned filas, unsigned columnas){
    switch (select)
    {
        case 'A':
            for(int i = 0; i < filas; i++)
                for(int j = 0; j < columnas; j++)
                    cin >> *(mtrzA + i*columnas + j);
            break;
        case 'B':
            for(int i = 0; i < filas; i++)
                for(int j = 0; j < columnas; j++)
                    cin >> *(mtrzB + i*columnas + j);
            break;
        default:
            cout <<"¡Selección inválida!";
    }
}

void execute (char select, unsigned &filas, unsigned &columnas,  bool fill,  bool show){

    cout << "Número de filas para la matriz " << select << ":" << endl;
    cin >> filas;
    cout << "Número de columnas para la matriz " << select << ":" << endl;
    cin >> columnas;
    allocateMemory(select, filas, columnas);

    (fill)? fillNumbers(select, filas, columnas) : manualFill(select, filas, columnas);

    if (show) print(select, filas, columnas);
}

void *calcular (void *parametros){ //producto matricial con parametros
    Data *datos = (Data *) parametros;
    for (; datos->i < datos->nFilA; datos->i++){
        for(datos->j = 0; datos->j <  datos->nColB; datos->j++){
            double sumOfProd = 0;
            for(datos->k = 0; datos->k < datos->nColAFilB; datos->k++){
                double a = *(mtrzA + datos->i*datos->nColAFilB + datos->k);
                double b = *(mtrzB + datos->k*datos->nColB + datos->j);
                sumOfProd += a*b;
                *(mtrzC + datos->i*datos->nColB + datos->j) = sumOfProd;
            }
        }
    }
    //print('C',fa,cb);
    return nullptr;
}

double productWithThreads(unsigned nthreads, unsigned fa, unsigned cafb, unsigned cb, bool mostrar){
    //Para medir el tiempo de ejecución
    double tStart, tEnd, tTotal;
    struct timeval time{};
    gettimeofday(&time, nullptr);
    tStart = time.tv_sec + time.tv_usec/1000000.0;

    cout << "Multiplicando matrices con " << nthreads << " hilos" << endl;
    pthread_t hilos[nthreads];

    for (int z = 0; z < nthreads; z++){
        Data *parametros = new Data;
        parametros->nColAFilB = cafb;
        parametros->nColB = cb;

        if(fa % nthreads == 0){ //si las filas de A son divisibles por la cantidad de hilos
            parametros->i = (z)*fa/nthreads; //liminf
            parametros->nFilA =(z+1)*fa/nthreads; //limsup

        }else{ //cuando no es divisible

            parametros->i = (z)*(fa/nthreads +1);
            parametros->nFilA = (z+1)*(fa/nthreads+1);
            //Corregir cuando el limsup supera a fa
            if(parametros->nFilA > fa || ((z+2)*fa/nthreads) > fa)
                parametros->nFilA = fa;
        }

        pthread_attr_t pt_attr;
        pthread_attr_init(&pt_attr);
        pthread_create(&hilos[z], &pt_attr, calcular, parametros);

    }

    //union de threads
    for(int z = 0; z < nthreads; z++) 
        pthread_join(hilos[z],nullptr);
    

    gettimeofday(&time, nullptr);
    tEnd = time.tv_sec + (time.tv_usec /1000000.0);
    tTotal = (double)(tEnd-tStart);

    if (mostrar) print('C',fa, cb);

    return tTotal;
}

 
int main(){
    bool autofill, show;
    unsigned nHilos = 1, fa, ca, fb, cb;
    cout << "Multiplicacion de Matrices" << endl;
    cout << "Desea autorrellenar las matrices? 1 (Sí), 0 (No) " << endl;
    cin >> autofill;
    cout << "Desea mostrar las matrices? 1 (Sí), 0 (No) " << endl;
    cin >> show;
    cout << "Cuantos hilos desea emplear?" << endl;
    cin >> nHilos;

    execute('A', fa, ca, autofill, show);
    execute('B', fb, cb, autofill, show);
    allocateMemory('C', fa, cb);
    
    if(validarProducto(ca, fb)){
        for(int i = 1; i <= nHilos; i++){ //Realizar la misma operación con diferenete num hilos
            cout << "----------------------------------------"  << endl;
            double time = productWithThreads(i, fa, ca, cb, false);
            cout << "Tiempo para " << i << " hilos " << time <<"s"<< endl;
            cout << "----------------------------------------\n"  << endl;
        }
    }else
        cout << "Producto Matricial Inválido!" << endl;


} //tEnd de main


