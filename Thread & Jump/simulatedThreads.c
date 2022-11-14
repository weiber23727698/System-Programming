#include "threadtools.h"
/*
有和b09902055、b09902073討論
參考http://www.csl.mtu.edu/cs4411.ck/common/Coroutines.pdf?fbclid=IwAR3TfoekpSyU5VG8YX72DIBiDleVNbvGhpqXTrFHU7sO6wuSf-es93eV7Z0
*/

// Please complete this three functions. You may refer to the macro function defined in "threadtools.h"

// Mountain Climbing
// You are required to solve this function via iterative method, instead of recursion.
void MountainClimbing(int thread_id, int number){
        ThreadInit(thread_id, number);
        Current->x = 1, Current->y = 1;
        if(Current->N==1 || Current->N==0){
                sleep(1);
                printf("Mountain Climbing: %d\n", 1);
                ThreadYield();
        }
        int ans;
        for(Current->i = 2;Current->i <= Current->N;(Current->i)++){
                sleep(1);
                ans = Current->x + Current->y;
                printf("Mountain Climbing: %d\n", ans);
                Current->y = Current->x;
                Current->x = ans;
                ThreadYield();
        }
        //printf("== mountain terminate ==\n");
        ThreadExit();
}

// Reduce Integer
// You are required to solve this function via iterative method, instead of recursion.
void ReduceInteger(int thread_id, int number){
        ThreadInit(thread_id, number);
        Current->x = Current->N, Current->y = 0;
        if(Current->x==1 || Current->x==0){//一定要有operation
                sleep(1);
                printf("Reduce Integer: 0\n");
                ThreadYield();
        }
        while(Current->x != 1){
                sleep(1);
                if((Current->x)%2 == 0)
                        Current->x = Current->x / 2;
                else{
                        if((Current->x)!=3 && (Current->x)%4==3)
                                (Current->x)++;
                        else
                                (Current->x)--;
                }
                (Current->y)++;
                printf("Reduce Integer: %d\n", Current->y);
                ThreadYield();
        }
        //printf("== reduce terminate ==\n");
        ThreadExit();
}

// Operation Count
// You are required to solve this function via iterative method, instead of recursion.
void OperationCount(int thread_id, int number){
        ThreadInit(thread_id, number);
        sleep(1);
        int ans;
        if((Current->N)%2 == 0)
                ans = ((Current->N)/2) * ((Current->N)/2);
        else
                ans = (((Current->N)+1)/2) * ((Current->N)/2);
        printf("Operation Count: %d\n", ans);
        ThreadYield();
        //printf("== count terminate ==\n");
        ThreadExit();
}
