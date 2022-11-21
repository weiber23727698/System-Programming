#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
    int host_id, depth, lucky;
    char *buffer2 = NULL;
    for(int i = 1;i < argc-1;i = i+2){
        if(strcmp(argv[i], "-m") == 0)
            host_id = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-d") == 0)
            depth = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-l") == 0)
            lucky = atoi(argv[i+1]);
    }
    if(depth == 0){
        FILE *fifo_0 = fopen("fifo_0.tmp", "w");
        char filename[100];
        sprintf(filename, "fifo_%d.tmp", host_id);
        FILE *fifo_x = fopen(filename, "r");
        int fd1[2], fd2[2], fd3[2], fd4[2];
        int id[8];//player list
        pipe(fd1), pipe(fd2), pipe(fd3), pipe(fd4);
        FILE *to_child1 = fdopen(fd1[1], "w");
        FILE *from_child1 = fdopen(fd2[0], "r");
        FILE *to_child2 = fdopen(fd3[1], "w");
        FILE *from_child2 = fdopen(fd4[0], "r");
        char lucky_number[10], host_number[4];
        sprintf(host_number, "%d", host_id);
        sprintf(lucky_number, "%d", lucky);
        if(fork() == 0){
            dup2(fd1[0], 0);// read from stdin
            dup2(fd2[1], 1);// write to stdout
            close(fd1[0]), close(fd1[1]), close(fd2[0]), close(fd2[1]), close(fd3[0]), close(fd3[1]), close(fd4[0]), close(fd4[1]);;
            execlp("./host", "./host", "-m", host_number, "-d", "1", "-l", lucky_number, NULL);
        }
        if(fork() == 0){
            dup2(fd3[0], 0);// read from stdin
            dup2(fd4[1], 1);// write to stdout
            close(fd1[0]), close(fd1[1]), close(fd2[0]), close(fd2[1]),close(fd3[0]), close(fd3[1]), close(fd4[0]), close(fd4[1]);
            execlp("./host", "./host", "-m", host_number, "-d", "1", "-l", lucky_number, NULL);
        }
        close(fd1[0]), close(fd2[1]), close(fd3[0]), close(fd4[1]);
        int score[13] = {0};
        while(1){
            for(int i = 0;i < 13;i++)
                score[i] = 0;
            fscanf(fifo_x, "%d %d %d %d %d %d %d %d", &id[0], &id[1], &id[2], &id[3], &id[4], &id[5], &id[6], &id[7]);
            fprintf(to_child1, "%d %d %d %d\n", id[0], id[1], id[2], id[3]);
            fflush(to_child1);
            fprintf(to_child2, "%d %d %d %d\n", id[4], id[5], id[6], id[7]);
            fflush(to_child2);
            if(id[0] != -1){
                int left, l_guess, right, r_guess;
                for(int i = 0;i < 10;i++){//for 10 round
                    fscanf(from_child1, "%d %d", &left, &l_guess);
                    fscanf(from_child2, "%d %d", &right, &r_guess);
                    if(abs(l_guess-lucky) <= abs(r_guess-lucky)){
                        score[left] = score[left] + 10;
                    }
                    else
                        score[right] = score[right] + 10;
                }
                fprintf(fifo_0, "%d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n", host_id, id[0], score[id[0]], id[1], score[id[1]], id[2], score[id[2]], id[3], score[id[3]], id[4], score[id[4]], id[5], score[id[5]], id[6], score[id[6]], id[7], score[id[7]]);
                fflush(fifo_0);
            }
            if(id[0] == -1)
                break;
        }
        if(id[0] == -1){
            while(wait(NULL) > 0);
            fclose(to_child1);
            fclose(to_child2);
            fclose(from_child1);
            fclose(from_child2);
            fclose(fifo_0);
            fclose(fifo_x);
        }
    }
    else if(depth == 1){
        int fd5[2], fd6[2], fd7[2], fd8[2];
        pipe(fd5), pipe(fd6), pipe(fd7), pipe(fd8);
        FILE *to_child3 = fdopen(fd5[1], "w");
        FILE *from_child3 = fdopen(fd6[0], "r");
        FILE *to_child4 = fdopen(fd7[1], "w");
        FILE *from_child4 = fdopen(fd8[0], "r");
        char lucky_number[10], host_number[4];
        sprintf(host_number, "%d", host_id);
        sprintf(lucky_number, "%d", lucky);
        if(fork() == 0){
            dup2(fd5[0], 0);// read from stdin
            dup2(fd6[1], 1);// write to stdout
            close(fd5[0]), close(fd5[1]), close(fd6[0]), close(fd6[1]),close(fd7[0]), close(fd7[1]), close(fd8[0]), close(fd8[1]);
            execlp("./host", "./host", "-m", host_number, "-d", "2", "-l", lucky_number, NULL);
        }
        if(fork() == 0){
            dup2(fd7[0], 0);// read from stdin
            dup2(fd8[1], 1);// write to stdout
            close(fd5[0]), close(fd5[1]), close(fd6[0]), close(fd6[1]),close(fd7[0]), close(fd7[1]), close(fd8[0]), close(fd8[1]);
            execlp("./host", "./host", "-m", host_number, "-d", "2", "-l", lucky_number, NULL);
        }
        close(fd5[0]), close(fd6[1]), close(fd7[0]), close(fd8[1]);
        int first, second, third, forth;
        while(1){
            scanf("%d %d %d %d", &first, &second, &third, &forth);
            fprintf(to_child3, "%d %d\n", first, second);
            fflush(to_child3);
            fprintf(to_child4, "%d %d\n", third, forth);
            fflush(to_child4);
            if(first != -1){
                int left, l_guess, right, r_guess;
                for(int i = 0;i < 10;i++){//for 10 round
                    fscanf(from_child3, "%d %d", &left, &l_guess);
                    fscanf(from_child4, "%d %d", &right, &r_guess);
                    if(abs(l_guess-lucky) <= abs(r_guess-lucky)){
                        printf("%d %d\n", left, l_guess);
                        fflush(stdout);
                    }
                    else{
                        printf("%d %d\n", right, r_guess);
                        fflush(stdout);
                    }
                }
            }
            if(first == -1)
                break;
        }
        if(first == -1){
            while(wait(NULL) > 0);
            fclose(to_child3);
            fclose(to_child4);
            fclose(from_child3);
            fclose(from_child4);
        }
    }
    else if(depth == 2){
        int player[2] = {0};
        FILE *from_left, *from_right;
        while(1){
            int left_leaf[2], right_leaf[2];
            pipe(left_leaf), pipe(right_leaf);
            from_left = fdopen(left_leaf[0], "r");
            from_right = fdopen(right_leaf[0], "r");
            scanf("%d %d", &player[0], &player[1]);
            if(player[0] == -1)
                break;
            char str1[10], str2[10];
            sprintf(str1, "%d", player[0]);
            sprintf(str2, "%d", player[1]);
            if(fork() == 0){
                dup2(left_leaf[1], 1);
                close(left_leaf[0]), close(left_leaf[1]), close(right_leaf[0]), close(right_leaf[1]);
                execlp("./player", "./player", "-n", str1, NULL);
            }
            if(fork() == 0){
                dup2(right_leaf[1], 1);
                close(left_leaf[0]), close(left_leaf[1]), close(right_leaf[0]), close(right_leaf[1]);
                execlp("./player", "./player", "-n", str2, NULL);
            }
            close(left_leaf[1]), close(right_leaf[1]);
            int left, l_guess, right, r_guess;
            for(int i = 0;i < 10;i++){//for 10 round
                fscanf(from_left, "%d %d", &left, &l_guess);
                fscanf(from_right, "%d %d", &right, &r_guess);
                if(abs(l_guess-lucky) <= abs(r_guess-lucky)){
                    printf("%d %d\n", left, l_guess);
                    fflush(stdout);
                }
                else{
                    printf("%d %d\n", right, r_guess);
                    fflush(stdout);
                }
            }
            while(wait(NULL) > 0);
        }
        if(player[0] == -1){
            fclose(from_left);
            fclose(from_right);
        }
    }
    return 0;
}
