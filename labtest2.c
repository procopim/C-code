/* Lab Test 2
Mark Procopio - 400344315
Jerin John - 400244670
*/

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>


sem_t sem;
sem_t sem2;
pthread_mutex_t mutex;

typedef struct threadparam{
    int a1;
    int a2;
    int rv1;
    int rv2;
} parameters;

void *divide(parameters * param);
void *print(void * param);
void *multiply(parameters * param);


int main (int argc, char * argv[]){

    parameters * data = (parameters *) malloc (sizeof(parameters));
    data -> a1 = atoi(argv[1]);
    data -> a2 = atoi(argv[2]);
    data -> rv1 = atoi(argv[1]);
    data -> rv2 = atoi(argv[2]);

    pthread_t t1;
    pthread_t t2;
    pthread_t t3;

    pthread_attr_t attrt1;
    pthread_attr_t attrt2;
    pthread_attr_t attrt3;

    pthread_attr_init(&attrt1);
    pthread_create(&t1, &attrt1, print, NULL);

    pthread_attr_init(&attrt2);
    pthread_create(&t2, &attrt2, divide, data);

    pthread_attr_init(&attrt3);
    pthread_create(&t3, &attrt3, multiply, data);

    if (sem_init(&sem, 0, 0)!= 0){
        printf("\nerror initializing semaphore\n");
    }

    if (sem_init(&sem2, 0, 0)!= 0){
        printf("\nerror initializing semaphore\n");
    }

    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    pthread_join(t3,NULL);

    return 0;

}

void *print(void * param){
    printf("\nI am the first thread!\n");
    sem_post(&sem); //signals to thread 2
    return 0;
}

void *divide(parameters * param){  
    sem_wait(&sem);

    pthread_mutex_lock(&mutex);
    int n1 = param->a1;
    int n2 = param->a2;
    int div1 = (int) (n1 / 5);
    int div2 = (int) (n2 / 10);

    param->rv1 = div1;
    param->rv2 = div2;

    pthread_mutex_unlock(&mutex);

    sem_post(&sem2);
    printf("division returns: %d, %d\n", div1, div2);
    pthread_exit(0);
}

void *multiply(parameters * param){
    sem_wait(&sem2);
    
    pthread_mutex_lock(&mutex);
    int n1 = param -> rv1;
    int n2 = param -> rv2;
    int mul = n1 * n2;
    pthread_mutex_unlock(&mutex);
    
    printf("\nmultiplication returns: %d\n", mul);
    
    pthread_exit(0);
}