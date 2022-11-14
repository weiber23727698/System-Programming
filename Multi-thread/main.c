#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>

int r, c, epoch, nthread, ecount;
char *table1, *table2;

void transform(int index){//bout:0~epoch-1
    int x = index / (c+1), y = index % (c+1);
    int alive = 0;
    char *new_board, *tmp_board;
    if(ecount%2 == 0){
        new_board = table2;
        tmp_board = table1;
    }
    else{
        new_board = table1;
        tmp_board = table2;
    }
    if(x>0 && tmp_board[index-c-1]=='O')    alive++;
    if(x<r-1 && tmp_board[index+c+1]=='O')    alive++;
    if(y>0 && tmp_board[index-1]=='O')    alive++;
    if(y<c && tmp_board[index+1]=='O')    alive++;
    if(x>0 && y>0 && tmp_board[index-c-1-1]=='O')   alive++;
    if(x>0 && y<c && tmp_board[index-c-1+1]=='O')   alive++;
    if(x<r-1 && y>0 && tmp_board[index+c+1-1]=='O')   alive++;
    if(x<r-1 && y<c && tmp_board[index+c+1+1]=='O')   alive++;
    if(tmp_board[index] == '.'){
        if(alive == 3)  new_board[index] = 'O';
        else    new_board[index] = '.';
    }
    else if(tmp_board[index] == 'O'){
        if(alive==2 || alive==3) new_board[index] = 'O';
        else    new_board[index] = '.';
    }
}
/*
void multipro(int tag){
    int up = (r*(c+1)-2) / 2;
    for(int j = 0;j <= up;j++){
        int idx = tag + 2*j;
        if(idx <= r*(c+1)-2){
            if(table1[idx]=='\n' || table2[idx]=='\n')
                table1[idx] = '\n', table2[idx] = '\n';
            else{
                //printf("idx is %d\n", idx);
                transform(idx);
            }
        }
    }
}
*/
void *working(void *arg){//check required~~~~~
    int *tag = (int*)arg;
    //printf("tag is %d\n", *tag);
    int up = (r*(c+1)-2) / nthread;
    //printf("up is %d\n", up);
    for(int i = 0;i <= up;i++){
        int idx = *tag + nthread*i;
        if(idx<=r*(c+1)-2){
            if(table1[idx]=='\n' || table2[idx]=='\n')
                table1[idx] = '\n', table2[idx] = '\n';
            else{
                //printf("idx is %d\n", idx);
                transform(idx);
            }
        }
    }
}

int main(int argc, char *argv[]){
    int mode;
    char in[20], out[20];
    if(strcmp("-p", argv[1])==0)    mode = 1;
    else if(strcmp("-t", argv[1])==0)   mode = 2;
    nthread = atoi(argv[2]);
    strcpy(in, argv[3]), strcpy(out, argv[4]);
    FILE *fpin = fopen(in, "r"), *fpout = fopen(out, "w");
    fscanf(fpin, "%d %d %d", &r, &c, &epoch);
    int fdin = fileno(fpin), fdout = fileno(fpout), len = ftell(fpin)+1;
    lseek(fdin, len, SEEK_SET);
    table1 = (char*)mmap(NULL, (c+2)*(r+2)*sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);//table1[r*(c+1)] is useless
    table2 = (char*)mmap(NULL, (c+2)*(r+2)*sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    read(fdin, table1, c*r+1*(r-1));//pending
    if(mode == 2){
        pthread_t tid[100];
        int id[100];
        for(int j = 0;j < epoch;j++){
            ecount = j;
            for(int i = 0;i < nthread;i++){
                id[i] = i;
                pthread_create(&tid[i], NULL, working, (void*)&id[i]);
            }
            void *tret;
	    
            for(int i = 0;i < nthread;i++)
                pthread_join(tid[i], &tret);
	    //pthread_exit(NULL);
        }
    }
    else if(mode == 1){
        int a = 0, b = 1;
	nthread = 2;
        for(int i = 0;i < epoch;i++){
            ecount = i;
            if(fork() == 0){
                working(&a);
                exit(0);
            }
            if(fork() == 0){
                working(&b);
                exit(0);
            }
            else{
                wait(NULL);
                wait(NULL);
            }
        }
    }
    char *final;
    if(epoch%2==0)  final = table1;
    else    final = table2;
    //printf("table1:\n%s\n", table1);
    //printf("table2:\n%s\n", table2);
    write(fdout, final, c*r+1*(r-1));//pending
    fclose(fpin), fclose(fpout);
    return 0;
}
