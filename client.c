#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_MESSAGE_LENGTH 1024

// Fonction pour envoyer un caractère par signal au serveur
void sendCharacterBySignal(pid_t server_pid, char character) {
  union sigval value;
  value.sival_int = (int)character;

  // Envoyer le signal SIGUSR1 au serveur avec la valeur du caractère
  if (sigqueue(server_pid, SIGUSR1, value) == -1) {
    perror("Erreur lors de l'envoi du signal");
    exit(1);
  }

  // Attendre un court instant entre chaque envoi de caractère
  usleep(500);
}

// Fonction pour envoyer le PID du client par signal au serveur
void sendPidBySignal(pid_t server_pid, pid_t client_pid) {
  union sigval value;

  // Envoyer chaque octet du PID du client au serveur
  for (int i = 0; i < sizeof(pid_t); ++i) {
    value.sival_int = (int)((client_pid >> (i * 8)) & 0xff);

    // Envoyer le signal SIGUSR1 au serveur avec la valeur de l'octet du PID
    if (sigqueue(server_pid, SIGUSR1, value) == -1) {
      perror("Erreur lors de l'envoi du PID");
      exit(1);
    }

    // Attendre un court instant entre chaque envoi d'octet de PID
    usleep(500);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Utilisation : %s <PID du serveur> <Message>\n", argv[0]);
    return 1;
  }

  // Récupérer le PID du serveur depuis les arguments
  pid_t server_pid = atoi(argv[1]);

  // Récupérer le message à envoyer depuis les arguments
  char *message = argv[2];

  // Vérifier que le message n'est pas trop long
  if (strlen(message) > MAX_MESSAGE_LENGTH) {
    fprintf(stderr, "Erreur : Message trop long.\n");
    return 1;
  }

  // Récupérer le PID du client
  pid_t client_pid = getpid();

  // Envoyer le PID du client au serveur
  sendPidBySignal(server_pid, client_pid);

  // Envoyer chaque caractère du message au serveur
  for (int i = 0; i <= strlen(message); ++i) {
    char character = (i < strlen(message)) ? message[i] : '\0';
    sendCharacterBySignal(server_pid, character);
  }

  printf("Message envoyé au PID du serveur : %d\n", server_pid);

return 0;
}
  