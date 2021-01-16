/*Non-Canonical Input Processing*/

#include "emissor.h"

volatile int STOP = FALSE;

int fd, fd2;
int controloEmissor = 0;
unsigned char buffer[5];
char *packet;
char *baudrate;
int trama_size_limit = 1024;
int totalPackets;

int sendDataPacket(char *text, int res)
{


  packet[0] = 0x01;
  packet[1] = (char)controloEmissor;
  packet[2] = (char)(res / 256);
  packet[3] = (char)(res % 256);
  for (int i = 0; i < res; i++){
    packet[4 + i] = text[i];
  }
  if (llwrite(fd, packet, res + 4) < 0)
  {
    printf("Error writing\n");
    return -2;
  }

  return 0;
}

int sendControlPacket(char controlByte, char *filename)
{
  struct stat fileStat;
  if (fstat(fd2, &fileStat) < 0)
    return -1;

  char *size = malloc(trama_size_limit);
  sprintf(size, "%ld", fileStat.st_size);
  int ssize = strlen(size);

  char *text = malloc(trama_size_limit / 2 - 6);
  text[0] = controlByte;
  text[1] = 0x00;
  text[2] = ssize;
  for (int i = 0; i < ssize; i++)
  {
    text[3 + i] = size[i];
  }
  text[3 + ssize] = 0x01;
  text[4 + ssize] = strlen(filename);
  for (int i = 0; i < strlen(filename); i++)
  {
    text[5 + ssize + i] = filename[i];
  }

  totalPackets = atoi(size) / (trama_size_limit / 2.0 - 6) + 2;

  if (llwrite(fd, text, 5 + ssize + strlen(filename)) < 0)
  {
    printf("Error writing\n");
    return -2;
  }
  free(size);
  free(text);
  return 0;
}

int main(int argc, char **argv)
{
  if ((argc < 5) ||
      ((strcmp("0", argv[1]) != 0) &&
       (strcmp("1", argv[1]) != 0) &&
       (strcmp("10", argv[1]) != 0) &&
       (strcmp("11", argv[1]) != 0)) ||
       compareBaudRate(argv[4]))
  {
    printf("Usage:\tnserial SerialPortNumber File TramaSizeLimit Baudrate\n\tex: nserial 1 pinguim.gif 1024 B38400\n");
    exit(1);
  }


  trama_size_limit = atoi(argv[3]);
  baudrate = argv[4];
  packet = malloc(trama_size_limit - 6);

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


  struct timespec initial_time;
  long initial_miliseconds;

  clock_gettime(CLOCK_REALTIME, &initial_time);
  initial_miliseconds = initial_time.tv_sec * 1000 + initial_time.tv_nsec / 10000000;

  if ((fd = llopen(atoi(argv[1]), ENDERECOEMISSOR)) < 0)
  {
    printf("Couldn't setup\n");
    exit(3);
  }

  if (fd < 0)
    exit(fd);

  if ((fd2 = open(argv[2], O_RDONLY)) < 0)
  {
    perror(argv[2]);
    exit(1);
  }

  if (sendControlPacket(0x02, argv[2]))
    exit(1);

  char *text = malloc(trama_size_limit / 2 - 6);

  memset(text, 0, trama_size_limit / 2 - 6);

  int res;
  while ((res = read(fd2, text, trama_size_limit / 2 - 6)) > 0)
  {
    if (sendDataPacket(text, res) < 0) exit(1);
    memset(text, 0, trama_size_limit / 2);
    controloEmissor = !controloEmissor;
  }

  if (sendControlPacket(0x03, argv[2]))
    exit(1);
  close(fd2);

  free(text);

  if (llclose(fd) < 0)
    exit(2);



  struct timespec final_time;
  long final_miliseconds;
  clock_gettime(CLOCK_REALTIME, &final_time);
  final_miliseconds = final_time.tv_sec * 1000 + final_time.tv_nsec / 10000000;

  printf("%ld %ld\n", final_miliseconds, initial_miliseconds);

  printf("Elapsed time: %ld ms\n" , (final_miliseconds - initial_miliseconds));

  return 0;
}
