#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAM_DECK 52

typedef enum { TREBOL, DIAMANTE, CORAZON, PICA } Palo;

typedef enum { AS = 1, DOS, TRES, CUATRO, CINCO, SEIS,
               SIETE, OCHO, NUEVE, DIEZ, J, Q, K } Valor;

typedef struct {
    Valor valor;         // Valor de la carta: 1 - 13 -> corregir que cartas valgan 1 - 10
    Palo palo;           // 'P', 'D', 'T', 'C'
} Carta;

typedef struct {
    Carta mano[12];      // Mano de cartas
    int num_cartas;      // Número de cartas en la mano
    int puntos;          // Puntos acumulados
    int gano_ronda;      // 1 si gano la ronda, 0 si pierde
} Jugador;

typedef struct {
    Carta mano[12];      // Mano de cartas
    int num_cartas;      // Número de cartas en la mano
    int puntos;          // Puntos acumulados
    int gano_ronda;      // 1 si gano la ronda, 0 si pierde
} Crupier;

// Auxiliares para convertir a cadena
const char *valor_str(Valor v) {
    static const char *names[] =
        { "", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
    return names[v];
}

const char palo_str(Palo p) {
    static const char *names[] =
        { "♣", "♦", "♥", "♠" };
    return names[p];
}

void print_carta(const Carta *c) {
    printf("%2s%s", valor_str(c->valor), palo_str(c->palo));
}

void init_deck(Carta mazo[]) {
    int index = 0;

    for (Palo p = TREBOL; p < PICA; p++) {      // 4 pintas
        for (Valor v = AS; v <= K; v++) {       // 13 cartas por pinta
            mazo[index].valor = v;              // corregir que J,Q,K valgan 10
            mazo[index].palo = p;
            index++;
        }
    }
}

//para pruebas
void print_deck(const Carta mazo[], int n) {
    for (int i = 0; i < n; i++) {
        print_carta(&mazo[i]);
        if ((i + 1) % 13 == 0)
            printf("\n");
        else
            printf("  ");
    }
    // si n no es múltiplo de 13, cerramos con un salto
    if (n % 13 != 0) printf("\n");
}

void barajar_deck(Carta mazo[]) {
    for (int i =  - 1; i > 0; i--) {
        int j = rand() % (i + 1);               // Índice aleatorio entre 0 y i
        // Intercambia carta i con carta j
        Carta tmp = mazo[i];
        mazo[i] = mazo[j];
        mazo[j] = tmp;
    }
}

void dar_carta(Jugador *j, Carta mazo[], int *indice_mazo) {
    if (*indice_mazo >= TAM_DECK) {
        printf("No quedan más cartas en el mazo.\n");
        return;
    }

    // Asignar la carta del mazo al jugador
    j->mano[j->num_cartas] = mazo[*indice_mazo];
    j->num_cartas++;
    
    // Avanzar el índice del mazo
    (*indice_mazo)++;

    return;
}

int valor_mano(Jugador j) { //revisar puede que sea Jugador* j
    int sum , ases = 0;
    
    for (int i = 0; i < j.num_cartas; i++) {
        Valor v = j.mano[i].valor;
        //paso 1: Suma los Aces como 11 y los cuenta
        if (v == AS) {
            ases++;
            sum += 11;
        //paso 2: corrige el valor de los J, Q, K a 10
        } else if (v >= J && v <= K) {
            sum += 10;
        //paso 3: toma el valor de los numeros
        } else {
            sum += (int)v;
        }
    }
    
    // Si la suma da mayor a 21 y tengo ases, bajo su valor a 1
    while (ases > 0 && sum > 21) {
        sum -= 10;       //11->1
        ases--;
    }
    
    return sum;
}

//devuelve un bool si el bot debe pedir
int Pedir_bot(Jugador j) {
    int total = valor_mano(j); 

    if (total <= 11) {
        return 1; // Siempre pide
    } else if (total >= 12 && total <= 18) {
        return rand() % 2; // 50% de chance (0 o 1)
    } else {
        return 0; // No pide más
    }
}

/*

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
*/

int main() {
    srand(time(NULL));   // Inicializa semilla aleatoria (una sola vez al inicio)
    
    Jugador jugador;
    Carta mazo[TAM_DECK];
    int indice_mazo = 0;
    
    // Inicializar jugadores
    jugador.num_cartas = 0;
    jugador.puntos = 0;
    jugador.gano_ronda = 0;
    
    
    // Incializar crupier
    jugador.num_cartas = 0;
    jugador.puntos = 0;
    jugador.gano_ronda = 0;
    
    //=================Definicion de rondas======================
    int rondas;
    printf("=== Bienvenido a Blackjack SO ===\n");
    printf("¿Cuántas rondas quieres jugar? (mínimo 5): ");
    scanf("%d", &rondas);
    
    if (rondas < 5) {
        printf("Se jugarán 5 rondas por defecto.\n");
        rondas = 5;
    }
    //=====================Rondas=================================
    int ronda_actual = 1;
    while (ronda_actual <= rondas) {
        /*
        barajar cartas
        */
        /*
        entregar cartas a bots y crupier
        mostrar cartas de bots
        mostrar 1 carta de crupier y la otra boca abajo (XX)
        */
        /* 
        por cada bot pedir cartas
        mostrar cartas
        repetir si es necesario
        si se pasan registrar que perdieron
        */
        /*
        dar cartas a crupier o plantarse
        si plantarse -> dar puntos a bots, 
                     -> contar derrotas
                     -> dar puntos a crupier (1 si 51% derrotas, 2 si 100% derrotas, 0 etoc)
        */
        
    }
    
    return 0;
}