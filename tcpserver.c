#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h> // for close()

// for sockets
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define BUFSIZE 1024
#define SERVPORT 1800
#define BACKLOG 5

// GPIO pins
#define LedPin 0 // GPIO 17
#define ButtonPin 1 // GPIO 18

void check_err(int8_t, char[]);
char *set_httpHeader(char[], const char *);
char get_path(char [], int);

int main(void)
{
    char httpHeader_ok[] = "HTTP/1.1 200 OK\r\n\n";
    int8_t server_socket, client_socket;
    struct sockaddr_in servaddr;

		if(wiringPiSetup() == -1) {
			fprintf(stderr, "wiringpi setup error");
			exit(0);
		}	

		pinMode(LedPin, OUTPUT);
		pinMode(ButtonPin, INPUT);

    check_err((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "Server socket: ");
    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVPORT);

    check_err(bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)), "Binding socket to server: ");

    check_err(listen(server_socket, BACKLOG), "Socket listen: ");

    char *message = set_httpHeader(httpHeader_ok, "index.html");

    while (1)
    {
        printf("Waiting for connection...\n");
        check_err((client_socket = accept(server_socket, NULL, NULL)), "Accepting client: ");

        if (client_socket > 0)
        {
            printf("Connected\n");
						write(client_socket, message, strlen(message));

						char path[BUFSIZE];
						get_path(path, client_socket);

						if (strcasecmp(path, "/on") == 0)
						{
								printf("ON\n");
								digitalWrite(LedPin, LOW);
						}
						else if (strcasecmp(path, "/off") == 0)
						{
								printf("OFF\n");
								digitalWrite(LedPin, HIGH);
						}

        }
        close(client_socket);
    }

    free(message);
    close(server_socket);

    return 0;
}

void check_err(int8_t to_check, char message[])
{
    if (to_check < 0)
    {
        perror(message);
        exit(1);
    }
}

char *set_httpHeader(char httpHeader[], const char *file_name)
{
    // File object to return
    FILE *htmlData = fopen(file_name, "r");

    char line[100];
    char *responseData = malloc(8000);

		memset(responseData, 0, 8000);
    strcat(responseData, httpHeader);
		printf("%s", responseData);

    while (fgets(line, 100, htmlData) != 0)
    {
        strcat(responseData, line);
    }

    return responseData;
}

char get_path(char path[], int fd)
{
    char buff[8000];
    int i = 0, j = 0;

    read(fd, buff, 8000);

    while ((buff[i++]) != ' ')
        ;
    while ((path[j++] = buff[i++]) != ' ')
        ;
    path[j - 1] = '\0';

    return *path;
}
