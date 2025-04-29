#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int valor;     // Valor de la carta: 1-11
    char pinta;    // 'P', 'D', 'T', 'C'
} Carta;

#define TOTAL_CARTAS 52

void inicializar_mazo(Carta mazo[]) {
    char pintas[] = {'P', 'D', 'T', 'C'}; // P = Picas, D = Diamantes, T = Tréboles, C = Corazones
    int index = 0;

    for (int p = 0; p < 4; p++) { // 4 pintas
        for (int v = 1; v <= 13; v++) { // 13 cartas por pinta
            mazo[index].valor = (v > 10) ? 10 : v; // J, Q, K valen 10
            mazo[index].pinta = pintas[p];
            index++;
        }
    }
}

void imprimir_mazo(Carta mazo[]) {
    for (int i = 0; i < TOTAL_CARTAS; i++) {
        printf("Carta %2d: Valor = %2d, Pinta = %c\n", i+1, mazo[i].valor, mazo[i].pinta);
    }
}

void revolver_mazo(Carta mazo[], int total_cartas) {
    srand(time(NULL)); // Inicializa semilla aleatoria (una sola vez al inicio)

    for (int i = total_cartas - 1; i > 0; i--) {
        int j = rand() % (i + 1); // Índice aleatorio entre 0 y i

        // Intercambia carta i con carta j
        Carta temp = mazo[i];
        mazo[i] = mazo[j];
        mazo[j] = temp;
    }
}



int main() {
    Carta mazo[TOTAL_CARTAS];

    inicializar_mazo(mazo);
    imprimir_mazo(mazo);
    revolver_mazo(mazo,TOTAL_CARTAS);
    imprimir_mazo(mazo);

    return 0;
}