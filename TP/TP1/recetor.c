/*Non-Canonical Input Processing*/

#include "recetor.h"

volatile int STOP = FALSE;

int fd;
extern int controloPosition;
int controloRecetor = 0;
unsigned char *text;
int packetNumber = 1;
int trama_size_limit = 1024;
char *baudrate;

int receiveDataPacket(char *data)
{
  int ret;
  int size = 0;

  ret = llread(fd, text);
  printf("Received Packet %d with return %d\n", packetNumber, ret);
  if (ret < 0)
    return -2;
  if (ret == 0)
    return 0;
  if (text[0] != 0x01 && text[0] != 0x03)
  {
    printf("Error on packet\n");
    return -3;
  }
  else if (text[0] == 0x03)
  {
    return -1;
  }
  else if (text[0] == 0x01)
  {
    size += ((int)text[2]) * 256;
    size += (int)text[3];
    for (int i = 0; i < size; i++)
    {
      data[i] = text[4 + i];
    }
    return size;
  }
  else
  {
    return -4;
  }
}

int receiveStartPacket(char *filename, int fileSize)
{
  int ret;
  int sizeL1 = 0;
  int sizeL2 = 0;
  ret = llread(fd, text);
  if (ret < 0) return -1;
  if (text[0] != 0x02)
  {
    printf("Error on start packet\n");
    return -1;
  }
  if (text[1] == 0x00)
  {
    sizeL1 = (int)text[2];
    char *number = malloc(sizeL1 + 1);
    for (int i = 0; i < sizeL1; i++)
    {
      number[i] = text[3 + i];
    }
    number[sizeL1] = '\0';
    fileSize = atoi(number);
  }
  if (text[sizeL1 + 3] == 0x01)
  {
    sizeL2 = (int)text[sizeL1 + 4];
    filename = realloc(filename, sizeL2);
    for (int i = 0; i < sizeL2; i++)
    {
      filename[i] = text[sizeL1 + 5 + i];
    }
  }
  return 0;
}

int main(int argc, char **argv)
{
  if ((argc < 4) ||
      ((strcmp("0", argv[1]) != 0) &&
       (strcmp("1", argv[1]) != 0) &&
       (strcmp("10", argv[1]) != 0) &&
       (strcmp("11", argv[1]) != 0)) ||
       compareBaudRate(argv[3]))
  {
    printf("Usage:\tnserial SerialPortNumber TramaSizeLimit Baudrate\n\tex: nserial 1 1024 B34200\n");
    exit(1);
  }
  srand(time(NULL));

  trama_size_limit = atoi(argv[2]);
  baudrate = argv[3];
  text = malloc(trama_size_limit - 6);

  /*read
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = llopen(atoi(argv[1]), ENDERECORECETOR);

  if (fd < 0)
    exit(fd);

  int ret;
  char *data = malloc(trama_size_limit - 6);
  int fileSize = 0;
  char *filename = malloc(256);

  receiveStartPacket(filename, fileSize);
  enviaResposta(fd, 1);
  printf("Received Packet %d\n", packetNumber);
  controloPosition = !controloPosition;

  char *temp = malloc(64);
  sprintf(temp, "%s.new", filename);

  int fd2;
  if ((fd2 = open(temp, O_WRONLY | O_CREAT, 0777)) < 0)
  {
    perror(temp);
    exit(1);
  }
  memset(data, 0, trama_size_limit - 6);

  while (1)
  {
    memset(data, 0, trama_size_limit - 6);
    ret = receiveDataPacket(data);
    if (ret == -1)
      break;
    enviaResposta(fd, ret);
    if (ret <= 0)
    {
      continue;
    }
    write(fd2, data, ret);
    controloRecetor = !controloRecetor;
    controloPosition = !controloPosition;
	packetNumber +=1;
  }
  enviaResposta(fd, 1);

  free(data);
  close(fd2);

  if (llclose(fd) < 0)
  {
    exit(2);
  }

  return 0;
}
