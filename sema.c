#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Goal: create 10 threads total; 7 run the deposit func and 3 run 
the withdraw func. We use mutex locks to solve for critical section 
race conditions. 

We use a counting semaphore to maintain conditions:
- no deposit if acctbalance >= $400
- no withdraw if acctbalance <= $100 

So 4 x deposit of $100 = $400, therefore 4 deposit limit in context
deposit waits the semaphore
withdraw signals the semaphore

The mutex locks the writing to acctbalance

compile args for gcc:
gcc -pthread  filename.c
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*runner function declarations*/
void *deposit(int *amt);
void *withdraw(int *amt);

#define NUM_THREADS 10
/*global var representing account balance*/
int acctbalance;

/*declare global semaphore*/
sem_t sem;

/*declare global mutex*/
pthread_mutex_t mutex;

int main (int argc, char *argv[]){

    acctbalance = 0;
    /*initialize semaphore - value=4 provided per above summary*/
    if (sem_init(&sem, 0, 4)!= 0){
        printf("\nerror initializing semaphore");
    }
    
    /*convert char input to int*/
    int dep = atoi(argv[1]);
    /*we fix withdraw to the deposit of $100 for this exercise*/    
    int wtd = atoi(argv[1]);
    
    /*declare array of id and attr types*/
    pthread_t transactions[NUM_THREADS];
    pthread_attr_t transattr[NUM_THREADS];

    /*initialize mutex*/
    if (pthread_mutex_init(&mutex, NULL) != 0){
        printf("\n error initializing mutex \n");
    }

    /*threads 0-6 are deposit, take dep as deposit amt*/
    for (int i  = 0; i < 7 % NUM_THREADS; i++){
        pthread_attr_init(&transattr[i]);
        pthread_create(&transactions[i], &transattr[i], deposit, &dep);
    };
    /*threads 7-9 are withdraw; take argv[2] as withdraw amt*/
    for (int i  = 7; i < NUM_THREADS; i++){
        pthread_attr_init(&transattr[i]);
        pthread_create(&transactions[i], &transattr[i], withdraw, &wtd);
    };
    
    /*ensure parents thread awaits return of all created threads*/
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(transactions[i], NULL);
    }

    printf("\n Final amt: %d \n", acctbalance);
    return 0;
}

void *deposit(int *amt){
    printf("\nexecuting deposit function");
    /*acquire semaphore*/
    sem_wait(&sem);

    if (pthread_mutex_lock(&mutex) != 0){
        printf("\nerror obtaining lock upon deposit\n");
    };

    acctbalance = (int)(acctbalance + *amt);

    if (pthread_mutex_unlock(&mutex) != 0){
        printf("\nerror releasing lock after deposit\n");
    };

    printf("\n Balance after Deposit amt: %d \n", acctbalance);
    pthread_exit(0);
}

void *withdraw(int *amt){
    printf("\nexecuting withdraw function");

    if (pthread_mutex_lock(&mutex) != 0){
        printf("\nerror obtaining lock upon withdraw\n");
    };
    acctbalance = (int)(acctbalance - *amt);
    if (pthread_mutex_unlock(&mutex) != 0){
        printf("\nerror releasing lock after withdraw\n");
    };

    /*signal semaphore*/
    sem_post(&sem);

    printf("\n Balance after Withdraw amt: %d \n", acctbalance);
    pthread_exit(0);
}