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
#include <sys/stat.h>
#include <ctype.h>


//thanks to WerWolv - https://github.com/WerWolv/EdiZon
bool isServiceRunning(const char *serviceName) {
  u8 tmp=0;
  SmServiceName service_name = smEncodeName(serviceName);
  Result rc = serviceDispatchInOut(smGetServiceSession(), 65100, service_name, tmp);
  if (R_SUCCEEDED(rc) && tmp & 1)
    return true;
  else
    return false;
}


bool is_dir(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}


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

void send_version(int sock, char to[]) {
    char version_packet[512];
	//irc->notice(hostd->nick,"\1VERSION %s\1",versionlist[curversion]); // my old irc bot hahaha
    sprintf(version_packet, "NOTICE %s :\1VERSION IRC Bot v%f for Nintendo Switch by ELY M.\1\r\n", to, APP_VERSION);
    send(sock, version_packet, strlen(version_packet), 0);
}

void send_time(int sock, char to[]) {
	const char* const months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	const char* const weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    char time_packet[512];
	
	//Clock
	const char* ampm = "AM";
	time_t unixTime = time(NULL);
	//struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	struct tm* timeStruct = localtime((const time_t *)&unixTime);
	int hours = timeStruct->tm_hour;
	int minutes = timeStruct->tm_min;
	int seconds = timeStruct->tm_sec;
	int day = timeStruct->tm_mday;
	int month = timeStruct->tm_mon;
	int year = timeStruct->tm_year +1900;
	int wday = timeStruct->tm_wday;
		
	if (hours <= 12 && hours >= 0) {
	//AM	
	ampm = "AM";
	}
    else if (hours >= 13 && hours <= 24)
    {
	hours = (hours - 12);
	//PM
	ampm = "PM";
    }	
	
	
    sprintf(time_packet, "NOTICE %s :\1TIME %s %s %i %i %02i:%02i:%02i %s\1\r\n", to, weekDays[wday], months[month], day, year, hours, minutes, seconds, ampm);
    send(sock, time_packet, strlen(time_packet), 0);
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

void part_channel(int sock, char channel[]) {
	printf("attempting to part %s\n", channel);
    char part_packet[512];
    sprintf(part_packet, "PART %s\r\n", channel);
    send(sock, part_packet, strlen(part_packet), 0);
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
    sprintf(quit_packet, "QUIT :%s\r\n", message);
    send(sock, quit_packet, strlen(quit_packet), 0);
}


char *removestr(char *str, const char *word)
{
    char *ptr = str;
    size_t len = strlen(word);
    while((ptr = strstr(ptr, word)))
    {
        if(isalnum(*(ptr + len)) || (str != ptr && isalnum(*(ptr -1))))
        {
            ptr += len;
        }
        else
        {
            memmove(ptr, ptr + len, strlen(ptr + len) + 1);
        }
    }
    return str;
}


int main(int argc, char **argv)
{

	consoleInit(NULL);
    
	// Initialise sockets
    socketInitializeDefault();
	
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	PadState pad;
	padInitializeDefault(&pad);	
	
	printf("\x1b[16;16HElys IRC Bot for Nintendo Switch\nPress PLUS to exit.");

	
	
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
       printf("Could not create socket");
       return 0;
    }
    
	
	
    char *ip = "35.193.150.184";
	//char *ip = "163.172.134.144";
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

        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
		
		if (kDown & HidNpadButton_Plus) { break; } // break in order to return to hbmenu
				
		
		
		char line[512];
        read_line(socket_desc, line);
		char *prefix = get_prefix(line);
        char *username = get_username(line);
        char *command = get_command(line);
        char *argument = get_last_argument(line);
		

		printf("CMD: %s\n", command);
		
		
		
		if (strcmp(command, "376") == 0) {
		join_channel(socket_desc, channel, key);
        }
	
		
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
			

			printf("ARG: %s\n", argument);
			
			if (strcmp(argument, "\1VERSION\1") == 0) {
			send_version(socket_desc, username);
			printf("Got Version. Replying to %s with version\n", username);
			}

			if (strcmp(argument, "\1TIME\1") == 0) {
			send_time(socket_desc, username);
			printf("Got Time. Replying to %s with time\n", username);
			}			
		
			
			if (strcmp(argument, "!version") == 0) {
            char *channel = get_argument(line, 1);
			char versionline[512];
			sprintf(versionline, "Hello, I am IRC Bot v%f for Nintendo Switch by ELY M.", APP_VERSION);
			send_message(socket_desc, channel, versionline);
            free(channel);
			}
			
			
			if (strcmp(argument, "!hello") == 0) {
            char *channel = get_argument(line, 1);
			send_message(socket_desc, channel, "Hello, I am Elys Switch :D");
            free(channel);
			}	
			
			//thanks to WerWolv - https://github.com/WerWolv/EdiZon
			if (strcmp(argument, "!cfw") == 0) {
            char *channel = get_argument(line, 1);
			char *cfwline = "I cant tell what cfw I am running.";
			if (isServiceRunning("tx")) {
			cfwline = "My cfw is SXOS";	
			} 
			else if (isServiceRunning("rnx")) {
			cfwline = "My cfw is REiNX";	
			}
			//atmosphere's cheat engine runs no matter what is in the setting ini. 
			//thanks to WerWolv
			else if (isServiceRunning("dmnt:cht")) {
			cfwline = "My cfw is Atmosphere";	
			}
			send_message(socket_desc, channel, cfwline);
            free(channel);
			}					
			
			
			//useless unless I can get 2nd part//
			//if (strcmp(argument, "!join") == 0) {
			if (strncmp (argument,"!joinxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",5) == 0) {	
			char joinline[512];
            char *channel = get_argument(line, 1);
			char *line2 =  get_argument(line, 2);
			char *line3 =  get_argument(line, 3);
			sprintf(joinline, "joined %s %s", line2, line3);
			send_message(socket_desc, channel, joinline);
			printf(joinline);
			printf("\n");
			join_channel(socket_desc, line2, line3);
            free(channel);
			}
			
			if (strncmp (argument,"!partxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",5) == 0) {	
			char partline[512];
            char *channel = get_argument(line, 1);
			char *line2 =  get_argument(line, 2);
			sprintf(partline, "parted %s", line2);
			printf(partline);
			printf("\n");
			send_message(socket_desc, channel, partline);
			part_channel(socket_desc, line2);
            free(channel);
			}
			
			if (strncmp (argument,"!privmsgxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",8) == 0) {	
			char privmsgline[512];
			char *to =  get_argument(line, 2);			
			char *message = removestr(argument, "!privmsg");
			char *smessage = removestr(message, "  ");
			char *fmessage = removestr(smessage, to);
			sprintf(privmsgline, "pmed to: %s msg: %s", to, fmessage);
			printf(privmsgline);
			printf("\n");
			send_message(socket_desc, to, fmessage);

			}


			//if (strcmp(argument, "!goodbye") == 0) {
			if (strncmp (argument,"!goodbyexxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",8) == 0) {
            char *channel = get_argument(line, 1);
			char *password = get_argument(line, 2);
			if (strcmp(password, "penis103") == 0) {
			send_quit(socket_desc, "Goodbye! I am Elys Modded Nintendo Switch");
			svcSleepThread(5000000000ull); // 5 secs
			break;
			} else {
			send_message(socket_desc, channel, "Nice try! you are logged :)");
			}
            free(channel);
			}
			
			
            free(channel);
        } //PRIVMSG
		
		
		
		//print other people joins, privmsgs, and quits 
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
