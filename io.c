#include "io.h"
#include <stdio.h>
#include <string.h>

bool parseaza_mutare(char* input, Pozitie* sursa, Pozitie* dest) {
    if (strlen(input) < 5) return false;

    char scol = input[0];
    char slin = input[1];
    char dcol = input[3];
    char dlin = input[4];

    if (scol < 'a' || scol > 'h') return false;
    if (dcol < 'a' || dcol > 'h') return false;
    if (slin < '1' || slin > '8') return false;
    if (dlin < '1' || dlin > '8') return false;

    sursa->coloana = scol - 'a';
    sursa->linie = 8 - (slin - '0');

    dest->coloana = dcol - 'a';
    dest->linie = 8 - (dlin - '0');

    return true;
}

void salveaza_joc(char const* nume_fisier, char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator rand) {
    FILE* f = fopen(nume_fisier, "wb");
    if (!f) {
        printf("Eroare la scrierea fisierului de salvare!\n");
        return;
    }
    fwrite(t, sizeof(char), DIMENSIUNE_TABLA * DIMENSIUNE_TABLA, f);
    fwrite(&rand, sizeof(Jucator), 1, f);
    
    // Salvare stări Rocadă
    fwrite(&rege_alb_mutat, sizeof(bool), 1, f);
    fwrite(&tura_alba_stanga_mutata, sizeof(bool), 1, f);
    fwrite(&tura_alba_dreapta_mutata, sizeof(bool), 1, f);
    
    fwrite(&rege_negru_mutat, sizeof(bool), 1, f);
    fwrite(&tura_neagra_stanga_mutata, sizeof(bool), 1, f);
    fwrite(&tura_neagra_dreapta_mutata, sizeof(bool), 1, f);
    
    fclose(f);
    printf("Joc salvat cu succes în %s!\n", nume_fisier);
}

bool incarca_joc(char const* nume_fisier, char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator* rand) {
    FILE* f = fopen(nume_fisier, "rb");
    if (!f) {
        printf("Nu s-a găsit fisierul de salvare!\n");
        return false;
    }
    
    fread(t, sizeof(char), DIMENSIUNE_TABLA * DIMENSIUNE_TABLA, f);
    fread(rand, sizeof(Jucator), 1, f);
    
    // Încărcare stări Rocadă
    fread(&rege_alb_mutat, sizeof(bool), 1, f);
    fread(&tura_alba_stanga_mutata, sizeof(bool), 1, f);
    fread(&tura_alba_dreapta_mutata, sizeof(bool), 1, f);
    
    fread(&rege_negru_mutat, sizeof(bool), 1, f);
    fread(&tura_neagra_stanga_mutata, sizeof(bool), 1, f);
    fread(&tura_neagra_dreapta_mutata, sizeof(bool), 1, f);

    fclose(f);
    printf("Joc încărcat cu succes din %s!\n", nume_fisier);
    return true;
}
