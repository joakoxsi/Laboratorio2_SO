#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define TAM_DECK 52
#define NUM_JUGADORES 4
#define CRUPIER -1

#define CMD_TERMINAR 0
#define CMD_NUEVA_RONDA 1
#define CMD_FIN_RONDA 2

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

typedef struct {
    Carta mazo[TAM_DECK];
    int idx_mazo;
    Jugador jugadores[NUM_JUGADORES];
    int comando[NUM_JUGADORES]; // el padre escribe comandos, hijos los lee
} MemCompartida;

// Auxiliares para convertir a cadena
const char *valor_str(Valor v) {
    static const char *names[] =
        { "", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
    return names[v];
}

const char *palo_str(Palo p) {
    static const char *names[] =
        { "♣", "♦", "♥", "♠" };
    return names[p];
}

void print_carta(Carta c) {
    printf("%2s%s", valor_str(c.valor), palo_str(c.palo));
}

void print_mano(Jugador j) {
    Carta* mano = j.mano;
    int cant_mano = j.num_cartas;

    for (int i = 0; i < cant_mano; i++) {
        print_carta(mano[i]);
    }
}

void print_jugador(Jugador j) {
    if (j.id_Jugador == CRUPIER) {
        printf("Crupier: ");
        print_mano(j);
        printf("\n");
    } else {
        printf("Bot %d: ", j.id_Jugador);
        print_mano(j);
        printf("\n");
    }
    return;
}

void print_crupierInit(Jugador j) {
    Carta* mano_init = j.mano;

    printf("Crupier: ");
    //como la primera mano siempre son 2 se imprime la 1ra carta
    print_carta(mano_init[0]);
    printf(" XX");
    printf("\n");

    return;
}

void init_deck(Carta mazo[]) {
    int index = 0;

    for (int p = 0; p < 4; p++) {               // 4 pintas
        for (int v = 1; v <= 13; v++) {         // 13 cartas por pinta
            mazo[index].valor = (Valor)v;       // corregir que J,Q,K valgan 10
            mazo[index].palo = (Palo)p;
            index++;
        }
    }
}

//para pruebas
void print_deck(const Carta mazo[], int n) {
    for (int i = 0; i < n; i++) {
        print_carta(mazo[i]);
        if ((i + 1) % 13 == 0)
            printf("\n");
        else
            printf("  ");
    }
    // si n no es múltiplo de 13, cerramos con un salto
    if (n % 13 != 0) printf("\n");
}

void barajar_deck(Carta mazo[]) {
    for (int i =  TAM_DECK - 1; i > 0; i--) {
        int j = rand() % (i + 1);               // Índice aleatorio entre 0 y i
        // Intercambia carta i con carta j
        Carta tmp = mazo[i];
        mazo[i] = mazo[j];
        mazo[j] = tmp;
    }
}

Carta dar_carta(Carta mazo[], int *indice_mazo) {
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
int pedir_carta(Jugador j) {
    int total = valor_mano(j); 

    if (total <= 11) {
        return 1; // Siempre pide
    } else if (total >= 12 && total <= 18) {
        return rand() % 2; // 50% de chance (0 o 1)
    } else {
        return 0; // No pide más
    }
}

int main() {
    srand(time(NULL));   // Inicializa semilla aleatoria

    //================Incializacion memCompartida==================
    FILE *f = fopen("ipc.key", "a"); fclose(f); //Creo fichero para ocuparlo en la llave
    //Se generan llaves para que los procesos compartan la memoria
    key_t shm_key = ftok("ipc.key", 'S');
    key_t sem_key = ftok("ipc.key", 'M');

    
    Jugador jugador;
    
    // Inicializar jugadores
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
    /*
    if (rondas < 5) {
        printf("Se jugarán 5 rondas por defecto.\n");
        rondas = 5;
    }*/
    //================Inicializacion de fork=======================
    int from_child[NUM_JUGADORES][2];   //PIPE para hijo escriba, padre lee
    int to_child[NUM_JUGADORES][2];     //PIPE para padre escriba, hijo lee
    pid_t pids[NUM_JUGADORES];

    jugador.id_Jugador = CRUPIER; //Para el proceso padre
    //========================fork==================================
    for (int i = 0; i < NUM_JUGADORES; i++) {
        pipe(to_child[i]);
        pipe(from_child[i]);
        pids[i] = fork();
        if (pids[i] < 0) { perror("fork"); exit(1); }
        //======================Hijo============================
        if (pids[i] == 0) {
            close(to_child[i][1]);      //cierro PIPE para que hijo no escriba
            close(from_child[i][0]);    //cierro PIPE para que hijo no lea
            int rd = to_child[i][0];
            int wr = from_child[i][1];
            jugador.id_Jugador = i + 1;

            while (1) {
                int comando;
                read(rd, &comando, sizeof(int));

                if (comando == CMD_TERMINAR) {
                    //al terminar comunicar los puntos al Crupier
                    write(wr, &jugador.puntos, sizeof(jugador.puntos));
                    close(rd);
                    close(wr);
                    exit(0); //cuidado con los procesos zombi
                } else if (comando == CMD_NUEVA_RONDA) {
                    //2 cartas iniciales
                    Carta c;
                    for (int j = 0; j < 2; j++) {
                        read(rd, &c, sizeof(c));
                        jugador.mano[jugador.num_cartas] = c;
                        jugador.num_cartas++;
                    } print_jugador(jugador);

                    //pedir más cartas segun estrategia 
                    int pedir = pedir_carta(jugador);
                    write(wr, &pedir, sizeof(pedir));
                    while (pedir) {
                        read(rd,&c,sizeof(Carta));
                        jugador.mano[jugador.num_cartas] = c;
                        jugador.num_cartas++;

                        pedir = pedir_carta(jugador);
                        write(wr, &pedir, sizeof(pedir));
                    }
                } else if (comando == CMD_FIN_RONDA) {
                    //mandar puntos
                    //recibir puntaje
                } 
            }
        }
        //======================Padre===========================
        close(to_child[i][0]);          //cierro PIPE para que padre no lea
        close(from_child[i][1]);        //cierro PIPE para que padre no escriba 
    }
    //=======================Rondas=================================
    for (int ronda_actual = 1; ronda_actual <= rondas; ronda_actual++) {
        printf("\n=== Ronda %d ===\n", ronda_actual);
        barajar_deck(mazo);
        idx_mazo = 0;
        //avisa a todos que se empezo una nueva ronda
        for (int i = 0; i < NUM_JUGADORES; i++) {
            int comando = CMD_NUEVA_RONDA;
            write(to_child[i][1], &comando, sizeof(comando));
        }
        //repartir 2 cartas iniciales
        for (int i = 0; i < NUM_JUGADORES; i++) {
            for (int j = 0; j < 2; j++) {
                Carta c = dar_carta(mazo, &idx_mazo);
                write(to_child[i][1], &c, sizeof(c));
            }
        }
        // Repartir 2 cartas al crupier --> a si mismo
        for (int j = 0; j < 2; j++) {
            Carta c = dar_carta(mazo, &idx_mazo);
            jugador.mano[jugador.num_cartas++] = c;
        }
        print_crupierInit(jugador);
        // Repartir cartas si piden
        for (int i = 0; i < NUM_JUGADORES; i++) {
            int pide;
            read(from_child[i][0], &pide, sizeof(pide));
            while (pide) {
                Carta c = dar_carta(mazo, &idx_mazo);
                write(to_child[i][1], &c, sizeof(c));
                read(from_child[i][0], &pide, sizeof(pide));
            }
        }
        //Turno del Crupier
        int opcion = 0;
        printf("=== Tu turno! ===\n");
        //imprimir ambas cartas
        printf("¿Qué quieres hacer?\n");
        printf("(1) Agregar carta            (2) Plantarse\n");
        printf("Escoge una opción: ");
        scanf("%d", &opcion);
        /*
        while (opcion != 2) {
            //dar carta
        }*/
        
    }
    //====================Termino del juego===============================
    for (int i = 0; i < NUM_JUGADORES; i++) {
        int comando = CMD_TERMINAR;
        write(to_child[i][1], &comando, sizeof(comando));
        //recibe puntos de bots i
        //decide quien gano
        close(to_child[i][1]);
        close(from_child[i][0]);
    }
    for (int i = 0; i < NUM_JUGADORES; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    return 0;
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