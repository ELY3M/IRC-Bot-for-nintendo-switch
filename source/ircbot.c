/*


I know this is old school.  
I use to mess with IRC Bots and stuff alot hahahahah  
let see if I could get everyones switches in a IRC channel lol       
Just kidding!   

This is so basic :)         


ELY M.    


*/

#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int read_line(int sock, char *buffer) {
    size_t length = 0;

    while (1) {
        char data;
        int result = recv(sock, &data, 1, 0);

        if ((result <= 0) || (data == EOF)){
            perror("Connection closed");
            exit(1);
        }

        buffer[length] = data;
        length++;
        
        if (length >= 2 && buffer[length-2] == '\r' && buffer[length-1] == '\n') {
            buffer[length-2] = '\0';
            return length;
        }
    }
}



char *get_prefix(char *line) {
    char *prefix = malloc(512);
    char clone[512];

    strncpy(clone, line, strlen(line) + 1);
    if (line[0] == ':') {
        char *splitted = strtok(clone, " ");
        if (splitted != NULL) {
            strncpy(prefix, splitted+1, strlen(splitted)+1);
        } else {
            prefix[0] = '\0';
        }
    } else {
        prefix[0] = '\0';
    }
    return prefix;
}

char *get_username(char *line) {
    char *username = malloc(512);
    char clone[512];

    strncpy(clone, line, strlen(line) + 1);
    if (strchr(clone, '!') != NULL) {
        char *splitted = strtok(clone, "!");
        if (splitted != NULL) {
            strncpy(username, splitted+1, strlen(splitted)+1);
        } else {
            username[0] = '\0';
        }
    } else {
        username[0] = '\0';
    }
    return username;
}

char *get_command(char line[]){
    char *command = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    char *splitted = strtok(clone, " ");
    if (splitted != NULL){
        if (splitted[0] == ':'){
            splitted = strtok(NULL, " ");
        }
        if (splitted != NULL){
            strncpy(command, splitted, strlen(splitted)+1);
        }else{
            command[0] = '\0';
        }
    }else{
        command[0] = '\0';
    }
    return command;
}

char *get_last_argument(char line[]){
    char *argument = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    char *splitted = strstr(clone, " :");
    if (splitted != NULL){
        strncpy(argument, splitted+2, strlen(splitted)+1);
    }else{
        argument[0] = '\0';
    }
    return argument;
}

char *get_argument(char line[], int argno){
    char *argument = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    
    int current_arg = 0;
    char *splitted = strtok(clone, " ");
    while (splitted != NULL){
        if (splitted[0] != ':'){
            current_arg++;
        }
        if (current_arg == argno+1){
            strncpy(argument, splitted, strlen(splitted)+1);
            return argument;
        }
        splitted = strtok(NULL, " ");
    }
    
    if (current_arg != argno){
        argument[0] = '\0';
    }
    return argument;
}

void set_nick(int sock, char nick[]){
    char nick_packet[512];
    sprintf(nick_packet, "NICK %s\r\n", nick);
    send(sock, nick_packet, strlen(nick_packet), 0);
}

void send_user_packet(int sock, char nick[]){
    char user_packet[512];
    sprintf(user_packet, "USER %s 0 * :%s\r\n", nick, nick);
    send(sock, user_packet, strlen(user_packet), 0);
}

void join_channel(int sock, char channel[], char key[]) {
	printf("attempting to join %s %s\n", channel, key);
    char join_packet[512];
    sprintf(join_packet, "JOIN %s %s\r\n", channel, key);
    send(sock, join_packet, strlen(join_packet), 0);
}

void send_pong(int sock, char argument[]) {
    char pong_packet[512];
    sprintf(pong_packet, "PONG :%s\r\n", argument);
    send(sock, pong_packet, strlen(pong_packet), 0);
}

void send_message(int sock, char to[], char message[]) {
    char message_packet[512];
    sprintf(message_packet, "PRIVMSG %s :%s\r\n", to, message);
    send(sock, message_packet, strlen(message_packet), 0);
}

void send_quit(int sock, char message[]) {
    char quit_packet[512];
    sprintf(quit_packet, "QUIT %s :%s\r\n", message);
    send(sock, quit_packet, strlen(quit_packet), 0);
}



int main(int argc, char **argv)
{

	consoleInit(NULL);
    
	// Initialise sockets
    socketInitializeDefault();
	
	printf("\x1b[16;16HElys IRC Bot for Nintendo Switch\nPress PLUS to exit.");

	
	
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
       printf("Could not create socket");
       return 0;
    }
    
	
	
    char *ip = "35.193.150.184";
    int port = 6667;

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //free(ip);
    //free(port);

    if (connect(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
       printf("Could not connect");
       return 0;
    }
    
	
	//lets make a random nick hahahah//  
	
	
    char *nick = "ElysSwitch";
    char *channel = "#switch";
	char *key = "key";

    set_nick(socket_desc, nick);
    send_user_packet(socket_desc, nick);
    join_channel(socket_desc, channel, key);


	

	while(appletMainLoop())
	{

		hidScanInput();
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
		
		if (kDown & KEY_PLUS) { break; } // break in order to return to hbmenu
				
		
		join_channel(socket_desc, channel, key);
		
		char line[512];
        read_line(socket_desc, line);
		char *prefix = get_prefix(line);
        char *username = get_username(line);
        char *command = get_command(line);
        char *argument = get_last_argument(line);
		

		
		if (strcmp(command, "PING") == 0) {
        send_pong(socket_desc, argument);
        printf("Got ping. Replying with pong...\n");
        }
		else if (strcmp(command, "PRIVMSG") == 0) {
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s/%s: %s", channel, username, argument);
            printf(logline);
			printf("\n");
			

			if (strcmp(argument, "!hello") == 0) {
			printf("Got !hello\n");
            char *channel = get_argument(line, 1);
			send_message(socket_desc, channel, "Hello, I am Elys Switch :D");
            free(channel);
			}	


			if (strcmp(argument, "!goodbye") == 0) {
			printf("Got !goodbye\n");
            char *channel = get_argument(line, 1);
			send_quit(socket_desc, "Goodbye! I am Elys modded nintendo switch");
            free(channel);
			break;
			}
			
			
            free(channel);
        }
		else if (strcmp(command, "JOIN") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s joined %s.", username, channel);
            printf(logline);
			printf("\n");
            free(channel);
        }
		else if (strcmp(command, "PART") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s left %s.", username, channel);
            printf(logline);
			printf("\n");
            free(channel);
        }
		else if (strcmp(command, "QUIT") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s quit %s.", username, channel);
            printf(logline);
			printf("\n");
            free(channel);
        }		
		

		

		
		consoleUpdate(NULL);

	
	}
	
	
	
	
	//free(ip);
    //free(port);
    //free(nick);
    //free(channels);
	socketExit();
	consoleExit(NULL);
	return 0;
	
	
}
