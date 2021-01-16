#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "macros.h"
#include "utils.h"

int llopen(int porta, char flag1);
int llread(int fd, unsigned char* buffer);
int llclose(int fd);
int byteDestuffing(char* trama, int length, char* novaTrama);
void enviaResposta(int fd, int ret);
