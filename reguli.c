#include "reguli.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

bool rege_alb_mutat = false;
bool tura_alba_stanga_mutata = false;
bool tura_alba_dreapta_mutata = false;

bool rege_negru_mutat = false;
bool tura_neagra_stanga_mutata = false;
bool tura_neagra_dreapta_mutata = false;

void initializare_stare_reguli() {
    rege_alb_mutat = false; tura_alba_stanga_mutata = false; tura_alba_dreapta_mutata = false;
    rege_negru_mutat = false; tura_neagra_stanga_mutata = false; tura_neagra_dreapta_mutata = false;
}

bool este_piesa_jucatorului(char piesa, Jucator jucator) {
    if (piesa == ' ') return false;
    if (jucator == ALB) return isupper(piesa);
    return islower(piesa);
}

bool mutare_pseudo_valida(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Pozitie sursa, Pozitie dest, Jucator jucator) {
    char piesa = t[sursa.linie][sursa.coloana];
    char destinatie = t[dest.linie][dest.coloana];

    if (!este_piesa_jucatorului(piesa, jucator)) return false;
    if (este_piesa_jucatorului(destinatie, jucator)) return false; // Traseul se opreste pe piesa proprie, invalid

    int d_linie = dest.linie - sursa.linie;
    int d_col = dest.coloana - sursa.coloana;
    char tip = tolower(piesa);

    // PION
    if (tip == 'p') {
        int directie = (jucator == ALB) ? -1 : 1;
        int linie_start = (jucator == ALB) ? 6 : 1;

        // Inaintare
        if (d_col == 0) {
            if (d_linie == directie && destinatie == ' ') return true;
            // Mutare de doua patratele
            if (sursa.linie == linie_start && d_linie == 2 * directie && destinatie == ' ' && t[sursa.linie + directie][sursa.coloana] == ' ') 
                return true;
        }
        // Capturare
        else if (abs(d_col) == 1 && d_linie == directie) {
            if (destinatie != ' ' && !este_piesa_jucatorului(destinatie, jucator)) return true;
        }
        return false;
    }

    // TURA (Turn)
    if (tip == 'r' || tip == 'q') {
        if (d_linie == 0 || d_col == 0) {
            int pas_l = (d_linie == 0) ? 0 : ((d_linie > 0) ? 1 : -1);
            int pas_c = (d_col == 0) ? 0 : ((d_col > 0) ? 1 : -1);
            int cx = sursa.linie + pas_l;
            int cy = sursa.coloana + pas_c;
            while (cx != dest.linie || cy != dest.coloana) {
                if (t[cx][cy] != ' ') return false; // Coliziune
                cx += pas_l; cy += pas_c;
            }
            if (tip == 'r') return true; // Daca este Regină, merge si la linia de diagonala mai jos
        }
    }

    // NEBUN
    if (tip == 'b' || tip == 'q') {
        if (abs(d_linie) == abs(d_col) && d_linie != 0) {
            int pas_l = (d_linie > 0) ? 1 : -1;
            int pas_c = (d_col > 0) ? 1 : -1;
            int cx = sursa.linie + pas_l;
            int cy = sursa.coloana + pas_c;
            while (cx != dest.linie || cy != dest.coloana) {
                if (t[cx][cy] != ' ') return false; // Coliziune
                cx += pas_l; cy += pas_c;
            }
            return true;
        }
    }

    // CAL
    if (tip == 'n') {
        if ((abs(d_linie) == 2 && abs(d_col) == 1) || (abs(d_linie) == 1 && abs(d_col) == 2)) return true;
        return false;
    }

    // REGE
    if (tip == 'k') {
        if (abs(d_linie) <= 1 && abs(d_col) <= 1) return true;
        
        // Logica pentru Rocada
        if (abs(d_col) == 2 && d_linie == 0) {
            if (jucator == ALB && sursa.linie == 7 && sursa.coloana == 4 && !rege_alb_mutat) {
                if (dest.coloana == 6 && !tura_alba_dreapta_mutata && t[7][5] == ' ' && t[7][6] == ' ') return true;
                if (dest.coloana == 2 && !tura_alba_stanga_mutata && t[7][1] == ' ' && t[7][2] == ' ' && t[7][3] == ' ') return true;
            }
            if (jucator == NEGRU && sursa.linie == 0 && sursa.coloana == 4 && !rege_negru_mutat) {
                if (dest.coloana == 6 && !tura_neagra_dreapta_mutata && t[0][5] == ' ' && t[0][6] == ' ') return true;
                if (dest.coloana == 2 && !tura_neagra_stanga_mutata && t[0][1] == ' ' && t[0][2] == ' ' && t[0][3] == ' ') return true;
            }
        }
    }

    return false;
}

bool este_regele_in_sah(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator jucator) {
    Pozitie rege_poz = {-1, -1};
    char rege_simbol = (jucator == ALB) ? 'K' : 'k';
    
    // Găsește regele
    for (int i = 0; i < DIMENSIUNE_TABLA; i++) {
        for (int j = 0; j < DIMENSIUNE_TABLA; j++) {
            if (t[i][j] == rege_simbol) {
                rege_poz.linie = i;
                rege_poz.coloana = j;
            }
        }
    }

    if (rege_poz.linie == -1) return false; // Ceva ciudat s-a intamplat

    // Verifică mutările adversarului
    Jucator adversar = (jucator == ALB) ? NEGRU : ALB;
    for (int i = 0; i < DIMENSIUNE_TABLA; i++) {
        for (int j = 0; j < DIMENSIUNE_TABLA; j++) {
            if (este_piesa_jucatorului(t[i][j], adversar)) {
                Pozitie adv_sursa = {i, j};
                if (mutare_pseudo_valida(t, adv_sursa, rege_poz, adversar)) return true;
            }
        }
    }
    return false;
}

bool mutare_valida(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Pozitie sursa, Pozitie dest, Jucator jucator, Mutare* detalii_mutare) {
    if (!mutare_pseudo_valida(t, sursa, dest, jucator)) {
        printf("Mutare invalidă pentru această piesă, sau traseul este blocat de altă piesă.\n");
        return false;
    }

    char tabla_clona[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA];
    clonare_tabla(t, tabla_clona);

    detalii_mutare->sursa = sursa;
    detalii_mutare->destinatie = dest;
    detalii_mutare->piesa_mutata = tabla_clona[sursa.linie][sursa.coloana];
    detalii_mutare->piesa_capturata = tabla_clona[dest.linie][dest.coloana];
    detalii_mutare->promovare = ' ';

    // Aplică mutarea în vizorul clonei
    tabla_clona[dest.linie][dest.coloana] = tabla_clona[sursa.linie][sursa.coloana];
    tabla_clona[sursa.linie][sursa.coloana] = ' ';

    if (este_regele_in_sah(tabla_clona, jucator)) {
        printf("Mutare imposibilă: Regele ar fi pus sau ar rămâne în șah!\n");
        return false;
    }

    return true;
}

bool verificare_sah_mat(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Jucator jucator) {
    for (int l_sursa = 0; l_sursa < DIMENSIUNE_TABLA; l_sursa++) {
        for (int c_sursa = 0; c_sursa < DIMENSIUNE_TABLA; c_sursa++) {
            if (este_piesa_jucatorului(t[l_sursa][c_sursa], jucator)) {
                Pozitie sursa = {l_sursa, c_sursa};
                for (int l_dest = 0; l_dest < DIMENSIUNE_TABLA; l_dest++) {
                    for (int c_dest = 0; c_dest < DIMENSIUNE_TABLA; c_dest++) {
                        Pozitie dest = {l_dest, c_dest};
                        if (mutare_pseudo_valida(t, sursa, dest, jucator)) {
                            char tabla_clona[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA];
                            clonare_tabla(t, tabla_clona);
                            tabla_clona[dest.linie][dest.coloana] = tabla_clona[sursa.linie][sursa.coloana];
                            tabla_clona[sursa.linie][sursa.coloana] = ' ';
                            if (!este_regele_in_sah(tabla_clona, jucator)) {
                                return false; // Există salvare din Șah
                            }
                        }
                    }
                }
            }
        }
    }
    return true; // Imposibil de scapat
}

void executa_mutare(char t[DIMENSIUNE_TABLA][DIMENSIUNE_TABLA], Mutare m) {
    char piesa = t[m.sursa.linie][m.sursa.coloana];
    
    // Validare steaguri Rocada
    if (piesa == 'K') rege_alb_mutat = true;
    if (piesa == 'k') rege_negru_mutat = true;
    if (piesa == 'R') {
        if (m.sursa.coloana == 0) tura_alba_stanga_mutata = true;
        if (m.sursa.coloana == 7) tura_alba_dreapta_mutata = true;
    }
    if (piesa == 'r') {
        if (m.sursa.coloana == 0) tura_neagra_stanga_mutata = true;
        if (m.sursa.coloana == 7) tura_neagra_dreapta_mutata = true;
    }

    // Efectuare Rocada dacă mutarea Rege sare 2 casute
    if (tolower(piesa) == 'k' && abs(m.destinatie.coloana - m.sursa.coloana) == 2) {
        if (m.destinatie.coloana == 6) { // Mica
            t[m.destinatie.linie][5] = t[m.destinatie.linie][7];
            t[m.destinatie.linie][7] = ' ';
        } else if (m.destinatie.coloana == 2) { // Mare
            t[m.destinatie.linie][3] = t[m.destinatie.linie][0];
            t[m.destinatie.linie][0] = ' ';
        }
    }

    t[m.destinatie.linie][m.destinatie.coloana] = piesa;
    t[m.sursa.linie][m.sursa.coloana] = ' ';

    // Promovarea Pionului (Cea mai simplă logica -> Regină direct)
    if (tolower(piesa) == 'p') {
        if (m.destinatie.linie == 0) {
            t[m.destinatie.linie][m.destinatie.coloana] = 'Q';
            printf("Pionul a fost promovat la Regină (Q)!\n");
        } else if (m.destinatie.linie == 7) {
            t[m.destinatie.linie][m.destinatie.coloana] = 'q';
            printf("Pionul a fost promovat la Regină (q)!\n");
        }
    }
}
