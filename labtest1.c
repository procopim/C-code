// Lab Test 1 
// Mark Procopio - 400344315

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main()
{
    printf("%d\n", getpid());  
    pid_t pid1 = fork(); 
    if(pid1 == 0){
        printf("%d\n", getpid());        
        pid_t pid2 = fork();
        
        if(pid2 == 0){
            printf("%d\n", getpid());        

            pid_t pid3 = fork();

            if(pid3 == 0){
                printf("%d\n", getpid());
            }
            else { 
                wait(NULL);
                pid_t pid4 = fork();
                if(pid4 == 0){
                    printf("%d\n", getpid());
                }
                else {wait(NULL);}
            }    

        } else {wait(NULL);}
    } 
    else {wait(NULL);}
    return 0;
}