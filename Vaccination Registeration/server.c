#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

char p1[100] = "Preference order for 902000 modified successed, new preference order is AZ > BNT > Moderna.\n";
char p2[100] = "Preference order for 902000 modified successed, new preference order is AZ > Moderna > BNT.\n";
char p3[100] = "Preference order for 902000 modified successed, new preference order is BNT > AZ > Moderna.\n";
char p4[100] = "Preference order for 902000 modified successed, new preference order is BNT > Moderna > AZ.\n";
char p5[100] = "Preference order for 902000 modified successed, new preference order is Moderna > AZ > BNT.\n";
char p6[100] = "Preference order for 902000 modified successed, new preference order is Moderna > BNT > AZ.\n";

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
    int stage;
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

typedef struct {
    int id;          // 902001-902020
    int AZ;          
    int BNT;         
    int Moderna;     
}registerRecord;

int handle_read(request* reqP) {
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
}

int main(int argc, char** argv) {
    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512];
    int buf_len;

    // Initialize server
    init_server((unsigned short) atoi(argv[1]));

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);
    //declare fd
    fd_set readfd, writefd, copyread, copywrite;
    FD_ZERO(&readfd);
    FD_ZERO(&writefd);
    FD_SET(svr.listen_fd, &readfd);
    FILE *fp;
    fp = fopen("registerRecord", "r+");
    setbuf(fp, NULL);//prevent delate write
    registerRecord rR[20];
    //lock
    int isread[20], iswrite[20];
    memset(isread, 0, sizeof(int)*20);
    memset(iswrite, 0, sizeof(int)*20);
    struct flock readlk, writelk, unlk;
    readlk.l_type = F_RDLCK, writelk.l_type = F_WRLCK, unlk.l_type = F_UNLCK;
    readlk.l_whence = writelk.l_whence = unlk.l_whence = SEEK_SET;
    readlk.l_len = writelk.l_len = unlk.l_len = sizeof(registerRecord);
    int sign = 0;

    while (1) {
        // Add IO multiplexing
        memcpy(&copyread, &readfd, sizeof(readfd));
        memcpy(&copywrite, &writefd, sizeof(writefd));
        int ready = select(maxfd, &copyread, &copywrite, NULL, NULL);
        // Check new connection
        if(FD_ISSET(svr.listen_fd, &copyread)){
            ready--;
            clilen = sizeof(cliaddr);
            conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
            if (conn_fd < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;  // try again
                if (errno == ENFILE) {
                    (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                    continue;
                }
                ERR_EXIT("accept");
            }
            FD_SET(conn_fd, &readfd);
            FD_SET(conn_fd, &writefd);
            requestP[conn_fd].stage = 1;
            requestP[conn_fd].wait_for_write = 0;
            requestP[conn_fd].conn_fd = conn_fd;
            strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
            fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
        }
        // handle requests from clients
        for(int i = 0;i < maxfd;i++){
            sign = 0;
            if(i == svr.listen_fd) continue;
            if(ready <= 0) break;
            if(FD_ISSET(i, &copywrite)){
                ready--;
                if(requestP[i].stage == 1){//determine the type of server
                    //printf("stage 1\n");
                    write(requestP[i].conn_fd, "Please enter your id (to check your preference order):\n", 55);
                    requestP[i].stage = 2;
                    #ifdef READ_SERVER
                            requestP[i].wait_for_write = 0;
                    #elif defined WRITE_SERVER
                            requestP[i].wait_for_write = 1;
                    #endif
                }
                else if(requestP[i].stage == 3){//write original preference
                    if(rR[requestP[i].id-902001].AZ == 1 && rR[requestP[i].id-902001].BNT == 3)
                        write(requestP[i].conn_fd, "Your preference order is AZ > Moderna > BNT.\n", 45);
                    else if(rR[requestP[i].id-902001].AZ == 1 && rR[requestP[i].id-902001].Moderna == 3)
                        write(requestP[i].conn_fd, "Your preference order is AZ > BNT > Moderna.\n", 45);
                    else if(rR[requestP[i].id-902001].BNT == 1 && rR[requestP[i].id-902001].AZ == 3)
                        write(requestP[i].conn_fd, "Your preference order is BNT > Moderna > AZ.\n", 45);
                    else if(rR[requestP[i].id-902001].BNT == 1 && rR[requestP[i].id-902001].Moderna == 3)
                        write(requestP[i].conn_fd, "Your preference order is BNT > AZ > Moderna.\n", 45);
                    else if(rR[requestP[i].id-902001].Moderna == 1 && rR[requestP[i].id-902001].AZ == 3)
                        write(requestP[i].conn_fd, "Your preference order is Moderna > BNT > AZ.\n", 45);
                    else if(rR[requestP[i].id-902001].Moderna == 1 && rR[requestP[i].id-902001].BNT == 3)
                        write(requestP[i].conn_fd, "Your preference order is Moderna > AZ > BNT.\n", 45);
                    if(requestP[i].wait_for_write == 0){
                        FD_CLR(i, &writefd);
                        FD_CLR(i, &readfd);
                        close(requestP[i].conn_fd);
                        free_request(&requestP[i]);
                    }
                    if(requestP[i].wait_for_write == 1){// write_server要往 stage 4 前進 // write_server get input
                        requestP[i].stage = 4;
                        //printf("stage 4\n");
                        write(requestP[i].conn_fd, "Please input your preference order respectively(AZ,BNT,Moderna):\n", 65);
                        requestP[i].stage = 5;
                    }
                }
                else if(requestP[i].stage == 6){// invalid id
                    write(requestP[i].conn_fd, "[Error] Operation failed. Please try again.\n", 44);
                    FD_CLR(i, &writefd);
                    FD_CLR(i, &readfd);
                    close(requestP[i].conn_fd);
                    free_request(&requestP[i]);
                }
                else if(requestP[i].stage>=7 && requestP[i].stage<=12){// write new preference
                    if(requestP[i].stage == 7){
                        p1[25] = (requestP[i].id-902000)/10 + '0', p1[26] = (requestP[i].id-902000)%10 + '0';
                        write(requestP[i].conn_fd, p1, 92);
                    }
                    else if(requestP[i].stage == 8){
                        p2[25] = (requestP[i].id-902000)/10 + '0', p2[26] = (requestP[i].id-902000)%10 + '0';
                        write(requestP[i].conn_fd, p2, 92);
                    }
                    else if(requestP[i].stage == 9){
                        p3[25] = (requestP[i].id-902000)/10 + '0', p3[26] = (requestP[i].id-902000)%10 + '0';
                        write(requestP[i].conn_fd, p3, 92);
                    }
                    else if(requestP[i].stage == 10){
                        p4[25] = (requestP[i].id-902000)/10 + '0', p4[26] = (requestP[i].id-902000)%10 + '0';
                        write(requestP[i].conn_fd, p4, 92);
                    }
                    else if(requestP[i].stage == 11){
                        p5[25] = (requestP[i].id-902000)/10 + '0', p5[26] = (requestP[i].id-902000)%10 + '0';
                        write(requestP[i].conn_fd, p5, 92);
                    }
                    else if(requestP[i].stage == 12){
                        p6[25] = (requestP[i].id-902000)/10 + '0', p6[26] = (requestP[i].id-902000)%10 + '0';
                        write(requestP[i].conn_fd, p6, 92);
                    }
                    FD_CLR(i, &writefd);
                    FD_CLR(i, &readfd);
                    close(requestP[i].conn_fd);
                    free_request(&requestP[i]);
                } 
                else if(requestP[i].stage == 13){// invalid preference
                    write(requestP[i].conn_fd, "[Error] Operation failed. Please try again.\n", 44);
                    FD_CLR(i, &writefd);
                    FD_CLR(i, &readfd);
                    close(requestP[i].conn_fd);
                    free_request(&requestP[i]);
                }
                else if(requestP[i].stage == 14){//Locked
                    write(requestP[i].conn_fd, "Locked.\n", 8);
                    FD_CLR(i, &writefd);
                    FD_CLR(i, &readfd);
                    close(requestP[i].conn_fd);
                    free_request(&requestP[i]);
                }
                FD_CLR(i, &writefd);
            }
            if(FD_ISSET(i, &copyread)){
                ready--;
                if(requestP[i].stage == 2){
                    //printf("stage 2\n");
                    handle_read(&requestP[i]);
                    if(requestP[i].buf_len != 6)
                        requestP[i].stage = 6;//invalid id
                    else{
                        int real = atoi(requestP[i].buf);
                        if(real>902020 || real<902001)
                            requestP[i].stage = 6;//invalid id
                        else{
                            requestP[i].id = real;
                            if(requestP[i].wait_for_write == 0){
                                if(isread[requestP[i].id-902001] == 1){
                                    sign = -1;
                                }
                                else{
                                    readlk.l_start = sizeof(registerRecord)*(requestP[i].id-902001);
                                    sign = fcntl(fileno(fp), F_SETLK, &readlk);
                                    if(sign != -1)
                                        isread[requestP[i].id-902001] = 1;
                                }
                            }
                            else if(requestP[i].wait_for_write == 1){   
                                if(iswrite[requestP[i].id-902001] == 1){
                                    sign = -1;
                                }
                                else{
                                    writelk.l_start = sizeof(registerRecord)*(requestP[i].id-902001);
                                    sign = fcntl(fileno(fp), F_SETLK, &writelk);
                                    if(sign != -1)
                                        iswrite[requestP[i].id-902001] = 1;
                                }
                            }
                            if(sign < 0)// locked => end connection
                                requestP[i].stage = 14;
                            else{
                                fseek(fp, sizeof(registerRecord)*(requestP[i].id-902001), SEEK_SET);
                                fread(&rR[requestP[i].id-902001], sizeof(registerRecord), 1, fp);
                                requestP[i].stage = 3;
                                if(requestP[i].wait_for_write == 0){// read server unlock it after read the wanted id
                                    unlk.l_start = sizeof(registerRecord)*(requestP[i].id-902001);
                                    fcntl(fileno(fp), F_SETLK, &unlk);
                                    isread[requestP[i].id-902001] = 0;
                                }
                            }
                        }
                    }           
                }
                else if(requestP[i].stage == 5){
                    handle_read(&requestP[i]);
                    if(requestP[i].buf_len != 5)
                        requestP[i].stage = 13;
                    else if(strcmp(requestP[i].buf, "1 2 3") == 0){
                        rR[requestP[i].id-902001].AZ = 1, rR[requestP[i].id-902001].BNT = 2, rR[requestP[i].id-902001].Moderna = 3;
                        fseek(fp, sizeof(registerRecord)*(requestP[i].id-902001), SEEK_SET);
                        fwrite(&rR[requestP[i].id-902001], sizeof(registerRecord), 1, fp);
                        requestP[i].stage = 7;                        
                    }
                    else if(strcmp(requestP[i].buf, "1 3 2") == 0){
                        rR[requestP[i].id-902001].AZ = 1, rR[requestP[i].id-902001].BNT = 3, rR[requestP[i].id-902001].Moderna = 2;
                        fseek(fp, sizeof(registerRecord)*(requestP[i].id-902001), SEEK_SET);
                        fwrite(&rR[requestP[i].id-902001], sizeof(registerRecord), 1, fp);
                        requestP[i].stage = 8;                        
                    }
                    else if(strcmp(requestP[i].buf, "2 1 3") == 0){
                        rR[requestP[i].id-902001].AZ = 2, rR[requestP[i].id-902001].BNT = 1, rR[requestP[i].id-902001].Moderna = 3;
                        fseek(fp, sizeof(registerRecord)*(requestP[i].id-902001), SEEK_SET);
                        fwrite(&rR[requestP[i].id-902001], sizeof(registerRecord), 1, fp);
                        requestP[i].stage = 9;
                    }
                    else if(strcmp(requestP[requestP[i].id-902001].buf, "2 3 1") == 0){
                        rR[requestP[i].id-902001].AZ = 2, rR[requestP[i].id-902001].BNT = 3, rR[requestP[i].id-902001].Moderna = 1;
                        fseek(fp, sizeof(registerRecord)*(requestP[i].id-902001), SEEK_SET);
                        fwrite(&rR[requestP[i].id-902001], sizeof(registerRecord), 1, fp);
                        requestP[i].stage = 10;
                    }
                    else if(strcmp(requestP[i].buf, "3 1 2") == 0){
                        rR[requestP[i].id-902001].AZ = 3, rR[requestP[i].id-902001].BNT = 1, rR[requestP[i].id-902001].Moderna = 2;
                        fseek(fp, sizeof(registerRecord)*(requestP[i].id-902001), SEEK_SET);
                        fwrite(&rR[requestP[i].id-902001], sizeof(registerRecord), 1, fp);
                        requestP[i].stage = 11;
                    }
                    else if(strcmp(requestP[i].buf, "3 2 1") == 0){
                        rR[requestP[i].id-902001].AZ = 3, rR[requestP[i].id-902001].BNT = 2, rR[requestP[i].id-902001].Moderna = 1;
                        fseek(fp, sizeof(registerRecord)*(requestP[i].id-902001), SEEK_SET);
                        fwrite(&rR[requestP[i].id-902001], sizeof(registerRecord), 1, fp);
                        requestP[i].stage = 12;
                    }
                    else
                        requestP[i].stage = 13;//invalid input release clock
                    // unlock it upon finishing entering the preference
                    unlk.l_start = sizeof(registerRecord)*(requestP[i].id-902001);
                    fcntl(fileno(fp), F_SETLK, &unlk);
                    iswrite[requestP[i].id-902001] = 0;
                }
                FD_SET(i, &writefd);
            }
        }

/*
#ifdef READ_SERVER      
        fprintf(stderr, "%s", requestP[conn_fd].buf);
        sprintf(buf,"%s : %s",accept_read_header,requestP[conn_fd].buf);
        write(requestP[conn_fd].conn_fd, buf, strlen(buf));  

#elif defined WRITE_SERVER
        fprintf(stderr, "%s", requestP[conn_fd].buf);
        sprintf(buf,"%s : %s",accept_write_header,requestP[conn_fd].buf);
        write(requestP[conn_fd].conn_fd, buf, strlen(buf));    
#endif
*/
        /*
        close(requestP[conn_fd].conn_fd);
        free_request(&requestP[conn_fd]);
        */
    }
    free(requestP);
    return 0;
}

// ======================================================================================================
#include <fcntl.h>

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
