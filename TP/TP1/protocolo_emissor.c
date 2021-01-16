#include "protocolo_emissor.h"
#include "utils.h"

int flag = 1, conta = 1;
int fd;
unsigned char buffer[5];
int controloPosition = 1;
char controlo[2] = {0x00, 0x40};
char respostaPositiva[2] = {0x85, 0x05};
char respostaNegativa[2] = {0x81, 0x01};
char *novaTrama;
int newLength;
int packetNumber = 0;
extern int trama_size_limit;
extern int totalPackets;

void atendeLLClose(){

    if (conta > 5) exit(1);

    buffer[0] = FLAG;
    buffer[1] = ENDERECOEMISSOR;
    buffer[2] = DISC;
    buffer[3] = ENDERECOEMISSOR ^ DISC;
    buffer[4] = FLAG;

    write(fd, buffer, 5);
    printf("disc # %d\n", conta);

    if (conta <= 5)
      alarm(3);

    flag = 1;
    conta++;
}

void atendeLLWrite(){
    if (conta > 5) exit(1);

    write(fd, novaTrama, newLength);
    printf("write packet #%d / %d, tentativa #%d\n", packetNumber, totalPackets, conta);

    if (conta <= 5)
      alarm(3);

    flag = 1;
    conta++;

}

void atendeLLOpen() // atende alarme
{
    if (conta > 5) exit(1);

    buffer[0] = FLAG;
    buffer[1] = ENDERECOEMISSOR;
    buffer[2] = SET;
    buffer[3] = ENDERECOEMISSOR ^ SET;
    buffer[4] = FLAG;

    write(fd, buffer, 5);
    printf("set # %d\n", conta);

    if (conta <= 5)
      alarm(3);

    flag = 1;
    conta++;
}

int byteStuffing(char* trama, int length, char* novaTrama){

  int newLength = 1;

  novaTrama[0] = trama[0];

  for (int i = 1; i < length - 1; i++){
    if (trama[i] == FLAG){
      novaTrama[newLength++] = ESCAPE;
      novaTrama[newLength++] = (FLAG ^ 0x20);
    }
    else if (trama[i] == ESCAPE) {
      novaTrama[newLength++] = ESCAPE;
      novaTrama[newLength++] = (ESCAPE ^ 0x20);
    }
    else if (trama[i] == 0x00) {
      novaTrama[newLength++] = ESCAPE;
      novaTrama[newLength++] = (0x00 ^ 0x20);
    }
    else {
      novaTrama[newLength++] = trama[i];
    }
  }

  novaTrama[newLength++] = trama[length - 1];

  return newLength;
}

int recebeResposta(int fd){

  int res = 0;
  char buffer[5];

  res += read(fd, &buffer[0], 1);
  res += read(fd, &buffer[1], 1);
  res += read(fd, &buffer[2], 1);
  res += read(fd, &buffer[3], 1);
  res += read(fd, &buffer[4], 1);

  int ret = -3;
  if (buffer[0] != FLAG || buffer[4] != FLAG) ret = -1;
  if (buffer[1] != ENDERECORECETOR) ret = -2;
  if (buffer[3] != (buffer[2] ^ buffer[1])) ret = -4;

  if (buffer[2] == respostaPositiva[controloPosition]) ret = 0;
  return ret;

}

int llopen(int port, char flag1){

  int res = 0;

  fd = open_fd(port);

  if (fd < 0){
    exit(fd);
  }

  char buffer[5];

  (void)signal(SIGALRM, atendeLLOpen); // instala  rotina que atende interrupcao

  buffer[0] = FLAG;
  buffer[1] = ENDERECOEMISSOR;
  buffer[2] = SET;
  buffer[3] = ENDERECOEMISSOR ^ SET;
  buffer[4] = FLAG;

  write(fd, buffer, 5);
  printf("set # %d\n", conta);

  conta++;

  while (conta < 5)
  {
      if (flag)
      {
          alarm(3); // activa alarme de 3s
          flag = 0;
          res += read(fd, &buffer[0], 1);
          res += read(fd, &buffer[1], 1);
          res += read(fd, &buffer[2], 1);
          res += read(fd, &buffer[3], 1);
          res += read(fd, &buffer[4], 1);
          if (res != 5) return -2;

          if (buffer[0] == FLAG && buffer[4] == FLAG && buffer[1] == ENDERECORECETOR)
          {
              break;
          }
      }
  }

  if (buffer[0] != FLAG || buffer[4] != FLAG || buffer[1] != ENDERECORECETOR || buffer[3] != (buffer[1] ^ buffer[2]))
  {
      return -1;
  }
  alarm(0);
  conta = 1;
  flag = 1;

  return fd;
}

int llwrite(int fd, char* buffer, int lenght){
  char *trama = malloc(lenght + 6);
  char bcc = 0x00;


  trama[0] = FLAG;
  trama[1] = ENDERECOEMISSOR;
  trama[2] = controlo[controloPosition];
  trama[3] = trama[1] ^ trama[2];
  for (int i = 0; i < lenght; i++){
    trama[4 + i] = buffer[i];
    bcc = bcc ^ buffer[i];
  }

  trama[lenght + 4] = bcc;
  trama[lenght + 5] = FLAG;



  novaTrama = malloc(trama_size_limit);
  newLength = byteStuffing(trama, lenght + 6, novaTrama);

  write(fd, novaTrama, newLength);
  printf("write packet #%d / %d\n", packetNumber, totalPackets);

  (void)signal(SIGALRM, atendeLLWrite); // instala  rotina que atende interrupcao
  int ret = 1;
  while (conta <= 5)
  {
      if (flag)
      {
          alarm(3);
          ret = recebeResposta(fd);
          if (ret < 0){
            read(fd, trama, trama_size_limit);
            conta--;  // Quando recebe uma resposta negativa não aumentar a variável conta
          }
          flag = 0;
          if (!ret){
            break;
          }
      }
  }
  if (ret){
    return -1;
  }

  alarm(0);
  conta = 1;
  flag = 1;


  controloPosition = !controloPosition;
  packetNumber++;

  free(novaTrama);
  return 0;
}

int llclose(int fd) {
  int res = 0;
  buffer[0] = FLAG;
  buffer[1] = ENDERECOEMISSOR;
  buffer[2] = DISC;
  buffer[3] = ENDERECOEMISSOR ^ DISC;
  buffer[4] = FLAG;
  if(write(fd, buffer, 5) < 0) return -1;
  printf("disc # %d\n", conta);

  conta++;

  (void)signal(SIGALRM, atendeLLClose); // instala  rotina que atende interrupcao

  while (conta < 5)
  {
      if (flag)
      {
          alarm(3); // activa alarme de 3s
          flag = 0;
          res += read(fd, &buffer[0], 1);
          res += read(fd, &buffer[1], 1);
          res += read(fd, &buffer[2], 1);
          res += read(fd, &buffer[3], 1);
          res += read(fd, &buffer[4], 1);
          if (res != 5) return -2;
          if (buffer[0] == FLAG || buffer[1] == ENDERECORECETOR || buffer[2] == DISC || buffer[3] == (ENDERECORECETOR ^ DISC) || buffer[4] == FLAG){
            break;
          }
      }
  }
  alarm(0);

  buffer[0] = FLAG;
  buffer[1] = ENDERECOEMISSOR;
  buffer[2] = UA;
  buffer[3] = ENDERECOEMISSOR ^ UA;
  buffer[4] = FLAG;
  if(write(fd, buffer, 5) < 0) return -1;

  usleep(10000); //Not writing this byte correctly without this line
  close_fd(fd);
  return 0;
}
