import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.Scanner;

public class Commande {
    public static void main(String[] args) {
        try {
            Registry registry = LocateRegistry.getRegistry("localhost", 1099);
            Services service = (Services) registry.lookup("service");

            Scanner scanner = new Scanner(System.in);

            while (true) {
                System.out.println("Entrez le nom de la pièce : ");
                String piece = scanner.nextLine();

                System.out.println("Entrez le niveau de chauffage [0 - 5] : ");
                int niveau = scanner.nextInt();
                scanner.nextLine(); // Pour consommer la nouvelle ligne

                MessageTemperature msg = new MessageTemperature(niveau, MessageTemperature.CHAUFFER, piece);
                service.centralCall(); // Envoyer la commande au serveur RMI

                System.out.println("Commande envoyée : Pièce = " + piece + ", Niveau = " + niveau);

                System.out.println("Voulez-vous faire une nouvelle demande ? (O/N)");
                String choix = scanner.nextLine();
                if (choix.equalsIgnoreCase("N")) {
                    break;
                }
            }
        } catch (Exception e) {
            System.err.println("Erreur : " + e.getMessage());
        }
    }
}