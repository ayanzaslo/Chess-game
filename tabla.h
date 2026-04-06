#ifndef TABLA_H
#define TABLA_H

// Dimensiunea standard a tablei de sah
#define DIMENSIUNE_TABLA 8

// Variabila globala pentru tabla de sah
// P, R, N, B, Q, K - Alb
// p, r, n, b, q, k - Negru
// ' ' - Spatiu gol
extern char tabla[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA];

// Structura pentru a retine o pozitie de pe tabla (linia si coloana)
typedef struct {
    int linie;   // 0-7 (corespunde la 8-1)
    int coloana; // 0-7 (corespunde la a-h)
} Pozitie;

// Initiaza tabla cu asezarea de start a pieselor
void initializare_tabla(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA]);

// Afiseaza tabla curenta in consola
void afisare_tabla(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA]);

// Cloneaza o tabla (util pentru a valida daca o mutare te pune in sah)
void clonare_tabla(char sursa[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], char destinatie[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA]);

#endif // TABLA_H
