#include "protocolo_recetor.h"
#include <time.h>

int fd;
int controloPosition = 1;
char controlo[2] = {0x00, 0x40};
char respostaPositiva[2] = {0x85, 0x05};
char respostaNegativa[2] = {0x81, 0x01};
extern int trama_size_limit;

int byteDestuffing(char *trama, int length, char *novaTrama)
{

  int newLength = 0;

  for (int i = 0; i < length; i++)
  {
    if (trama[i] == ESCAPE && trama[i + 1] == 0x5e)
    {
      novaTrama[newLength++] = 0x7e;
      i++;
    }
    else if (trama[i] == ESCAPE && trama[i + 1] == 0x5d)
    {
      novaTrama[newLength++] = 0x7d;
      i++;
    }
    else if (trama[i] == ESCAPE && trama[i + 1] == (0x00 ^ 0x20))
    {
      novaTrama[newLength++] = 0x00;
      i++;
    }
    else
    {
      novaTrama[newLength++] = trama[i];
    }
  }

  return newLength;
}

void enviaResposta(int fd, int ret)
{
  char trama[5];
  if (ret > 0)
  {
    trama[0] = FLAG;
    trama[1] = ENDERECORECETOR;
    trama[2] = respostaPositiva[controloPosition];
    trama[3] = trama[1] ^ trama[2];
    trama[4] = FLAG;
    write(fd, trama, 5);
  } else if (ret == 0){
    trama[0] = FLAG;
    trama[1] = ENDERECORECETOR;
    trama[2] = respostaPositiva[!controloPosition];
    trama[3] = trama[1] ^ trama[2];
    trama[4] = FLAG;
    write(fd, trama, 5);
  }
  else
  {
    trama[0] = FLAG;
    trama[1] = ENDERECORECETOR;
    trama[2] = respostaNegativa[controloPosition];
    trama[3] = trama[1] ^ trama[2];
    trama[4] = FLAG;
    write(fd, trama, 5);
  }
}

int llopen(int porta, char flag1)
{
  int res = 0;

  fd = open_fd(porta);

  if (fd < 0)
  {
    return fd;
  }

  char buffer[5];

  res += read(fd, &buffer[0], 1);
  res += read(fd, &buffer[1], 1);
  res += read(fd, &buffer[2], 1);
  res += read(fd, &buffer[3], 1);
  res += read(fd, &buffer[4], 1);
  if (res != 5)
    return -2;

  if (buffer[0] == FLAG && buffer[4] == FLAG && buffer[1] == ENDERECOEMISSOR && buffer[2] == SET && buffer[3] == (buffer[1] ^ buffer[2]))
  {
    buffer[0] = FLAG;
    buffer[1] = ENDERECORECETOR;
    buffer[2] = UA;
    buffer[3] = ENDERECORECETOR ^ UA;
    buffer[4] = FLAG;

    write(fd, buffer, 5);

    printf("Setup finished\n");
  }
  else
  {
    printf("Couldn't setup\n");
    return -1;
  }

  return fd;
}

int llread(int fd, unsigned char *buffer)
{
  char *trama = malloc(trama_size_limit);
  char *temp = malloc(trama_size_limit);
  char bcc = 0x00;
  int size = 0;
  memset(trama, 0, trama_size_limit);

  while (1)
  {
    memset(temp, 0, trama_size_limit);
    size += read(fd, temp, trama_size_limit);
    strcat(trama, temp);
    if (trama[size - 1] == FLAG && size > 1)
      break;
    if (size > trama_size_limit)
      return -1;
  }

  int r = rand() % 10;

  if (r == 0 || r == 1 || r == 2 || r == 3 ){ //40 % de FER
    trama[0] = 0x01;
  }

  char *newTrama = malloc(trama_size_limit);
  int newLength = byteDestuffing(trama, size, newTrama);

  int ret = 0;
  if (size <= 0)
  {
    return size;
  }

  if (newTrama[0] != FLAG || newTrama[newLength - 1] != FLAG)
    ret = -1;
  if (newTrama[2] == DISC && newTrama[3] == (newTrama[2] ^ newTrama[1]))
    return -5;
  if (newTrama[2] != controlo[controloPosition])
  {
    if (newTrama[2] == controlo[!controloPosition])
      return 0;
  }

  if (newTrama[3] != (newTrama[1] ^ newTrama[2]))
    ret = -3;

  for (int i = 4; i < newLength - 2; i++)
  {
    buffer[i - 4] = newTrama[i];
    bcc = bcc ^ newTrama[i];
  }

  if (bcc != newTrama[newLength - 2])
    ret = -4;

  if (ret == 0)
    ret = newLength - 6;


  free(newTrama);
  free(trama);
  return ret;
}

int llclose(int fd)
{
  char buffer[5];
  int res = 0;

  res += read(fd, &buffer[0], 1);
  res += read(fd, &buffer[1], 1);
  res += read(fd, &buffer[2], 1);
  res += read(fd, &buffer[3], 1);
  res += read(fd, &buffer[4], 1);
  if (buffer[0] != FLAG || buffer[1] != ENDERECOEMISSOR || buffer[2] != DISC || buffer[3] != (ENDERECOEMISSOR ^ DISC) || buffer[4] != FLAG)
    return -1;

  buffer[0] = FLAG;
  buffer[1] = ENDERECORECETOR;
  buffer[2] = DISC;
  buffer[3] = ENDERECORECETOR ^ DISC;
  buffer[4] = FLAG;

  if (write(fd, buffer, 5) < 0)
    return -1;

  res += read(fd, &buffer[0], 1);
  res += read(fd, &buffer[1], 1);
  res += read(fd, &buffer[2], 1);
  res += read(fd, &buffer[3], 1);
  res += read(fd, &buffer[4], 1);

  if (buffer[0] != FLAG || buffer[1] != ENDERECOEMISSOR || buffer[2] != UA || buffer[3] != (ENDERECOEMISSOR ^ UA) || buffer[4] != FLAG)
    return -2;

  close_fd(fd);

  return 0;
}
