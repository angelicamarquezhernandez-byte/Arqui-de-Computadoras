#include <stdio.h>
#include <stdlib.h>

void genFibonacci(int n){
    if(n <= 0){
        printf("Introduzca un num mayor a 0: /n");
        return;
    }
    long long t1=0, t2=1, sigTermino;
    printf("Serie de Fibonacci con %d terminos: /n",n);
    for(int i = 1; i <= n; i++){
        printf("%lld ",t1);
        sigTermino = t1 + t2;
        t1 = t2;
        t2 = sigTermino;
    }
    printf("/n");
}

int main(){
    int n=10;
    printf("Primeros %d numeros de la serie: /n",n);
    genFibonacci(n);
    return 0;
}
