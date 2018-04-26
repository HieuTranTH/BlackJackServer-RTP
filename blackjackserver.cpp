#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <vector>
#include <algorithm>    // std::random_shuffle
#include <signal.h>

using namespace std;

#define NUMBER_OF_CARDS 52

static void ctrl_handler(int);

int main(void){
    if (signal(SIGINT, ctrl_handler) == SIG_ERR) {
        fprintf(stderr, "\nCannot set signal handler");
        exit(0);
    }
    
    int fd_server = open("./house_balance.log", O_RDWR | O_CREAT | O_APPEND, 0644);
	
    int srv_socket, client_socket;
    struct sockaddr_un srv_addr;
    
    pid_t pid1, pid2;
    unlink("server.sock");
    srv_socket = socket(AF_UNIX, SOCK_STREAM, 0);
/*    
    int fd_flags;
    fd_flags = fcntl(srv_socket, F_GETFD);  //read current file flags
    fd_flags = fd_flags | O_NONBLOCK;      //add O_NONBLOCK bit
    fcntl(srv_socket, F_SETFD, fd_flags);   //write new flags back
*/    
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, "server.sock");
    
    bind(srv_socket, (const struct sockaddr*)&srv_addr, sizeof(srv_addr));
    
    listen(srv_socket, 5);
    
    while(1){
        client_socket = accept(srv_socket, NULL, 0);
        pid1 = fork();
        if (pid1 == 0){
            pid2= fork();
            
            if (pid2 > 0) exit(0);
            
            srand (time(NULL));

        	char buff[100];

            printf("There is a connection.\r\n");
                        
            lseek(fd_server, 0L, SEEK_SET);
		    read(fd_server, buff, sizeof(buff));
		    int init_balance = atoi(buff);
			int balance = init_balance;
			//printf("%d\r\n", balance);
			
            char name[100];
            int total_chip, bet_chip, number_of_hand;
            vector<int> deck, hand_player[2], hand_house[2];
            
            int index = 0;
            int sum_player = 0, sum_house = 0;
            int number, suit;
            int number_of_ace_player = 0, number_of_ace_house = 0;
            char c;
            int next = 1;
            int blackjack = 0;
                        
            strcpy(buff, "What is the player's name?\r\n");
            write(client_socket, buff, sizeof(buff));                        
            read(client_socket, name, sizeof(name));
            name[strlen(name) - 1] = '\0';
            printf("Player %s is playing...\r\n", name);
            
            char path[100]; 
            sprintf(path, "player_%s.log", name);
            int fd_client = open(path, O_RDWR | O_CREAT | O_APPEND, 0644);
            
            sprintf(buff, "--------------------------------------------------------\r\n", total_chip);
            write(fd_client, buff, strlen(buff));
            
            strcpy(buff, "Start playing...\r\n");
            write(client_socket, buff, sizeof(buff));
            
            strcpy(buff, "How many chips do you want to buy?\r\n");
            write(client_socket, buff, sizeof(buff));                        
            read(client_socket, buff, sizeof(buff));
            total_chip = atoi(buff);
            sprintf(buff, "You bought: %d chips.\r\n", total_chip);
            write(fd_client, buff, strlen(buff));
            
            while(1){
        		deck.clear();
            	//Prepare a new deck
	            for (int i = 0; i < NUMBER_OF_CARDS; ++i) deck.push_back(i);
	
	            //Shuffle the deck
	            random_shuffle ( deck.begin(), deck.end() );
	
			    for (int i = 0; i < NUMBER_OF_CARDS; i++){
	                printf("%d    ", deck[i]);
	            }     
	            printf("\r\n");
	            printf("\r\n");
	            
	            strcpy(buff, "\nA new deck of cards has been prepared.\r\n");
	            write(client_socket, buff, sizeof(buff));
	            index = 0;
					            
	            sprintf(buff, "Your total amount of chips: %d.\r\n", total_chip);
	            write(client_socket, buff, sizeof(buff));
	            sprintf(buff, "New round, your total amount of chips: %d chips.\r\n", total_chip);
	            write(fd_client, buff, strlen(buff));
	            
	            sum_player = 0, sum_house = 0;
	            number_of_ace_player = 0;
	            number_of_ace_house = 0;
	            next = 1;
	            blackjack = 0;
	            hand_player[0].clear();
	            hand_player[1].clear();
	            hand_house[0].clear();
	            hand_house[1].clear();
	            
	            while(1){
		            sprintf(buff, "How many chips do you want to bet?\r\n");
		            write(client_socket, buff, sizeof(buff));                        
		            read(client_socket, buff, sizeof(buff));            
		            bet_chip = atoi(buff);
		            if(bet_chip > total_chip){
		            	sprintf(buff, "Out of money.\r\n");
			            write(client_socket, buff, sizeof(buff));
		            }else {
			            break;	            
		            }
	            }
	            
	            total_chip -= bet_chip;
	            sprintf(buff, "Your total amount of chips: %d.\r\n", total_chip);
	            write(client_socket, buff, sizeof(buff));
	            sprintf(buff, "\nYour bet amount: %d.\r\n", bet_chip);
	            write(client_socket, buff, sizeof(buff));
	            sprintf(buff, "You bet: %d chips.\r\n", bet_chip);
	            write(fd_client, buff, strlen(buff));
	            
	            number = deck[index] / 4;
	            suit = deck[index] % 4;
	            hand_player[0].push_back(number);
	            hand_player[1].push_back(suit);
	            index++;
	            if ((number >= 0) && (number <= 8)){
	                sum_player = sum_player + number + 1;
	            }
	            else if ((number >= 9) && (number <= 12)){
	                sum_player = sum_player + 10;
	            }
	                        
	            number = deck[index] / 4;
	            suit = deck[index] % 4;
	            hand_player[0].push_back(number);
	            hand_player[1].push_back(suit);
	            index++;
	            if ((number >= 0) && (number <= 8)){
	                sum_player = sum_player + number + 1;
	            }
	            else if ((number >= 9) && (number <= 12)){
	                sum_player = sum_player + 10;
	            }            
	            
	            strcpy(buff, "Your hand is: ");
	            for (int i = 0; i < hand_player[0].size(); i++){
	                char card[5];
	                switch(hand_player[0][i]){
	                    case 0:
	                        sprintf(card, "A");
	                        number_of_ace_player++;
	                        break;
	                    case 1:
	                        sprintf(card, "2");
	                        break;
	                    case 2:
	                        sprintf(card, "3");
	                        break;
	                    case 3:
	                        sprintf(card, "4");
	                        break;    
	                    case 4:
	                        sprintf(card, "5");
	                        break;
	                    case 5:
	                        sprintf(card, "6");
	                        break;
	                    case 6:
	                        sprintf(card, "7");
	                        break;
	                    case 7:
	                        sprintf(card, "8");
	                        break;
	                    case 8:
	                        sprintf(card, "9");
	                        break;
	                    case 9:
	                        sprintf(card, "10");
	                        break;
	                    case 10:
	                        sprintf(card, "J");
	                        break;
	                    case 11:
	                        sprintf(card, "Q");
	                        break;
	                    case 12:
	                        sprintf(card, "K");
	                        break;                                                   
	                }
	                switch(hand_player[1][i]){
	                    case 0:
	                        strcat(card, "\u2661 ");
	                        break;
	                    case 1:
	                        strcat(card, "\u2662 ");
	                        break;
	                    case 2:
	                        strcat(card, "\u2663 ");
	                        break;
	                    case 3:
	                        strcat(card, "\u2660 ");
	                        break;                                           
	                }              
	                strcat(buff, card);
	            }
	            strcat(buff, "\r\n");
	            write(client_socket, buff, sizeof(buff));
	            
	            number = deck[index] / 4;
	            suit = deck[index] % 4;
	            hand_house[0].push_back(number);
	            hand_house[1].push_back(suit);
	            index++;
	            if ((number >= 0) && (number <= 8)){
	                sum_house = sum_house + number + 1;
	            }
	            else if ((number >= 9) && (number <= 12)){
	                sum_house = sum_house + 10;
	            }
	            	            
	            strcpy(buff, "The house's hand is: ");
	            for (int i = 0; i < hand_house[0].size(); i++){
	                char card[5];
	                switch(hand_house[0][i]){
	                    case 0:
	                        sprintf(card, "A");
	                        number_of_ace_house++;
	                        break;
	                    case 1:
	                        sprintf(card, "2");
	                        break;
	                    case 2:
	                        sprintf(card, "3");
	                        break;
	                    case 3:
	                        sprintf(card, "4");
	                        break;    
	                    case 4:
	                        sprintf(card, "5");
	                        break;
	                    case 5:
	                        sprintf(card, "6");
	                        break;
	                    case 6:
	                        sprintf(card, "7");
	                        break;
	                    case 7:
	                        sprintf(card, "8");
	                        break;
	                    case 8:
	                        sprintf(card, "9");
	                        break;
	                    case 9:
	                        sprintf(card, "10");
	                        break;
	                    case 10:
	                        sprintf(card, "J");
	                        break;
	                    case 11:
	                        sprintf(card, "Q");
	                        break;
	                    case 12:
	                        sprintf(card, "K");
	                        break;                                                   
	                }
	                switch(hand_house[1][i]){
	                    case 0:
	                        strcat(card, "\u2661 ");
	                        break;
	                    case 1:
	                        strcat(card, "\u2662 ");
	                        break;
	                    case 2:
	                        strcat(card, "\u2663 ");
	                        break;
	                    case 3:
	                        strcat(card, "\u2660 ");
	                        break;                                           
	                }              
	                strcat(buff, card);
	            }
	            strcat(buff, "\r\n\r\n");
	            write(client_socket, buff, sizeof(buff));
	            
	            if (number_of_ace_player > 0){
	                if ((sum_player + 10) == 21){
	                	sum_player += 10;
	                	number_of_ace_player = 0;
	                	blackjack = 1;
	                	next = 0;
			        }			        
	            }
	            
	            while((next) && (sum_player < 21)){
	                strcpy(buff, "Do you want to hit (y/n)?\r\n");
	                write(client_socket, buff, sizeof(buff));                        
	                read(client_socket, buff, sizeof(buff));
	                if (buff[0] == 'y'){
	                    number = deck[index] / 4;
	                    suit = deck[index] % 4;
	                    hand_player[0].push_back(number);
	                    hand_player[1].push_back(suit);
	                    index++;
	                    if ((number >= 0) && (number <= 8)){
	                        sum_player = sum_player + number + 1;
	                    }else if ((number >= 9) && (number <= 12)){
	                        sum_player = sum_player + 10;
	                    }
	                                        
	                    strcpy(buff, "Your hand is: ");
			            for (int i = 0; i < hand_player[0].size(); i++){
			                char card[5];
			                switch(hand_player[0][i]){
			                    case 0:
			                        sprintf(card, "A");
			                        number_of_ace_player++;
			                        break;
			                    case 1:
			                        sprintf(card, "2");
			                        break;
			                    case 2:
			                        sprintf(card, "3");
			                        break;
			                    case 3:
			                        sprintf(card, "4");
			                        break;    
			                    case 4:
			                        sprintf(card, "5");
			                        break;
			                    case 5:
			                        sprintf(card, "6");
			                        break;
			                    case 6:
			                        sprintf(card, "7");
			                        break;
			                    case 7:
			                        sprintf(card, "8");
			                        break;
			                    case 8:
			                        sprintf(card, "9");
			                        break;
			                    case 9:
			                        sprintf(card, "10");
			                        break;
			                    case 10:
			                        sprintf(card, "J");
			                        break;
			                    case 11:
			                        sprintf(card, "Q");
			                        break;
			                    case 12:
			                        sprintf(card, "K");
			                        break;                                                   
			                }
			                switch(hand_player[1][i]){
			                    case 0:
			                        strcat(card, "\u2661 ");
			                        break;
			                    case 1:
			                        strcat(card, "\u2662 ");
			                        break;
			                    case 2:
			                        strcat(card, "\u2663 ");
			                        break;
			                    case 3:
			                        strcat(card, "\u2660 ");
			                        break;                                           
			                }              
			                strcat(buff, card);
			            }
			            strcat(buff, "\r\n");
			            write(client_socket, buff, sizeof(buff));
			            if (number_of_ace_player > 0){
	                        if ((sum_player + 10) == 21){
			                	sum_player += 10;
			                	number_of_ace_player = 0;
			                	next = 0;
					        }					        
	                    }	                	
	                }else if (buff[0] == 'n'){
	                    if (number_of_ace_player > 0){
	                        if ((sum_player + 10) <= 21){
			                	sum_player += 10;
			                	number_of_ace_player = 0;
					        }
	                    }
	                	next = 0;
	                }else {
	                    strcpy(buff, "Wrong input.\r\n");
	                    write(client_socket, buff, sizeof(buff));
	                    continue;
	                } 
		                
	//	            sprintf(buff, "Your points: %d\r\n", sum_player);
	//				write(client_socket, buff, sizeof(buff));          
	            }
	            
	            strcpy(buff, "Your hand is: ");
	            for (int i = 0; i < hand_player[0].size(); i++){
	                char card[5];
	                switch(hand_player[0][i]){
	                    case 0:
	                        sprintf(card, "A");
	                        break;
	                    case 1:
	                        sprintf(card, "2");
	                        break;
	                    case 2:
	                        sprintf(card, "3");
	                        break;
	                    case 3:
	                        sprintf(card, "4");
	                        break;    
	                    case 4:
	                        sprintf(card, "5");
	                        break;
	                    case 5:
	                        sprintf(card, "6");
	                        break;
	                    case 6:
	                        sprintf(card, "7");
	                        break;
	                    case 7:
	                        sprintf(card, "8");
	                        break;
	                    case 8:
	                        sprintf(card, "9");
	                        break;
	                    case 9:
	                        sprintf(card, "10");
	                        break;
	                    case 10:
	                        sprintf(card, "J");
	                        break;
	                    case 11:
	                        sprintf(card, "Q");
	                        break;
	                    case 12:
	                        sprintf(card, "K");
	                        break;                                                   
	                }
	                switch(hand_player[1][i]){
	                    case 0:
	                        strcat(card, "\u2661 ");
	                        break;
	                    case 1:
	                        strcat(card, "\u2662 ");
	                        break;
	                    case 2:
	                        strcat(card, "\u2663 ");
	                        break;
	                    case 3:
	                        strcat(card, "\u2660 ");
	                        break;                                           
	                }              
	                strcat(buff, card);
	            }
	            strcat(buff, "\r\n");
	            write(fd_client, buff, strlen(buff));
	            
	            sprintf(buff, "Your points: %d\r\n", sum_player);
				write(client_socket, buff, sizeof(buff));
            
	            if (sum_player > 21){
	            	strcpy(buff, "YOU LOSE.\r\n");
	                write(client_socket, buff, sizeof(buff));
	            	balance += bet_chip;
	            	sprintf(buff, "You lose: %d.\r\n", bet_chip);
		            write(fd_client, buff, strlen(buff));
	            }else if(sum_player == 21){
	                strcpy(buff, "YOU WIN.\r\n");
	                write(client_socket, buff, sizeof(buff));
	                if (blackjack = 1){
	                	total_chip += bet_chip*2.5;
	                	balance -= bet_chip*1.5;
	                	sprintf(buff, "You win: %d.\r\n", bet_chip*1.5);
			            write(fd_client, buff, strlen(buff));
	                }else {
	                	total_chip += bet_chip*2;
	                	balance -= bet_chip;
	                	sprintf(buff, "You win: %d.\r\n", bet_chip);
			            write(fd_client, buff, strlen(buff));
	                }
	            }else{	            
		            number = deck[index] / 4;
		            suit = deck[index] % 4;
		            hand_house[0].push_back(number);
		            hand_house[1].push_back(suit);
		            index++;
		            if ((number >= 0) && (number <= 8)){
		                sum_house = sum_house + number + 1;
		            }
		            else if ((number >= 9) && (number <= 12)){
		                sum_house = sum_house + 10;
		            }
		            	            
		            strcpy(buff, "The house's hand is: ");
		            for (int i = 0; i < hand_house[0].size(); i++){
		                char card[5];
		                switch(hand_house[0][i]){
		                    case 0:
		                        sprintf(card, "A");
		                        number_of_ace_house++;
		                        break;
		                    case 1:
		                        sprintf(card, "2");
		                        break;
		                    case 2:
		                        sprintf(card, "3");
		                        break;
		                    case 3:
		                        sprintf(card, "4");
		                        break;    
		                    case 4:
		                        sprintf(card, "5");
		                        break;
		                    case 5:
		                        sprintf(card, "6");
		                        break;
		                    case 6:
		                        sprintf(card, "7");
		                        break;
		                    case 7:
		                        sprintf(card, "8");
		                        break;
		                    case 8:
		                        sprintf(card, "9");
		                        break;
		                    case 9:
		                        sprintf(card, "10");
		                        break;
		                    case 10:
		                        sprintf(card, "J");
		                        break;
		                    case 11:
		                        sprintf(card, "Q");
		                        break;
		                    case 12:
		                        sprintf(card, "K");
		                        break;                                                   
		                }
		                switch(hand_house[1][i]){
		                    case 0:
		                        strcat(card, "\u2661 ");
		                        break;
		                    case 1:
		                        strcat(card, "\u2662 ");
		                        break;
		                    case 2:
		                        strcat(card, "\u2663 ");
		                        break;
		                    case 3:
		                        strcat(card, "\u2660 ");
		                        break;                                           
		                }              
		                strcat(buff, card);
		            }
		            strcat(buff, "\r\n");
		            write(client_socket, buff, sizeof(buff));
		            
			        sprintf(buff, "The house's points: %d\r\n", sum_house);
					write(client_socket, buff, sizeof(buff));
			             
		            if(number_of_ace_house > 0){		            
			            if (((sum_house + 10) > sum_player) && ((sum_house + 10) <= 21)){
		                	sum_house += 10;
		                	number_of_ace_house = 0;
				        }
			            
		            }
		            
			   //     sprintf(buff, "The house's points: %d\r\n", sum_house);
			//		write(client_socket, buff, sizeof(buff));
					    
			        while((sum_house < sum_player) && (sum_house < 21)){
			        	if (index == deck.size()) break;
			        	number = deck[index] / 4;
			            suit = deck[index] % 4;
			            hand_house[0].push_back(number);
			            hand_house[1].push_back(suit);
			            index++;
			            if (number == 0) number_of_ace_house++;
			            if ((number >= 0) && (number <= 8)){
			                sum_house = sum_house + number + 1;
			            }
			            else if ((number >= 9) && (number <= 12)){
			                sum_house = sum_house + 10;
			            }
			            if(number_of_ace_house > 0){
				            if (((sum_house + 10) > sum_player) && ((sum_house + 10) <= 21)){
			                	sum_house += 10;
			                	number_of_ace_house = 0;
					        }
					    }
			//		    sprintf(buff, "The house's points: %d\r\n", sum_house);
			//			write(client_socket, buff, sizeof(buff));
		            }
		            
		            strcpy(buff, "The house's hand is: ");
		            for (int i = 0; i < hand_house[0].size(); i++){
		                char card[5];
		                switch(hand_house[0][i]){
		                    case 0:
		                        sprintf(card, "A");
		                        break;
		                    case 1:
		                        sprintf(card, "2");
		                        break;
		                    case 2:
		                        sprintf(card, "3");
		                        break;
		                    case 3:
		                        sprintf(card, "4");
		                        break;    
		                    case 4:
		                        sprintf(card, "5");
		                        break;
		                    case 5:
		                        sprintf(card, "6");
		                        break;
		                    case 6:
		                        sprintf(card, "7");
		                        break;
		                    case 7:
		                        sprintf(card, "8");
		                        break;
		                    case 8:
		                        sprintf(card, "9");
		                        break;
		                    case 9:
		                        sprintf(card, "10");
		                        break;
		                    case 10:
		                        sprintf(card, "J");
		                        break;
		                    case 11:
		                        sprintf(card, "Q");
		                        break;
		                    case 12:
		                        sprintf(card, "K");
		                        break;                                                   
		                }
		                switch(hand_house[1][i]){
		                    case 0:
		                        strcat(card, "\u2661 ");
		                        break;
		                    case 1:
		                        strcat(card, "\u2662 ");
		                        break;
		                    case 2:
		                        strcat(card, "\u2663 ");
		                        break;
		                    case 3:
		                        strcat(card, "\u2660 ");
		                        break;                                           
		                }              
		                strcat(buff, card);
		            }
		            strcat(buff, "\r\n");
		            write(client_socket, buff, sizeof(buff));
		            write(fd_client, buff, strlen(buff));
		            
		            sprintf(buff, "The house's points: %d\r\n", sum_house);
					write(client_socket, buff, sizeof(buff));
					
		            if(sum_house > 21){
		            	strcpy(buff, "YOU WIN.\r\n");
		                write(client_socket, buff, sizeof(buff));
		                total_chip += bet_chip*2;
	                	balance -= bet_chip;
	                	sprintf(buff, "You win: %d.\r\n", bet_chip);
			            write(fd_client, buff, strlen(buff));
		            }else if(sum_house > sum_player){
		            	strcpy(buff, "YOU LOSE.\r\n");
		                write(client_socket, buff, sizeof(buff));
		                balance += bet_chip;
		                sprintf(buff, "You lose: %d.\r\n", bet_chip);
			            write(fd_client, buff, strlen(buff));

		            }else {
			            strcpy(buff, "DRAW.\r\n");
		                write(client_socket, buff, sizeof(buff));
		                total_chip += bet_chip;
		                sprintf(buff, "You win: 0.\r\n");
			            write(fd_client, buff, strlen(buff));
		            }
		        }
		        	            
	            sprintf(buff, "Your total amount of chips: %d.\r\n", total_chip);
	            write(client_socket, buff, sizeof(buff));
	            
	            if(total_chip <= 0) {
		            strcpy(buff, "See you again.\r\n");
	                write(client_socket, buff, sizeof(buff));
	                
	                int profit = balance - init_balance;
	                //printf("%d\r\n",profit);
	                lseek(fd_server, 0L, SEEK_SET);
				    read(fd_server, buff, sizeof(buff));
				    init_balance = atoi(buff);
				    //printf("%d\r\n",init_balance);
				    //printf("%d\r\n",init_balance + profit);
				    sprintf(buff, "%d\r\n", init_balance + profit);
				    ftruncate(fd_server, 0);
					write(fd_server, buff, strlen(buff));
					
					sprintf(buff, "Your total amount of chips: %d chips.\r\n", total_chip);
		            write(fd_client, buff, strlen(buff));
		            
					printf("Player %s has ended the session.\r\n", name);
	            	break;
	            }
		        //again?
		        strcpy(buff, "\nDo you want to play again (y/n)?\r\n");
                write(client_socket, buff, sizeof(buff));                        
                read(client_socket, buff, sizeof(buff));
                if (buff[0] == 'y'){
	                
                }else if (buff[0] == 'n'){
	                strcpy(buff, "See you again.\r\n");
	                write(client_socket, buff, sizeof(buff));
	                
	                int profit = balance - init_balance;
	                //printf("%d\r\n",profit);
	                lseek(fd_server, 0L, SEEK_SET);
				    read(fd_server, buff, sizeof(buff));
				    init_balance = atoi(buff);
				    //printf("%d\r\n",init_balance);
				    //printf("%d\r\n",init_balance + profit);
				    sprintf(buff, "%d\r\n", init_balance + profit);
				    ftruncate(fd_server, 0);
					write(fd_server, buff, strlen(buff));

					sprintf(buff, "Your total amount of chips: %d chips.\r\n", total_chip);
		            write(fd_client, buff, strlen(buff));
		            
					printf("Player %s has ended the session.\r\n", name);
	                break;
                }
	        }
	        exit(0);                    
        }
        close(client_socket);
        wait(NULL);
    }
    
    
    return 0;
}

static void ctrl_handler(int sig_no) { 
	char answer;
	if(sig_no == SIGINT) {
		printf("Do you really want to stop?\n"); 
		printf("Press Y and Enter stop, Enter to cont. \n");
	    answer = getchar();
	    if (answer == 'Y') exit(0);
    }
	return; 
}