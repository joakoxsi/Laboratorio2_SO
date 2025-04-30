#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int id;         // Nuevo: ID único para cada carta
    int valor;      // Valor de la carta: 1-11
    char pinta;     // 'P', 'D', 'T', 'C'
} Carta;

typedef struct {
    char nombre[50];     // Nombre del jugador
    int puntos;          // Puntos acumulados
    int is_crupier;      // 1 si es crupier, 0 si es jugador normal
    Carta mano[12];      // Mano de cartas
    int num_cartas;      // Número de cartas en la mano
    int estado_ronda;
} Jugador;

int menu_inicial() {
    int rondas;
    printf("=== Bienvenido a Blackjack SO ===\n");
    printf("¿Cuántas rondas quieres jugar? (mínimo 5): ");
    scanf("%d", &rondas);

    if (rondas < 5) {
        printf("Se jugarán 5 rondas por defecto.\n");
        rondas = 5;
    }

    return rondas;
}







#define TOTAL_CARTAS 52

void inicializar_mazo(Carta mazo[]) {
    char pintas[] = {'P', 'D', 'T', 'C'}; // P = Picas, D = Diamantes, T = Tréboles, C = Corazones
    int index = 0;

    for (int p = 0; p < 4; p++) { // 4 pintas
        for (int v = 1; v <= 13; v++) { // 13 cartas por pinta
            mazo[index].id = index; // Asignamos ID secuencial
            mazo[index].valor = (v > 10) ? 10 : v; // J, Q, K valen 10
            mazo[index].pinta = pintas[p];
            index++;
        }
    }
}

void imprimir_mazo(Carta mazo[]) {
    for (int i = 0; i < TOTAL_CARTAS; i++) {
        printf("Carta %2d (ID %2d): Valor = %2d, Pinta = %c\n", i+1, mazo[i].id, mazo[i].valor, mazo[i].pinta);
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

void dar_carta_desde_mazo(Jugador *j, Carta mazo[], int *indice_mazo) {
    if (*indice_mazo >= TOTAL_CARTAS) {
        printf("No quedan más cartas en el mazo.\n");
        return;
    }

    if (j->num_cartas >= 12) {
        printf("El jugador %s ya tiene demasiadas cartas.\n", j->nombre);
        return;
    }

    // Asignar la carta del mazo al jugador
    j->mano[j->num_cartas] = mazo[*indice_mazo];
    j->num_cartas++;

    // Avanzar el índice del mazo
    (*indice_mazo)++;
}
void mostrar_mano(Jugador j) {
    if (j.is_crupier)
        printf("Mano del Crupier (%d cartas):\n", j.num_cartas);
    else
        printf("Mano de %s (%d cartas):\n", j.nombre, j.num_cartas);

    for (int i = 0; i < j.num_cartas; i++) {
        Carta c = j.mano[i];
        printf("  Carta %2d (ID %2d): Valor = %2d, Pinta = %c\n",
               i + 1, c.id, c.valor, c.pinta);
    }
}





int sumar_cartas(Jugador j) {
    int suma = 0;
    int cantidad_ases = 0;

    // Paso 1: suma normal y cuenta Ases
    for (int i = 0; i < j.num_cartas; i++) {
        int valor = j.mano[i].valor;
        if (valor == 1) {
            cantidad_ases++;
        }
        suma += valor;
    }

    // Paso 2: convierte Ases de 1 a 11 si conviene (sin pasarse de 21)
    while (cantidad_ases > 0 && suma + 10 <= 21) {
        suma += 10; // convertir un As de 1 → 11
        cantidad_ases--;
    }

    return suma;
}

int Pedir_bot(Jugador j) {
    int total = sumar_cartas(j); // Usa la función que ya hicimos

    if (total <= 11) {
        return 1; // Siempre pide
    } else if (total >= 12 && total <= 18) {
        return rand() % 2; // 50% de chance (0 o 1)
    } else {
        return 0; // No pide más
    }
}
#define NUM_JUGADORES 5  // 4 bots + 1 crupier

void inicializacion_(Jugador jugadores[], Carta mazo[], int *indice_mazo) {
    printf("=== Repartiendo cartas ===\n");

    // Repartir 2 cartas a cada jugador
    for (int i = 0; i < NUM_JUGADORES; i++) {
        dar_carta_desde_mazo(&jugadores[i], mazo, indice_mazo);
        dar_carta_desde_mazo(&jugadores[i], mazo, indice_mazo);
    }

    printf("\n=== Estado inicial de cada jugador ===\n");
    for (int i = 0; i < NUM_JUGADORES; i++) {
        if (jugadores[i].is_crupier) {
            // Mostrar solo la primera carta del crupier
            printf("Crupier muestra su primera carta:\n");
            Carta c = jugadores[i].mano[0];
            printf("  Carta visible (ID %d): Valor = %d, Pinta = %c\n", c.id, c.valor, c.pinta);
        } else {
            mostrar_mano(jugadores[i]);
        }
        printf("\n");
    }
}


void actualizar_estado(Jugador *j) {
    if (sumar_cartas(*j) > 21) {
        j->sobre_21 = 1;
    } else {
        j->sobre_21 = 0;
    }
}







int main() {

    Jugador jugadores[5];
    Carta mazo[TOTAL_CARTAS];
    int indice_mazo = 0;
    
    // Inicializar jugadores
    for (int i = 0; i < 5; i++) {
        char nombre[20];
        sprintf(nombre, "Bot %d", i + 1);
        strcpy(jugadores[i].nombre, nombre);
        jugadores[i].puntos = 0;
        jugadores[i].is_crupier = 0;
        jugadores[i].num_cartas = 0;
    }
    
    // El último es el crupier
    strcpy(jugadores[4].nombre, "Crupier");
    jugadores[4].is_crupier = 1;
    
    // Mazo inicial
    inicializar_mazo(mazo);
    revolver_mazo(mazo, TOTAL_CARTAS);
    
    // Repartir y mostrar
    inicializacion_(jugadores, mazo, &indice_mazo);


    return 0;
}