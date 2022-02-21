#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define MAX_LINE 80 /* the manimum length command */ 


int main(void){

    char in_stream[MAX_LINE]; //for raw user input
    char *args[MAX_LINE/2 +1 ]; // array of character pointers - [MAX_LINE/2 + 1]  is an estimate of how many pointers we'll need
    char *hist[MAX_LINE/2 +1]; //for history | array of character pointers
    int hist_counter = 0; 

    //histindex holds 100 history int entries, recording size of each full command before enter pressed
    int histindex[100];

    int should_run = 1; /*flag to determine when to exit program*/

    /*holds concatenated raw user input streams for history functionality*/
    char hist_stream[100000]; 

    while (should_run >0){
        /*count of parsed prompt arguments, different than hist_counter which is count of input opportunities */
        int arg_counter = 0;

        printf("osh>"); // UX prompt 
        fflush(stdout); //flush the output file
        /*get used input*/
        fgets(in_stream, MAX_LINE, stdin); 
        /*remove the \n from fgets*/
        in_stream[strlen(in_stream) -1] = NULL; 
        /*concat input stream to a history stream*/
        strcat(hist_stream,in_stream);
        /*histindex records char count of the command before enter pressed*/
        histindex[hist_counter] = strlen(in_stream);
        hist_counter++;
        printf("histcount: %d\n",hist_counter);

        /******INPUT STR PARSING*****/
        /*strtok gets chars up to first space, 
        arg[0] points to starting address within in_stream*/
        args[0] = strtok(in_stream, " "); 
        
        /*args are dangling pointers now; only arg[0] points to an address
        loop fwd and parse the rest of the string. we use NULL as arg in strtok
        as strtok remembers the char array it last dealt with so it continues
        from the last null it read. If we put a new string as arg 1, then it
        start over */
        int i = 0;
        while (args[i] != NULL){
            args[++i] = strtok(NULL, " ");
            arg_counter++;
        }
        /*for '&' case*/
        int index = arg_counter - 1;
        /*strcmp - value 4 compares first 4 chars in word*/
        if (strncmp(args[0], "exit",4) == 0){ 
            should_run = should_run - 1;
            break; //for some reason should_run doesnt break loop
        }
        else if (strncmp(args[0], "history",7) == 0){
            int max_list = 5;
            int g;
            /*account for last arg being history, and to align to correct index*/
            g = hist_counter -2;
            while(max_list > 0){
                /*i is the char array index we stop hist_stream at*/
                int i =0;
                /*mod ensures we get all the full words that are in front of the current*/
                int mod;
                /*we sum size of strings in histstream up to desired start point g */
                for (int j = g-1; j > -1; j--){   
                    mod = j % g;
                    i += histindex[mod];
                }                  
                /*output handler*/  
                char out[MAX_LINE]; 
                strncpy(out,hist_stream+i, histindex[g]);
                printf("\n%d - %s\n", max_list, out);
                fflush(stdout); //flush the output file
                memset(&out[0],0,sizeof(out));
                g--;
                max_list--;
            }
            /*clear input stream*/
            memset(&in_stream[0], 0, sizeof(in_stream));
        }           
        else if(strncmp(args[0],"!!",2) == 0){
            if (hist_counter < 2){printf("\nNo commands in history\n");}
            else{
                //adjust for "!!"" command, plus getting the start of the word we want
                int g = hist_counter -2;
                /*i is the char array index we stop hist_stream at*/
                int i =0;
                /*mod ensures we get all the full words that are in front of the current*/
                int mod;
                /*we sum size of strings in char array up to desired start point g */
                for (int j = g-1; j > -1; j--){   
                    mod = j % g;
                    i += histindex[mod];
                }
                /*execvp needs an array of pointers*/  
                char *prev[MAX_LINE/2+1];
                char out[MAX_LINE]; 
                strncpy(out,hist_stream+i, histindex[g]);          

                prev[0] = strtok(out, " ");         
                int w = 0;
                while (prev[w] != NULL){
                    prev[++w] = strtok(NULL, " ");
                }
                pid_t pid = fork();
                if(pid == 0){  //child process
                    execvp(prev[0], prev); //
                }
                else{wait(NULL);}
                memset(&out[0],0,sizeof(out));
            }
        }
        else if (strcmp(args[index],"&") == 0){
            args[index] = '\0';
            pid_t pid = fork();
            if(pid == 0){  //child process
                execvp(args[0], args); //
            }
            else{wait(NULL);}

        } 
        else if (strncmp(args[0],"!",1) == 0){
            /*convert number char to int*/
            int n = *args[1] - '0';
            printf("n val: %d:\n",n);
            if((hist_counter - n) < 1){printf("\nno such command history\n");}
            else{
                int g;
                /*account for last arg being history, and to align to correct index*/
                g = hist_counter - n - 1;
                /*i is the char array index we stop hist_stream at*/
                int i =0;
                /*mod ensures we get all the full words that are in front of the current*/
                int mod;
                /*we sum size of strings in histstream up to desired start point g */
                for (int j = g-1; j > -1; j--){   
                    mod = j % g;
                    i += histindex[mod];
                }                  
                /*output handler*/  
                char out[MAX_LINE]; 
                strncpy(out,hist_stream+i, histindex[g]);

                /*execvp needs an array of pointers*/  
                char *prev[MAX_LINE/2+1];
                prev[0] = strtok(out, " ");         
                int w = 0;
                while (prev[w] != NULL){
                    prev[++w] = strtok(NULL, " ");
                }
                printf("prev[0]: %s\n",prev[0]);
                pid_t pid = fork();
                if(pid == 0){  //child process
                    execvp(prev[0], prev); //
                }
                else{
                    wait(NULL);
                    /*clear input stream*/
                    memset(&out[0],0,sizeof(out));
                    memset(&in_stream[0], 0, sizeof(in_stream));
                    }


            }      
        }
        else {
            pid_t pid = fork();
            if(pid == 0){  //child process

                /* 
                execvp first arg is the command to execute, and then it needs
                it again as the second arg, hence we supply args ref to entire
                array
                */
                execvp(args[0], args); //
            }
            else { wait(NULL); }
        }
    }
    return 0;
}

