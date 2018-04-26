#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[]){
    fd_set fd_read_set, fd_write_set;
    int i, n;
    
    int srv_socket;
    struct sockaddr_un srv_addr;
    
    srv_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, "server.sock");
    
    connect(srv_socket, (const struct sockaddr*)&srv_addr, sizeof(srv_addr));

    char buff[100];
    int read_len;

    while(1){    
        FD_ZERO(&fd_read_set);
        FD_SET(STDIN_FILENO, &fd_read_set);
        FD_SET(srv_socket, &fd_read_set);
        n = select(max(STDIN_FILENO, srv_socket) + 1, &fd_read_set, NULL, NULL, NULL);
        if (n > 0){
            if (FD_ISSET(STDIN_FILENO, &fd_read_set)){
                read_len = read(STDIN_FILENO, buff, sizeof(buff));
                if(read_len > 0){
                    buff[read_len] = '\0';
                    write(srv_socket, buff, sizeof(buff));                    
                }
            }
            if (FD_ISSET(srv_socket, &fd_read_set)){
                read_len = read(srv_socket, buff, sizeof(buff));
                if(read_len > 0){
                    printf(buff);
                }                
	        }        
        }
    }
    return 0;
}