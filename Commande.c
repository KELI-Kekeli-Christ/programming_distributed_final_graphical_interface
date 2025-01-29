#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include "Commande.h"

void *envoyerChauffage(void *ptr) {
    CommandeChauffage *commande = (CommandeChauffage *)ptr;
    int res_code = envoyerReponseTcp(commande->socket, &commande->temperature);
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

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Erreur : Nombre d'arguments insuffisants.\n");
        fprintf(stderr, "Usage: %s adresseServeurTcp portTcp\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serveur_tcp_addr = argv[1];
    unsigned int port_tcp = strtol(argv[2], NULL, 10);
    if (port_tcp <= 0 || port_tcp > 65535) {
        fprintf(stderr, "Erreur : Port TCP invalide.\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    printf("----------------------- COMMANDE -------------------------------------------------------------------------------- \n");
    printf("-------------- Systeme Central [Serveur GestionConsole] : %s, Port TCP : %d ------------------------------------- \n \n", serveur_tcp_addr, port_tcp);

    int socket_tcp;

    while (1) {
        printf("En attente de connexion au système central [Serveur GestionConsole]...\n");

        int count = 0;
        do {
            socket_tcp = configuerSocketTCP(serveur_tcp_addr, port_tcp);
            if (count != 0) {
                printf("... \n");
            }
            sleep(1);
            count++;
        } while (socket_tcp == -1);

        printf("\nConnexion établie avec le système central [Serveur GestionConsole] \n");

        while (1) {
            printf("Choisissez le mode de gestion : \n");
            char mode[2];
            do {
                printf("Tapez (M) pour Manuel ou Tapez (A) pour Auto \n");
                scanf("%1s", mode);
                printf("#COMMANDE  ");
                if (strcasecmp(mode, "M") != 0 && strcasecmp(mode, "A") != 0) {
                    fprintf(stderr, "Erreur : Entrez un mode valide (M) ou (A).\n");
                }
            } while (strcasecmp(mode, "M") != 0 && strcasecmp(mode, "A") != 0);

            printf("\n");

            int error = 0;

            if (strcasecmp(mode, "M") == 0) {
                printf("Mode Manuel [ACTIVER] \n");

                while (1) {
                    char piece[100];
                    do {
                        printf("Entrez le nom de la pièce : \n");
                        printf("#COMMANDE  ");
                        scanf("%99s", piece);
                        if (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0) {
                            fprintf(stderr, "Erreur : Entrée invalide, veuillez réessayer.\n");
                        }
                    } while (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0);

                    int niveau = choisirNiveau();

                    MessageTemperature chauffage;
                    chauffage.valeur = niveau;
                    chauffage.type = CHAUFFER;
                    strncpy(chauffage.piece, piece, sizeof(chauffage.piece) - 1);
                    chauffage.piece[sizeof(chauffage.piece) - 1] = '\0';

                    CommandeChauffage commande;
                    commande.temperature = chauffage;
                    commande.socket = socket_tcp;

                    u_char test[BUFFER_SIZE];
                    if (recv(socket_tcp, test, BUFFER_SIZE, MSG_DONTWAIT) == 0) {
                        fprintf(stderr, "Erreur : Connexion perdue avec le système central [Serveur GestionConsole].\n");
                        error = -1;
                        break;
                    }

                    pthread_t demande_thread;
                    if (pthread_create(&demande_thread, NULL, envoyerChauffage, (void *)&commande) != 0) {
                        fprintf(stderr, "Erreur : Impossible de créer le thread.\n");
                        continue;
                    }
                    pthread_join(demande_thread, NULL);

                    char choix = continuerDemande();
                    if (strcasecmp(&choix, "N") == 0) {
                        break;
                    }
                }

                printf("Mode Manuel [DESACTIVER] \n");
            } else {
                printf("Mode Auto [ACTIVER] \n");

                while (1) {
                    char piece[100];
                    do {
                        printf("Entrez le nom de la pièce : \n");
                        printf("#COMMANDE  ");
                        scanf("%99s", piece);
                        if (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0) {
                            fprintf(stderr, "Erreur : Entrée invalide, veuillez réessayer.\n");
                        }
                    } while (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0);

                    int temperature = choisirTemperature();

                    u_char information[BUFFER_SIZE];
                    ssize_t octet = recv(socket_tcp, information, BUFFER_SIZE, MSG_DONTWAIT);

                    if (octet > 0) {
                        MessageTemperature *message_temperature = fromBytes(information, BUFFER_SIZE);

                        if (message_temperature != NULL && message_temperature->type == MESURE && strcmp(message_temperature->piece, piece) == 0) {
                            MessageTemperature chauffage;
                            chauffage.type = CHAUFFER;
                            strncpy(chauffage.piece, piece, sizeof(chauffage.piece) - 1);
                            chauffage.piece[sizeof(chauffage.piece) - 1] = '\0';

                            if (message_temperature->valeur < temperature) {
                                chauffage.valeur = rand() % 5 + 1;
                            } else {
                                chauffage.valeur = 0;
                            }

                            CommandeChauffage commande;
                            commande.temperature = chauffage;
                            commande.socket = socket_tcp;

                            pthread_t demande_thread;
                            if (pthread_create(&demande_thread, NULL, envoyerChauffage, (void *)&commande) != 0) {
                                fprintf(stderr, "Erreur : Impossible de créer le thread.\n");
                                continue;
                            }
                            pthread_join(demande_thread, NULL);
                        }

                        free(message_temperature);
                    } else if (octet == 0) {
                        fprintf(stderr, "Erreur : Connexion perdue avec le système central [Serveur GestionConsole].\n");
                        error = -1;
                        break;
                    } else {
                        fprintf(stderr, "Erreur : Impossible de lire le message d'information depuis le système central.\n");
                    }

                    char choix = continuerDemande();
                    if (strcasecmp(&choix, "N") == 0) {
                        break;
                    }
                }

                printf("Mode Auto [DESACTIVER] \n");
            }

            sleep(1);

            if (error == -1) {
                break;
            }
        }

        close(socket_tcp);
        printf("Tentative de reconnexion au système central...\n");
    }

    return 0;
}