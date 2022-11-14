#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

/*
有和b09902055、b09902073討論
參考http://www.csl.mtu.edu/cs4411.ck/common/Coroutines.pdf?fbclid=IwAR3TfoekpSyU5VG8YX72DIBiDleVNbvGhpqXTrFHU7sO6wuSf-es93eV7Z0
*/

extern int timeslice, switchmode;

typedef struct TCB_NODE *TCB_ptr;
typedef struct TCB_NODE{
    jmp_buf  Environment;
    int      Thread_id;
    TCB_ptr  Next;
    TCB_ptr  Prev;
    int i, N;
    int w, x, y, z;
} TCB;

extern jmp_buf MAIN, SCHEDULER;
extern TCB_ptr Head;//固定
extern TCB_ptr Current;//node of current function
extern TCB_ptr Work;//暫存???
extern sigset_t base_mask, waiting_mask, tstp_mask, alrm_mask;

void sighandler(int signo);
void scheduler();

// Call function in the argument that is passed in
#define ThreadCreate(function, thread_id, number)                           \
{                                                                            \
    if(setjmp(MAIN) == 0){                                                    \
        function(thread_id, number);                                           \
    }                                                                           \
}

// Build up TCB_NODE for each function, insert it into circular linked-list
#define ThreadInit(thread_id, number)                                         \
{                                                                              \
    Work = (TCB_ptr)malloc(sizeof(TCB));                                        \
    Work->Thread_id = thread_id;                                                 \
    if(Head == NULL){                                                             \
        Head = Work;                                                               \
    }                                                                               \
    else{                                                                            \
        Current->Next = Work;                                                         \
        Work->Prev = Current;                                                          \
    }                                                                                   \
    Work->Next = Head;                                                                   \
    Head->Prev = Work;                                                                    \
    Current = Work;                                                                        \
    Current->N = number;                                                                    \
    if(setjmp(Work->Environment) == 0)                                                       \
        longjmp(MAIN, 1);                                                                     \
}

// Call this while a thread is terminated(simply jump back to scheduler)
#define ThreadExit()                                                             \
{                                                                                 \
    longjmp(SCHEDULER, 2);                                                         \
}

// Decided whether to "context switch" based on the switchmode argument passed in main. (catch signal)
//first setjmp and justify mode(since both mode need setjmp and longjmp)
#define ThreadYield()                                                              \
{                                                                                   \
    if(setjmp(Current->Environment) == 0){                                           \
        if(switchmode == 0){                                                          \
            longjmp(SCHEDULER, 1);                                                     \
        }                                                                                   \
        else{                                                                                \
            sigpending(&waiting_mask);                                                        \
            if(sigismember(&waiting_mask, SIGTSTP)){                                           \
                sigprocmask(SIG_UNBLOCK, &tstp_mask, NULL);                                     \
            }                                                                                    \
            else if(sigismember(&waiting_mask, SIGALRM)){                                         \
                sigprocmask(SIG_UNBLOCK, &alrm_mask, NULL);                                        \
            }                                                                                       \
        }                                                                                            \
    }                                                                                                 \
}
