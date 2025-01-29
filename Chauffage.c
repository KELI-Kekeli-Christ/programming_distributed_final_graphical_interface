#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SystemeCentral.h"
#include "Chauffage.h"

int main(int argc, const char *argv[]) {
    if (argc < 6) {
        perror("Chauffage lancé avec un nombre d'arguments insuffisants.\n"
               "Erreur dans les arguments \n"
               "Usage: $ ./Chauffage groupeMulticast portUDP adresseServeurTcp portTcp NomPiece ");
        exit(EXIT_FAILURE);
    }

    const char *multicast_group = argv[1];
    const char *serveur_tcp_addr = argv[3];
    const char *piece = argv[5];
    unsigned int port_udp = strtol(argv[2], NULL, 10);
    unsigned int port_tcp = strtol(argv[4], NULL, 10);

    printf("-------------- Chauffage --------------------------------------------- \n");
    printf("-------------- Piece: %s ---------------------------------- \n", piece);
    printf("-------------- Groupe Multicast : %s, Port UDP : %d ------------------------------- \n", multicast_group, port_udp);
    printf("-------------- Systeme Central [Serveur CommunicationTemperature] : %s, Port TCP : %d ------------------------------------- \n \n", serveur_tcp_addr, port_tcp);

    int socket_udp, socket_tcp;
    struct sockaddr_in multicastAddr = configAdrr(port_udp);

    if ((socket_udp = configurerSocketUDP(multicast_group, port_udp, multicastAddr)) != -1) {
        MessageTemperature chauffage_initiale;
        memcpy(chauffage_initiale.piece, piece, sizeof(chauffage_initiale.piece));
        chauffage_initiale.valeur = 0;
        chauffage_initiale.type = CHAUFFER;

        while (1) {
            printf("En attente de connexion au système central [Serveur CommunicationTemperature] \n");

            int count = 0;
            do {
                socket_tcp = configuerSocketTCP(serveur_tcp_addr, port_tcp);
                if (count != 0) {
                    printf("... \n");
                }
                sleep(1);
                count++;
            } while (socket_tcp == -1);

            printf("\nConnexion établie avec le système central [Serveur CommunicationTemperature en ligne] \n");

            while (1) {
                if (ecrireSocket(socket_tcp, socket_udp, (struct sockaddr *)&multicastAddr, &chauffage_initiale) == -2) {
                    perror("Connexion perdue avec le système central ");
                    break;
                }

                printf("Chauffage pour la pièce %s : Etat = ", chauffage_initiale.piece);
                if (chauffage_initiale.valeur == 0) {
                    printf("[ETEINT], (puissance = 0) \n");
                } else {
                    printf("[ALLUMER], puissance = %d \n", chauffage_initiale.valeur);
                }

                sleep(1);
            }
        }
    }

    perror("Echec du lancement de Chauffage.");
    exit(EXIT_FAILURE);
}