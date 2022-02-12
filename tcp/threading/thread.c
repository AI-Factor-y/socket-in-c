#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
  
// A normal C function that is executed as a thread 
// when its name is specified in pthread_create()

struct Args{

    int arg1;
    int arg2;
};


void *myThreadFun(void *vargp)
{
    sleep(1);
    struct Args *args=(struct Args *)vargp;
    args->arg1+=3;
    printf("Printing GeeksQuiz from Thread %d %d\n",args->arg1,args->arg2);
    return NULL;
}
   
int main()
{
    struct Args args;

    args.arg1=1;
    args.arg2=20;

    pthread_t thread_id;
    printf("Before Thread\n");
    
    pthread_create(&thread_id, NULL, myThreadFun, (void *) &args);
    pthread_join(thread_id, NULL);
    printf("After Thread\n");
    exit(0);
}
