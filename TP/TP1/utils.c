#include "utils.h"

struct termios oldtio;
extern char *baudrate;

int open_fd(int port_number) {

  struct termios newtio;
  char* port;
  if (port_number > 1){
    port = malloc(10);
  } else {
    port = malloc(9);
  }
  sprintf(port, "/dev/ttyS%d", port_number);

  int fd = open(port, O_RDWR | O_NOCTTY);


  if (fd < 0)
  {
      perror(port);
      return -1;
  }

  if (tcgetattr(fd, &oldtio) == -1)
  { /* save current port settings */
      perror("tcgetattr");
      return -1;
  }

  bzero(&newtio, sizeof(newtio));

  if (strcmp("B4800", baudrate) == 0)
    newtio.c_cflag = B4800 | CS8 | CLOCAL | CREAD;
  else if (strcmp("B9600", baudrate) == 0)
    newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  else if (strcmp("B19200", baudrate) == 0)
    newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
  else if (strcmp("B38400", baudrate) == 0)
    newtio.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
  else if (strcmp("B115200", baudrate) == 0)
    newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;

  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 5 chars received */

  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es)
*/

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
      perror("tcsetattr");
      return -1;
  }

  printf("New termios structure set\n");

  return fd;
}

int close_fd(int fd){
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
      perror("tcsetattr");
      exit(-1);
  }

  close(fd);
  return 0;
}

int compareBaudRate(char *rate){
  if ((strcmp("B4800", rate) != 0)   &&
      (strcmp("B9600", rate) != 0)   &&
      (strcmp("B19200", rate) != 0)  &&
      (strcmp("B38400", rate) != 0)  &&
      (strcmp("B115200", rate) != 0)
  ) {
    printf("Invalid baudrate \n");
    return 1;
  }
  return 0;
}
