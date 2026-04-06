#ifndef IO_H
#define IO_H

#include "reguli.h"
#include <stdbool.h>

// Împarte comanda din consolă (ex: "e2 e4") în sursă și destinație
bool parseaza_mutare(char* input, Pozitie* sursa, Pozitie* dest);

// Stochează tabla, flag-urile interne (rocadă) și randul de mutare într-un fișier binar
void salveaza_joc(char const* nume_fisier, char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator rand);

// Încarcă și rescrie variabila tabla pentru a reporni jocul
bool incarca_joc(char const* nume_fisier, char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator* rand);

#endif // IO_H
