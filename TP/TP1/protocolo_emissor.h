#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "macros.h"

void atendeLLWrite();
void atendeLLOpen();
int llopen(int porta, char flag);
int llwrite(int fd, char* buffer, int length);
int llclose(int fd);
int byteStuffing(char* trama, int length, char* novaTrama);
