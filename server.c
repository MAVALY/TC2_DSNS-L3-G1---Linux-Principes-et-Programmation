#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define Longueur_Max_Message 1024

// Déclaration des variables globales
char message[Longueur_Max_Message];
int message_position = 0;         // Position actuelle dans le message
unsigned int pid_client = 0;      // PID du client
int Nombre_octet_recu = 0;        // Nombre d'octets reçus pour le PID
int message_complet = 0;          // Indicateur de message complet

// Fonction de gestion des signaux
void handle_signal(int signum, siginfo_t *info, void *context) {
  unsigned char received_char = (unsigned char)info->si_value.sival_int;

  // Si le PID du client n'a pas encore été reçu, on le récupère
  if (Nombre_octet_recu < sizeof(pid_t)) {
    pid_client |= ((unsigned int)received_char << (Nombre_octet_recu * 8));
    Nombre_octet_recu++;
  } else if (received_char == '\0') {
    message_complet = 1; // Marquer le message comme complet
  } else if (message_position < Longueur_Max_Message - 1) {
    message[message_position++] = received_char; // Ajouter le caractère au message
  }
}

int main() {
  printf("Serveur PID : %d\n", getpid());

  // Configuration du traitement du signal SIGUSR1
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = &handle_signal;

  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("Erreur lors de la configuration de SIGUSR1");
    exit(1);
  }

  while (1) {
    if (message_complet) {
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
      message_position = 0;
      pid_client = 0;
      Nombre_octet_recu = 0;
      message_complet = 0;
    }
    usleep(100000); // Attente de 100 ms
  }

  return 0;
}
