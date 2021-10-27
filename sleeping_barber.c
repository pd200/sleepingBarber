#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h> 

// Definisco il numero massimo dei thread cliente
#define CLIENTI_MASSIMI 50

// Prototipi delle funzioni
void * cliente(void * num);
void * barbiere(void * );

void randwait(int secondi);

// Definizione dei semafori.

// Questo semaforo serve per limitare il numero di clienti che possono entrare nella sala d'attesa in una sola volta
sem_t stanzaAttesa;

// Questo semaforo garantisce l'accesso mutamente esclusivo alla poltrona del barbiere
sem_t poltronaBarbiere;

//Questo semaforo viene utilizzato per consentire al barbiere di dormire finchè non arriva un cliente
sem_t cuscinoBarbiere;

// Questo semaforo viene usato per far aspettare il cliente fino a quando il barbiere finisce il taglio
sem_t taglio;

// Serve per interrompere il thread barbiere quando ha servito tutti i clienti.
int tuttoFatto = 0;

int main(int argc, char * argv[]) {
  pthread_t btid;
  pthread_t tid[CLIENTI_MASSIMI];
  int i, numClienti, numeroSedie;
  int Numero[CLIENTI_MASSIMI];

  printf("INSERISCI IL NUMERO DI CLIENTI(Max 50) : ");
  scanf("%d", & numClienti);
  printf("INSERISCI IL NUMERO DELLE SEDIE: ");
  scanf("%d", & numeroSedie);

  // Convalida per vedere che il numero inserito dall'utente sia minore dei threads cliente supportati
  if (numClienti > CLIENTI_MASSIMI) {
    printf("IL NUMERO MASSIMO DI CLIENTI E' %d.\n", CLIENTI_MASSIMI);
    exit(-1);
  }

  // Inizializza l'array di numeri.
  for (i = 0; i < CLIENTI_MASSIMI; i++) {
    Numero[i] = i;
  }

  // Inizializzazione dei semafori con i valori iniziali…
  sem_init( & stanzaAttesa, 0, numeroSedie);
  sem_init( & poltronaBarbiere, 0, 1);
  sem_init( & cuscinoBarbiere, 0, 0);
  sem_init( & taglio, 0, 0);

  // Crazione del thread barbiere.
  pthread_create( & btid, NULL, barbiere, NULL);

  // Crezione dei threads cliente.
  for (i = 0; i < numClienti; i++) {
    pthread_create( & tid[i], NULL, cliente, (void * ) & Numero[i]);
    sleep(1);
  }

  // Unisciti a ciascuno dei thread per attendere che finiscano.
  for (i = 0; i < numClienti; i++) {
    pthread_join(tid[i], NULL);
    sleep(1);
  }

  //Quando tutti i threads cliente sono finiti killa il thread barbiere
  tuttoFatto = 1;
  // Sveglia il barbiere.
  sem_post( & cuscinoBarbiere);
  pthread_join(btid, NULL);
}

void * cliente(void * numero) {
  int num = *(int * )numero;
  /*Parti per il negozio e impiega un tempo random per arrivare
  printf("CLIENTE %d : *** E' PARTITO PER IL NEGOZIO.\n", num + 1);
  randwait(2);
  printf("CLIENTE %d : ***E' ARRIVATO AL NEGOZIO.\n", num + 1);*/
  // Aspetta che ci sia spazio per entrare nella sala d'attesa…
  sem_wait( & stanzaAttesa);
  printf("CLIENTE %d : ***E' ENTRATO NELLA SALA D'ATTESA.\n", num + 1);
  // Aspetta che la poltrona del barbiere si liberi.
  sem_wait( & poltronaBarbiere);
  // La poltrona del barbiere è libera quindi non occupare più il posto in sala d'attesa.
  sem_post( & stanzaAttesa);
  // Risveglia il barbiere…
  printf("CLIENTE %d : ***RISVEGLIA IL BARBIERE.\n", num + 1);
  sem_post( & cuscinoBarbiere);
  // Aspetta che il barbiere finisca di tagliarti i capelli.
  sem_wait( & taglio);
  // Lascia libera la poltrona ed esci dal negozio.
  sem_post( & poltronaBarbiere);
  printf("CLIENTE %d : ***LASCIA IL NEGOZIO.\n", num + 1);
}

void * barbiere(void * junk) {
  // Mentre ci sono ancora clienti da servire il thread barbiere è onnisciente e può dire se ci sono ancora clienti sulla strada per il suo negozio
  while (!tuttoFatto) {
    // Dormi fin quando arriva un cliente a svegliarti..
    printf("BARBIERE : ***DORME...\n");
    sem_wait( & cuscinoBarbiere);
    // Salta queste azioni e torna a casa …
    if (!tuttoFatto) {
      // Impiega un tempo random per tagliare i capelli.
      printf("BARBIERE : ***STA TAGLIANDO I CAPELLI\n");
      randwait(2);
      printf("BARBIERE : ***HA FINITO DI TAGLIARE I CAPELLI\n");
      // Rilascia il thread cliente quando hai effettuato il taglio…
      sem_post( & taglio);
    } else {
      printf("BARBIERE : ***PUO' ANDARE A CASA PER OGGI.\n");
    }
  }
}

void randwait(int secondi) {
  int len;
  // Genera un numero random…
  len = (int)((1 * secondi) + 1);
  sleep(len);
}

