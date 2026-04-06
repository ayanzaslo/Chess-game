#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabla.h"
#include "reguli.h"
#include "io.h"

int main() {
    char input[256];
    Jucator rand = ALB;
    initializare_stare_reguli();
    initializare_tabla(tabla);

    printf("\n=================================");
    printf("\n=== JOC DE SAH (IN CONSOLA) ===");
    printf("\n=================================\n");
    printf("Comenzi suportate:\n");
    printf("- e2 e4 (pentru a muta de la sursa la destinatie, cu spatiu)\n");
    printf("- save (pentru a salva jocul)\n");
    printf("- load (pentru a incarca jocul)\n");
    printf("- exit (pentru a parasi jocul)\n");
    printf("---------------------------------\n");

    while (true) {
        afisare_tabla(tabla);

        // Verificam starea de Șah și Șah Mat
        if (este_regele_in_sah(tabla, rand)) {
            if (verificare_sah_mat(tabla, rand)) {
                printf("\n=================================\n");
                printf("ȘAH MAT! Jucătorul %s a fost învins!\n", (rand == ALB) ? "NEGRU" : "ALB");
                printf("=================================\n");
                break;
            } else {
                printf("Atenție! Regele tău este în ȘAH!\n");
            }
        } else {
            // Verificam Remiză (fără mutări posibile dar fără a fi în șah)
            if (verificare_sah_mat(tabla, rand)) {
                printf("\n=================================\n");
                printf("REMIZĂ! (Stalemate) Niciun jucător nu poate efectua vreo mutare legală.\n");
                printf("=================================\n");
                break;
            }
        }

        printf("Tura jucătorului %s. Introdu o comandă: ", (rand == ALB) ? "ALB" : "NEGRU");
        
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0; // Eliminam newline pt Windows / Linux
        
        if (strcmp(input, "\0") == 0) continue;

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        } else if (strcmp(input, "save") == 0) {
            salveaza_joc("sah_salvare.bin", tabla, rand);
            continue;
        } else if (strcmp(input, "load") == 0) {
            if (incarca_joc("sah_salvare.bin", tabla, &rand)) {
                continue;
            }
        } else {
            Pozitie sursa, dest;
            if (!parseaza_mutare(input, &sursa, &dest)) {
                printf("Eroare de sintaxă sau coordonate depășite.\nSintaxa corectă (Litere Mici): 'e2 e4'. Coordonate acceptate: litere (a-h), cifre (1-8).\n");
                continue;
            }

            Mutare m;
            // Valideaza mutarea pe baza tablei actuale și a jucătorului
            if (mutare_valida(tabla, sursa, dest, rand, &m)) {
                executa_mutare(tabla, m);
                // Alternăm turele
                rand = (rand == ALB) ? NEGRU : ALB;
            }
        }
    }

    printf("Joc finalizat. La revedere!\n");
    return 0;
}