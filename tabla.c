#include <stdio.h>
#include "tabla.h"

char tabla[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA];

void initializare_tabla(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA]) {
    // Randul 0: Piesele grele ale jucatorului Negru
    t[0][0] = 'r'; t[0][1] = 'n'; t[0][2] = 'b'; t[0][3] = 'q';
    t[0][4] = 'k'; t[0][5] = 'b'; t[0][6] = 'n'; t[0][7] = 'r';

    // Randul 1: Pionii jucatorului Negru
    for (int col = 0; col < DIMENSIUNE_TABLA; col++) {
        t[1][col] = 'p';
    }

    // Randurile 2-5: Casute goale
    for (int linie = 2; linie < 6; linie++) {
        for (int col = 0; col < DIMENSIUNE_TABLA; col++) {
            t[linie][col] = ' ';
        }
    }

    // Randul 6: Pionii jucatorului Alb
    for (int col = 0; col < DIMENSIUNE_TABLA; col++) {
        t[6][col] = 'P';
    }

    // Randul 7: Piesele grele ale jucatorului Alb
    t[7][0] = 'R'; t[7][1] = 'N'; t[7][2] = 'B'; t[7][3] = 'Q';
    t[7][4] = 'K'; t[7][5] = 'B'; t[7][6] = 'N'; t[7][7] = 'R';
}

void afisare_tabla(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA]) {
    printf("\n");
    for (int linie = 0; linie < DIMENSIUNE_TABLA; linie++) {
        // Afisam coordonatele pe verticala (8 la 1)
        printf("%d | ", 8 - linie);

        for (int col = 0; col < DIMENSIUNE_TABLA; col++) {
            printf("%c ", t[linie][col]);
        }
        printf("\n");
    }
    // Linia de despartire
    printf("  -----------------\n");
    // Afisam coordonatele pe orizontala (a la h)
    printf("    a b c d e f g h\n\n");
}

void clonare_tabla(char sursa[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], char destinatie[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA]) {
    for (int linie = 0; linie < DIMENSIUNE_TABLA; linie++) {
        for (int col = 0; col < DIMENSIUNE_TABLA; col++) {
            destinatie[linie][col] = sursa[linie][col];
        }
    }
}
