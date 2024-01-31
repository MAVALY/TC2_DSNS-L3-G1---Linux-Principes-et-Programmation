#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define Longueur_Max_Message 1024

// Déclaration des variables globales
char message[Longueur_Max_Message]; // declaration d'un tableau de caractère et macro qui represente la longueur du message
int message_position = 0;         // Position actuelle dans le message
unsigned int pid_client = 0;      // PID du client
int Nombre_octet_recu = 0;        // Nombre d'octets reçus pour le PID
int message_complet = 0;          // Indicateur de message complet

// Fonction de gestion des signaux
void handle_signal(int signum, siginfo_t *info, void *context) { // fonction de gestionnaire de signal avec 3 paramètre dont un void qui est un pointeur générique (il peut contenir l'adresse de n'importe quel type de donnée)
  unsigned char received_char = (unsigned char)info->si_value.sival_int; // 

  // Si le PID du client n'a pas encore été reçu, on le récupère
  if (Nombre_octet_recu < sizeof(pid_t)) { // si le nombre d'octet recu est inférieur a la taille de pid_t exécuter la ligne d'en dessous
    pid_client |= ((unsigned int)received_char << (Nombre_octet_recu * 8)); // décaler 8 bit à gauche dans received char qui est une variable non signée ce qui sert a constituer le PID_client
    Nombre_octet_recu++; // incrémenter de 1
  } else if (received_char == '\0') { // si la variable detecte \0 il detecte la fin du message
    message_complet = 1; // Marquer le message comme complet
  } else if (message_position < Longueur_Max_Message - 1) { // Le message nde doit pas etre plus long que la taille max du message
    message[message_position++] = received_char; // Ajouter le caractère received_char a la position dans message et incremente message_position++, c'est utiliser pour construire un message 
  }
}

int main() {
  printf("Serveur PID : %d\n", getpid());

  // Configuration du traitement du signal SIGUSR1
  struct sigaction sa; // Specifier le comportement d'un gestionnaire de signal
  memset(&sa, 0, sizeof(sa)); //& recuperer une adresse que j'initialise à 0
  sa.sa_flags = SA_SIGINFO; // utilisation de version etendue du gestionnaire de signal
  sa.sa_sigaction = &handle_signal; // recevoir des informations détaillé sur le signal grâce a l'utilisation de SA_SIGINFO

  if (sigaction(SIGUSR1, &sa, NULL) == -1) { // NUll est un pointeur sur 0 ,sigaction permet de faire une action avec le signal
    perror("Erreur lors de la configuration de SIGUSR1"); // informe qu'il y'a une erreur lors de la config de SIGUSR1
    exit(1);       // indique une sortie avec erreur // donne la valeur de retour du programme  false = 1
  }

  while (1) {
    if (message_complet) { // si le message est complet execute le code
      // Obtenir l'horodatage actuel
      char timestamp[64]; // declare un tableau timestamp de 64 caractère, il peux contenir que 63 caractere
      time_t now = time(NULL); //creer une variable now de type time_t et lui assigne la valeur actuelle de l'heure en seconde depuis le 1 er janvier 1970
      struct tm *tm_info = localtime(&now); // structure tm pour stocker des info sur l'heure, la date, le mois. *tm_info pointeur vers une structure tm qui contient les composant horaire
      strftime(timestamp, sizeof(timestamp), "[%H:%M:%S - %d/%m/%Y]", tm_info); // utiliser pour formater l'heure et la date en une chaine de caractère

      // Ouvrir le fichier de logs en mode ajout
      FILE *log_file = fopen("conversation.logs", "a+"); // log_file pointeur vers File qui represente un fichier ouvert, fopen sert a ouvrir un fichier
      if (log_file != NULL) { // si le fichier n'est pas null execute les lignes de codes
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
    usleep(100000); // Attente de 100 ms avant le prochain signal
  }

  return 0;
}
