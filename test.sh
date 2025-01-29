#!/bin/bash

# Demander les paramètres à l'utilisateur avec zenity
PARAMS=$(zenity --forms --title="Paramètres du système" \
    --text="Entrez les paramètres pour lancer le système" \
    --add-entry="Adresse du serveur" \
    --add-entry="Adresse multicast" \
    --add-entry="Port multicast" \
    --add-entry="Pièce" \
    --add-entry="Port UDP" \
    --add-entry="Port TCP" \
    --add-entry="Nombre maximum de clients")

# Vérifier si l'utilisateur a annulé
if [ $? -ne 0 ]; then
    echo "L'utilisateur a annulé."
    exit 1
fi

# Extraire les paramètres
ADRESSE_SERVEUR=$(echo "$PARAMS" | cut -d'|' -f1)
MULTICAST_ADDR=$(echo "$PARAMS" | cut -d'|' -f2)
PORT_MULTICAST=$(echo "$PARAMS" | cut -d'|' -f3)
PIECE=$(echo "$PARAMS" | cut -d'|' -f4)
PORT_UDP=$(echo "$PARAMS" | cut -d'|' -f5)
PORT_TCP=$(echo "$PARAMS" | cut -d'|' -f6)
MAX_CLIENT=$(echo "$PARAMS" | cut -d'|' -f7)

# Chemin vers les exécutables des programmes
MULTICAST="./Multicast"
COMMANDE="./Commande"
CHAUFFAGE="./Chauffage"
CONSOLE="./Console"
CONSOLE_JAVA="java Console"
SYSTEM_CENTRAL="./GestionConsole"
COMMUNICATION_TEMPERATURE="./CommunicationTemperature"
RMI="java ServeurRMI"
THERMOMETRE="./Thermometre"
AIR="java Air"

# Fonction pour lancer un programme dans un terminal indépendant
lancer_terminal() {
    local titre="$1"
    local commande="$2"
    gnome-terminal --title="$titre" -- bash -c "$commande; exec bash" &
}

# Lancer tous les programmes en parallèle
echo "Lancement des programmes dans des terminaux indépendants..."
lancer_terminal "Air" "$AIR $MULTICAST_ADDR $PORT_MULTICAST $PIECE"
sleep 1
lancer_terminal "Multicast" "$MULTICAST $MULTICAST_ADDR $PORT_MULTICAST"
sleep 1
lancer_terminal "Système Central" "$SYSTEM_CENTRAL $MULTICAST_ADDR $PORT_TCP $PORT_UDP $MAX_CLIENT"
sleep 1
lancer_terminal "Communication Temperature" "$COMMUNICATION_TEMPERATURE $PORT_TCP $MAX_CLIENT"
sleep 1
lancer_terminal "Thermometre" "$THERMOMETRE $MULTICAST_ADDR $PORT_UDP $ADRESSE_SERVEUR $PORT_TCP $PIECE"
sleep 1
lancer_terminal "Chauffage" "$CHAUFFAGE $MULTICAST_ADDR $PORT_UDP $ADRESSE_SERVEUR $PORT_TCP $PIECE"
sleep 1
lancer_terminal "RMI" "$RMI $MULTICAST_ADDR $PORT_MULTICAST"
sleep 1
lancer_terminal "Console" "$CONSOLE $ADRESSE_SERVEUR $PORT_TCP"
sleep 1
lancer_terminal "Console Java" "$CONSOLE_JAVA"
sleep 1
lancer_terminal "Commande" "$COMMANDE $ADRESSE_SERVEUR $PORT_TCP"

echo "Tous les programmes ont été lancés simultanément !"