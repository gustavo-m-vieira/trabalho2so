/*
ALUNO Gustavo Miguel de Oliveira Vieira
DRE 119041187
Problema escolhido: FIFO Barber Shop
SO Manjaro
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX 10000 // tamanho maximo da sala de espera
#define true 1

pthread_t *clients;
pthread_t barber;
pthread_t *waitingRoom;

int qtdClients = 0, qtdChairsInWaitingRoom = 0, clientesNaSalaDeEspera = 0, barberIsAwake = 0, clientesFinalizados = 0;
int clienteDaVez;

// parametros da sala de espera
int salaDeEspera[MAX];
int posicaoInsercao = 0;
int posicaoLeitura = 0;


pthread_mutex_t waitingRoomMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t clientGotCut = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberMadeACut = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeUpBarber = PTHREAD_COND_INITIALIZER;


void getInWaitingRoom(int clientId) {
  printf("Entrei na sala de espera - disse %d\n", clientId);
  salaDeEspera[posicaoInsercao] = clientId;
  posicaoInsercao++;
  if (posicaoInsercao == qtdChairsInWaitingRoom) posicaoInsercao = 0;
  clientesNaSalaDeEspera++;
}

int removeClientFromWaitingRoom() {
  int tmp = salaDeEspera[posicaoLeitura];
  posicaoLeitura++;
  if(posicaoLeitura == qtdChairsInWaitingRoom) posicaoLeitura = 0;
  clientesNaSalaDeEspera--;
  return tmp;
}

void getHairCut(int myId) {
  printf("Eu, %d, estou tendo o meu cabelo cortado...\n", myId);
  clientesFinalizados++;
}

void cutHair(int customerId) {
  printf("Estou cortando o cabelo do(a) %d...\n", customerId);
}

void balk(int myId) {
  printf("'Poxa Vida, ta cheio, volto outro dia' - disse %d\n", myId);
  clientesFinalizados++;
}


void *handle_barber() {
  while (clientesFinalizados != qtdClients) {
    pthread_mutex_lock(&waitingRoomMutex);
    pthread_cond_wait(&wakeUpBarber,&waitingRoomMutex);
    barberIsAwake = 1;
    pthread_mutex_unlock(&waitingRoomMutex);

    while (clientesNaSalaDeEspera) {
      pthread_mutex_lock(&waitingRoomMutex);
      clienteDaVez = removeClientFromWaitingRoom();
      pthread_cond_signal(&barberMadeACut);
      cutHair(clienteDaVez);
      pthread_cond_wait(&clientGotCut,&waitingRoomMutex);
      printf("UHULL, deu match\n");
      pthread_mutex_unlock(&waitingRoomMutex);
    }
    printf("Fui Dormir\n\n");
    barberIsAwake = 0;
  }
  printf("Fim dos atendimentos do dia!\n");
  pthread_exit(NULL);
}

void *handler_client(void *id) {  
  int myId = *((int*) id);
  pthread_mutex_lock(&waitingRoomMutex);

  if (barberIsAwake == 0) {
    pthread_cond_signal(&wakeUpBarber);
  }

  if (clientesNaSalaDeEspera == qtdChairsInWaitingRoom) {
    balk(myId);
    pthread_mutex_unlock(&waitingRoomMutex);
  } else {
    getInWaitingRoom(myId);
    pthread_mutex_unlock(&waitingRoomMutex);

    while(true) {
      pthread_mutex_lock(&waitingRoomMutex);
      pthread_cond_wait(&barberMadeACut,&waitingRoomMutex);
      if (clienteDaVez == myId) {
        getHairCut(myId);
       pthread_cond_signal(&clientGotCut);
        pthread_mutex_unlock(&waitingRoomMutex);
        break;
      }
      pthread_mutex_unlock(&waitingRoomMutex);
    }
  }

  pthread_exit(NULL);
}

int main() {
  printf("MY PID %d \n\n\n", getpid());
  printf("Digite o número de clientes: \n");
  scanf("%d", &qtdClients);
  printf("Digite a quantidade de lugares na sala de espera: \n");
  scanf("%d", &qtdChairsInWaitingRoom);

  clients = (pthread_t *)malloc(sizeof(pthread_t) * qtdClients);

  // Creating barber thread
  pthread_create(&barber, NULL, handle_barber, NULL);

  int i;
  for (i = 0; i < qtdClients; i++) {
    int *id = malloc(sizeof(int));
    *id = i;
    pthread_create(&clients[i], NULL, handler_client, id);
  }
  // Waiting for barber to exit
  pthread_join(barber, NULL);

  return 0;
}