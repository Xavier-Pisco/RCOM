/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "macros.h"
int open_fd(int porta);
int close_fd(int fd);
int compareBaudRate(char *rate);
