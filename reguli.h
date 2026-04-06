#ifndef REGULI_H
#define REGULI_H

#include "tabla.h"
#include <stdbool.h>

// Definirea celor doi jucatori
typedef enum {
    ALB = 0,
    NEGRU = 1
} Jucator;

// Structura pentru a retine detaliile complete ale unei mutari
typedef struct {
    Pozitie sursa;
    Pozitie destinatie;
    char piesa_mutata;
    char piesa_capturata;
    char promovare;
} Mutare;

// Flag-uri globale pentru a permite rocada
extern bool rege_alb_mutat;
extern bool tura_alba_stanga_mutata;
extern bool tura_alba_dreapta_mutata;

extern bool rege_negru_mutat;
extern bool tura_neagra_stanga_mutata;
extern bool tura_neagra_dreapta_mutata;

// Reseteaza starea jocului (util in caz de inceput de joc sau incarcarea jocului)
void initializare_stare_reguli();

// Valideaza daca o mutare respecta regulile pieselor (fara sa includa logica de sah)
bool mutare_pseudo_valida(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Pozitie sursa, Pozitie dest, Jucator jucator);

// Valideaza daca o mutare este posibila absolut (include clonare tabla si verificarea sa nu fie propriul rege in sah)
bool mutare_valida(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Pozitie sursa, Pozitie dest, Jucator jucator, Mutare* detalii_mutare);

// Verifica in acel moment daca regele jucatorului transmis este in vizorul inamicului
bool este_regele_in_sah(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator jucator);

// Verifica daca exista macar o mutare care poate debloca sah-ul. Daca nu exista (si e in sah) => SAH MAT
bool verificare_sah_mat(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator jucator);

// Aplica pe tabla mutarea validata, inclusiv regulile speciale (Rocada, Promovare Pion)
void executa_mutare(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Mutare m);

// Helper intern
bool este_piesa_jucatorului(char piesa, Jucator jucator);

#endif // REGULI_H
