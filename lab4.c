#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 2
#define LIST_SIZE 20

typedef struct {
    int from_index;
    int to_index;
} parameters;

int sum; /* this data is shared by thread(s) */
void *runner(parameters *data); /* threads call this function*/

int list[LIST_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

int main (int argc, char *argv[]){

    sum = 0;

    pthread_t tid1; /* the thread identifier for data1 */
    pthread_attr_t attr1; /* set of thread attributes data1*/

    pthread_t tid2; /* the thread identifier data2*/
    pthread_attr_t attr2; /* set of thread attributes for data2*/

    parameters *data1 = (parameters *)malloc(sizeof(parameters));
    data1->from_index = 0;  
    data1->to_index = (LIST_SIZE/2) - 1;
    parameters *data2 = (parameters *)malloc(sizeof(parameters));
    data2->from_index = LIST_SIZE/2;
    data2->to_index = LIST_SIZE -1;

    /* set the default attributes of the thread */
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    /* create the thread */
    pthread_create(&tid1, &attr1, runner, data1);
    pthread_create(&tid2, &attr2, runner, data2);

    /*wait for the thread to exit */
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    free(data1);
    free(data2);

    printf("sum = %d\n", sum);
}

/* The thread will execute in this function */
void *runner(parameters *data){
    int i = data->from_index;
    int upper = data->to_index;

    for (int j = i; j <= upper; j++){
        sum += list[j];
    }
    pthread_exit(0);
}