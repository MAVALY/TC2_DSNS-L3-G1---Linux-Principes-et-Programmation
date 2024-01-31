#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
#define Longueur_Max_Message 1024

// Fonction pour envoyer un caractère par signal au serveur
void EnvoieCharacterparSignal(pid_t pid_server, char character) {
  union sigval value; // création d'une variable nommée value qui contient différentes types de valeur de signaux 
  value.sival_int = (int)character; // assignation de valeur charactere en entier

  // Envoyer le signal SIGUSR1 au serveur avec la valeur du caractère
  if (sigqueue(pid_server, SIGUSR1, value) == -1) { // envoie un signal a un processus avec des donnée supplémentaire si c'est égale a -1 il y'a une erreur
    perror("Erreur lors de l'envoi du signal");
    exit(1); // Le 1 signal un exit avec une erreur
  }

  // Attendre un court instant entre chaque envoi de caractère
  usleep(500);
}

// Fonction pour envoyer le PID du client par signal au serveur
void EnvoiePidparSignal(pid_t pid_server, pid_t pid_client) {
  union sigval value;

  // Envoyer chaque octet du PID du client au serveur
  for (int i = 0; i < sizeof(pid_t); ++i) {
    value.sival_int = (int)((pid_client >> (i * 8)) & 0xff);

    // Envoyer le signal SIGUSR1 au serveur avec la valeur de l'octet du PID
    if (sigqueue(pid_server, SIGUSR1, value) == -1) {
      perror("Erreur lors de l'envoi du PID");
      exit(1);
    }

    // Attendre un court instant entre chaque envoi d'octet de PID
    usleep(500);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) { // Tester si le nombre d'argument est different de 3
    fprintf(stderr, "Utilisation : %s <PID du serveur> <Message>\n", argv[0]);// imprimer un message en indiquant comment il doit etre utiliser 
    return 1;
  }

  // Récupérer le PID du serveur depuis les arguments
  pid_t pid_server = atoi(argv[1]); //atoi convertis une char en int

  // Récupérer le message à envoyer depuis les arguments
  char *message = argv[2]; // pointeur message qui pointe vers la troisieme chaine de caractere passée en argument

  // Vérifier que le message n'est pas trop long
  if (strlen(message) > Longueur_Max_Message) {
    fprintf(stderr, "Erreur : Message trop long.\n");
    return 1;
  }

  // Récupérer le PID du client
  pid_t pid_client = getpid();

  // Envoyer le PID du client au serveur
  EnvoiePidparSignal(pid_server, pid_client);

  // Envoyer chaque caractère du message au serveur
  for (int i = 0; i <= strlen(message); ++i) { // initialisation de i a 0 et tant que i est inferieur ou egale a la longueur de la chaine de caractere la boucle continue et incrmente i de 1
    char character = (i < strlen(message)) ? message[i] : '\0'; // assignation a la variable charactere le i qui represente le caractere de la chaine message sinon elle lui assigne le caractere \0
    EnvoieCharacterparSignal(pid_server, character);
  }

  printf("Message envoyé au PID du serveur : %d\n", pid_server);

return 0;
}
  


