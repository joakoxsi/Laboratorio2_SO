#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define TAM_DECK 52
#define NUM_JUGADORES 5
#define CRUPIER -1
typedef enum { TREBOL, DIAMANTE, CORAZON, PICA } Palo;

typedef enum { AS = 1, DOS, TRES, CUATRO, CINCO, SEIS,
               SIETE, OCHO, NUEVE, DIEZ, J, Q, K } Valor;

typedef struct {
    Valor valor;         // Valor de la carta: 1 - 13 -> corregir que cartas valgan 1 - 10
    Palo palo;           // 'P', 'D', 'T', 'C'
} Carta;

typedef struct {
    int id_Jugador;      //Indica que jugador es
    Carta mano[12];      // Mano de cartas
    int num_cartas;      // Número de cartas en la mano
    int puntos;          // Puntos acumulados
    int gano_ronda;      // 1 si gano la ronda, 0 si pierde
} Jugador;


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

Carta dar_carta(Carta mazo[], int *indice_mazo) {
    if (*indice_mazo >= TAM_DECK) {
        printf("No quedan más cartas en el mazo.\n");
        return;
    }

    Carta carta = mazo[(*indice_mazo)];
    
    // Avanzar el índice del mazo
    (*indice_mazo)++;

    return carta;
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
    
    // Inicializar jugadores
    jugador.num_cartas = 0;
    jugador.puntos = 0;
    jugador.gano_ronda = 0;
    
    
    // Incializar crupier
    jugador.num_cartas = 0;
    jugador.puntos = 0;
    jugador.gano_ronda = 0;

    //Inicializar mazo
    Carta mazo[TAM_DECK];
    init_deck(mazo);
    int idx_mazo = 0;
    
    //=================Definicion de rondas======================
    int rondas;
    printf("=== Bienvenido a Blackjack SO ===\n");
    printf("¿Cuántas rondas quieres jugar? (mínimo 5): ");
    scanf("%d", &rondas);
    
    if (rondas < 5) {
        printf("Se jugarán 5 rondas por defecto.\n");
        rondas = 5;
    }
    //================Inicializacion de fork=======================
    int from_child[NUM_JUGADORES][2];   //PIPE para hijo escriba, padre lee
    int to_child[NUM_JUGADORES][2];     //PIPE para  padre escriba, hijo lee
    pid_t pids[NUM_JUGADORES];

    jugador.id_Jugador = CRUPIER; //Para el proceso padre
    //=======================Rondas=================================
    int ronda_actual = 1;
    while (ronda_actual <= rondas) {
        //======================fork===============================
        for (int i = 1; i <= NUM_JUGADORES; i++) {
            pids[i] = fork();

            if (pids[i] < 0) {
                perror("fork");
                exit(1);
            }
            //=================Proceso hijo=======================
            if (pids[i] == 0) {
                jugador.id_Jugador = i;
                //cerar PIPES para evitar errores
                close(to_child[i][1]);    // hijo no escribe al “to_child”
                close(from_child[i][0]);  // hijo no lee del “from_child”
                
                //definimos PIPES
                int rd = to_child[i][0];        // lee del padre
                int wr = from_child[i][1];      // escribe al padre
                
                //se reparten 2 cartas iniciales
                for (int j = 0; j < 2; j++) {
                    read(rd, &jugador.mano[jugador.num_cartas], sizeof(Carta));
                    jugador.num_cartas++;
                }

                //se cierran los PIPES por seguridad
                close(rd);
                close(wr);
                exit(0); //termina el hijo
            }
            //=================Proceso padre========================
            //cerar PIPES para evitar errores
            close(to_child[i][0]);              // padre no lee de “to_child”
            close(from_child[i][1]);            // padre no escribe en “from_child”  

            //definimos PIPES
            int rd_padre = from_child[i][0];          // lee del hijo
            int wr_padre = to_child[i][1];            // escribe al hijo
            
            barajar_deck(mazo); //el proceso padre maneja las cartas
            
            // Padre reparte dos cartas al jugador i
            for (int j = 0; j < 2; j++) {
                Carta carta = dar_carta(mazo, idx_mazo);  // generar carta desde el mazo
                write(wr_padre, &carta, sizeof(Carta));
            }
       }
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
        rondas++;
    }
    
    return 0;
}