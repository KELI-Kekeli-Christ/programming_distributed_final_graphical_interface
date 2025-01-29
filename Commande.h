#include "MessageTemperature.h"
#include "SystemeCentral.h"
#include "MaitreSocket.h"

#ifndef COMMANDE_H
#define COMMANDE_H



#define BUFFER_SIZE 128

struct CommandeChauffage {
    MessageTemperature temperature;
    int socket;
};

typedef struct CommandeChauffage CommandeChauffage;

void *envoyerChauffage(void *ptr);
int choisirNiveau();
int choisirTemperature();
char continuerDemande();


void *envoyerChauffage(void *ptr) {
    CommandeChauffage *commandeChauffage = (CommandeChauffage *)ptr;

    int res_code = envoyerReponseTcp(commandeChauffage->socket, &commandeChauffage->temperature);

    pthread_exit((void *)(intptr_t)res_code);
}

int choisirNiveau() {
    int niveau;

    do {
        printf("Entrez le niveau de chauffage [0 - 5] : \n");
        printf("#COMMANDE  ");
        scanf("%d", &niveau);
        if (niveau < 0 || niveau > 5) {
            fprintf(stderr, "Erreur : Entrez un niveau de chauffage valide compris entre 0 et 5.\n");
        }
    } while (niveau < 0 || niveau > 5);

    return niveau;
}

int choisirTemperature() {
    int temp;

    do {
        printf("Entrez la température souhaitée : \n");
        printf("#COMMANDE  ");
        scanf("%d", &temp);
        if (temp <= 0) {
            fprintf(stderr, "Erreur : Entrez une température supérieure à 0.\n");
        }
    } while (temp <= 0);

    return temp;
}

char continuerDemande() {
    char choix[2];

    do {
        printf("Voulez-vous faire une nouvelle demande ? Tapez O pour (Oui) ou Tapez N pour (Non) \n");
        scanf("%1s", choix);
        printf("#COMMANDE  ");

        if (strcasecmp(choix, "O") != 0 && strcasecmp(choix, "N") != 0) {
            fprintf(stderr, "Erreur : Entrez une option valide (O) ou (N).\n");
        }
    } while (strcasecmp(choix, "O") != 0 && strcasecmp(choix, "N") != 0);

    return choix[0];
}

#endif // COMMANDE_H