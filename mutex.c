#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*Goal: create 6 threads total that deposit and withdraw
from bank account, using mutex locks to solve for critical section 
race conditions */


void *deposit(int *amt);
void *withdraw(int *amt);

#define NUM_THREADS 6

int acctbalance;

/*declare global mutex*/
pthread_mutex_t mutex;

int main (int argc, char *argv[]){

    acctbalance = 0;

    int dep = atoi(argv[1]);
    int wtd = atoi(argv[2]);

    pthread_t transactions[NUM_THREADS];
    pthread_attr_t transattr[NUM_THREADS];

    /*initialize mutex*/
    if (pthread_mutex_init(&mutex, NULL) != 0){
        printf("\n error initializing mutex \n");
    }

    /*threads 0-2 are deposit, take argv[1] as withdraw amt*/
    for (int i  = 0; i < NUM_THREADS/2; i++){
        pthread_attr_init(&transattr[i]);
        pthread_create(&transactions[i], &transattr[i], deposit, &dep);
    };
    /*threads 3-6 are withdraw; take argv[2] as withdraw amt*/
    for (int i  = 3; i < NUM_THREADS; i++){
        pthread_attr_init(&transattr[i]);
        pthread_create(&transactions[i], &transattr[i], withdraw, &wtd);
    };

    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(transactions[i], NULL);
    }

    // /*withdraw threads*/
    // pthread_t wd1; 
    // pthread_attr_t attr_wd1; 
    // pthread_t wd2; 
    // pthread_attr_t attr_wd2; 
    // pthread_t wd3; 
    // pthread_attr_t attr_wd3; 
    // /*deposit threads*/
    // pthread_t dp1; 
    // pthread_attr_t attr_dp1; 
    // pthread_t dp2; 
    // pthread_attr_t attr_dp2; 
    // pthread_t dp3;
    // pthread_attr_t attr_dp3; 

    printf("\n Final amt: %d \n", acctbalance);
    return 0;
}

void *deposit(int *amt){
    pthread_mutex_lock(&mutex);
    acctbalance = (int)(acctbalance + *amt);
    pthread_mutex_unlock(&mutex);
    printf("\n Deposit amt: %d \n", acctbalance);
    pthread_exit(0);
}

void *withdraw(int *amt){
    pthread_mutex_lock(&mutex);
    acctbalance = (int)(acctbalance - *amt);
    pthread_mutex_unlock(&mutex);
    printf("\n Withdraw amt: %d \n", acctbalance);
    pthread_exit(0);
}