#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define Longueur_Max_Message 1024

char message[Longueur_Max_Message];
 pid_t pid_client = 0;
int message_complet = 0;
pid_t server_pid = 0;

// Fonction de gestion des signaux
void handle_signal(int signum) {
    if (signum == SIGUSR1 || signum == SIGUSR2) {
        printf("Message reçu par le serveur du client %d\n", pid_client);
    }
}

int main() {
    // Afficher le PID du serveur au démarrage
    server_pid = getpid();
    printf("Serveur PID : %d\n", server_pid);

    
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Erreur lors de la configuration de SIGUSR1");
        exit(1);
    }

    
    while (1) {
        // Attente de réception de messages
        printf("Attente de messages...\n");

        
        // Exemple : lire un message de la console
        fgets(message, sizeof(message), stdin);

        // Afficher le message reçu
        printf("Message reçu : %s\n", message);

       
        if (strlen(message) > Longueur_Max_Message) {
            perror("Message trop long");
        } else {
            
            // Obtenir l'horodatage actuel
            char timestamp[64];
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            strftime(timestamp, sizeof(timestamp), "[%H:%M:%S - %d/%m/%Y]", tm_info);

            // Ouvrir le fichier de logs en mode ajout
            FILE *log_file = fopen("conversation.logs", "a+");
            if (log_file != NULL) {
                // Écrire le message dans le fichier de logs
                fprintf(log_file, "[%s] - [Client %u] %s\n", timestamp, pid_client, message);
                fclose(log_file);
            } else {
                perror("Erreur lors de l'ouverture ou de la création du fichier de logs");
            }

            // Afficher le message dans la console
            printf("%s - [Client %u] %s\n", timestamp, pid_client, message);

            // Réinitialiser les variables pour le prochain message
            memset(message, 0, Longueur_Max_Message);
            pid_client = 0;
            message_complet = 0;
        }
        usleep(100000); // Attente de 100 ms avant le prochain signal
    }

    return 0;
}
