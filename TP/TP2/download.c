#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

int connectToIp(char *address, int port, int *sockfd)
{
	struct sockaddr_in server_addr;

	/*server address handling*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);				  /*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		return -1;
	}
	/*connect to the server*/
	if (connect(*sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect()");
		return -1;
	}

	return 0;
}

struct hostent *getIp(char *hostname)
{
	struct hostent *h;

	if ((h = gethostbyname(hostname)) == NULL)
	{
		herror("gethostbyname");
		exit(1);
	}

	return h;
}

int sendCommand(int sockfd, char *command)
{
	char finalCommand[4096];
	sprintf(finalCommand, "%s\r\n", command);

	if (write(sockfd, finalCommand, strlen(finalCommand)) < 0)
	{
		perror("Error writing command\n");
		return -1;
	}
	return 0;
}

int readResponse(int sockfd, char *response)
{
	int ret;
	if ((ret = read(sockfd, response, 4096)) < 0)
	{
		perror("Error reading response\n");
		return -1;
	}
	return ret;
}

int parsePasv(char *response, char *address, int *port)
{
	int i;
	for (i = 0; i < strlen(response); i++)
	{
		if (response[i] == '(')
		{
			break;
		}
	}
	i++;
	char *delimPasv;
	delimPasv = malloc(256);
	for (int j = i; j < strlen(response); j++)
	{
		delimPasv[j - i] = response[j];
	}
	char *a1 = strtok(delimPasv, ",");
	char *a2 = strtok(NULL, ",");
	char *a3 = strtok(NULL, ",");
	char *a4 = strtok(NULL, ",");
	char *p1 = strtok(NULL, ",");
	char *p2 = strtok(NULL, ")");
	*port = atoi(p1) * 256 + atoi(p2);
	sprintf(address, "%s.%s.%s.%s", a1, a2, a3, a4);
	return 0;
}

void parseInput(char *input, char **user, char **password, char **host, char **path)
{
	input = &input[6];
	int userValid = 0;
	int passwordValid = 0;
	for (int i = 0; i < strlen(input); i++)
	{
		if (input[i] == '@')
		{
			userValid = 1;
			break;
		}
		if (input[i] == ':')
		{
			passwordValid = 1;
		}
	}

	if (userValid == 1)
	{
		if (passwordValid == 1)
		{
			*user = strtok(input, ":");
			*password = strtok(NULL, "@");
			*host = strtok(NULL, "/");
			*path = strtok(NULL, "\0");
		}
		else
		{
			*user = strtok(input, "@");
			*password = "anonymous";
			*host = strtok(NULL, "/");
			*path = strtok(NULL, "\0");
		}
	}
	else
	{
		*user = "anonymous";
		*password = "anonymous";
		*host = strtok(input, "/");
		*path = strtok(NULL, "\0");
	}
}

char *getFirst3Chars(char *string)
{
	char *ret = malloc(3);
	for (int i = 0; i < 3; i++)
	{
		ret[i] = string[i];
	}
	return ret;
}

char *pathToFile(char *path)
{
	int i;
	char *temp = malloc(256);
	memset(temp, 0, 256);
	for (i = strlen(path) - 1; i >= 0; i--)
	{
		if (path[i] == '/')
		{
			break;
		}
	}
	i++;
	int j = 0;
	for (i; i < strlen(path); i++)
	{
		temp[j] = path[i];
		j++;
	}
	return temp;
}

int main(int argc, char **argv)
{
	struct hostent *h;
	int sockfd;

	if (argc != 2)
	{
		printf("Usage: ./download ftp://[<user>:<password>@]<host>/<url-path>");
		exit(1);
	}

	char *user = NULL;
	char *password = NULL;
	char *host = NULL;
	char *path = NULL;
	parseInput(argv[1], &user, &password, &host, &path);

	h = getIp(host);
	connectToIp(inet_ntoa(*((struct in_addr *)h->h_addr)), 21, &sockfd);
	char response[4096];
	sleep(1);
	while (readResponse(sockfd, response) == 4096)
	{
		memset(response, 0, 4096);
		continue;
	}

	char *cmd = malloc(4096);
	sprintf(cmd, "user %s", user);
	sendCommand(sockfd, cmd);
	memset(response, 0, 4096);
	sleep(1);
	readResponse(sockfd, response);
	char *code = malloc(3);
	code = getFirst3Chars(response);
	if (atoi(code) != 331 && atoi(code) != 230)
	{
		printf("Invalid username\n");
		close(sockfd);
		exit(1);
	}
	printf("%s", response);

	if (atoi(code) != 230)
	{
		sprintf(cmd, "pass %s", password);
		sendCommand(sockfd, cmd);
		memset(response, 0, 4096);
		sleep(1);
		readResponse(sockfd, response);
		code = getFirst3Chars(response);
		if (atoi(code) != 230)
		{
			printf("Wrong password\n");
			close(sockfd);
			exit(1);
		}
		printf("%s", response);
	}

	sendCommand(sockfd, "pasv");
	memset(response, 0, 4096);
	usleep(1000);
	readResponse(sockfd, response);
	code = getFirst3Chars(response);
	if (atoi(code) != 227)
	{
		printf("Failed to enter Passive Mode\n");
		close(sockfd);
		exit(1);
	}
	printf("%s", response);

	char newAdress[4096];
	int port;
	int newSockfd;
	parsePasv(response, newAdress, &port);
	connectToIp(newAdress, port, &newSockfd);

	sprintf(cmd, "retr %s", path);
	sendCommand(sockfd, cmd);
	memset(response, 0, 4096);
	usleep(1000);
	readResponse(sockfd, response);
	code = getFirst3Chars(response);
	if (atoi(code) != 150)
	{
		printf("Failed to open binary mode data\n");
		close(newSockfd);
		close(sockfd);
		exit(1);
	}
	printf("%s", response);

	int fd;
	char *file = pathToFile(path);
	if ((fd = open(file, O_WRONLY | O_CREAT, 0777)) < 0)
	{
		perror("Error creating file\n");
		close(newSockfd);
		close(sockfd);
		exit(1);
	}

	memset(response, 0, 4096);
	int ret;
	while ((ret = readResponse(newSockfd, response)) > 0)
	{
		write(fd, response, ret);
		memset(response, 0, 4096);
	}

	memset(response, 0, 40);
	usleep(1000);
	readResponse(sockfd, response);
	code = getFirst3Chars(response);
	if (atoi(code) != 226)
	{
		printf("Failed to transfer file\n");
		close(newSockfd);
		close(sockfd);
		exit(1);
	}
	printf("%s", response);

	close(newSockfd);
	close(sockfd);
	exit(0);
}
