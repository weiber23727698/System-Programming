#include <memory.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define Min(a, b) (a < b ? a : b)
#define Max(a, b) (a > b ? a : b)

int main(int argc, char **argv) {
    int row = atoi(argv[1]), col = atoi(argv[2]), epoch = atoi(argv[3]);
    srand(time(NULL));
    char*** board = (char***)malloc(sizeof(char**) * row);
    for (int i = 0; i < row; i++) {
        board[i] = (char**)malloc(sizeof(char*) * col);
        for (int j = 0; j < col; j++) {
            board[i][j] = (char*)malloc(sizeof(char) * 2);
            board[i][j][0] = rand() % 2;
        }
    }

    FILE *in = fopen("in.txt", "w"),
         *out = fopen("ans.txt", "w");
    fprintf(in, "%d %d %d\n", row, col, epoch);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            fprintf(in, "%c", ".O"[board[i][j][0]]);
        }
        if (i + 1 != row) fprintf(in, "\n");
    }

    for (int cnt = 0; cnt < epoch; cnt++) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                int num = 0, live = 0;
                for (int i2 = Max(i - 1, 0); i2 < Min(i + 2, row); i2++) {
                    for (int j2 = Max(j - 1, 0); j2 < Min(j + 2, col); j2++) {
                        num += board[i2][j2][cnt % 2];
                    }
                }
                if (num == 3 || board[i][j][cnt % 2] && num == 4)
                    live = 1;
                board[i][j][(cnt + 1) % 2] = live;
            }
        }
    }

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            fprintf(out, "%c", ".O"[board[i][j][epoch%2]]);
        }
        if (i + 1 != row) fprintf(out, "\n");
    }
}