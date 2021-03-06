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

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define ENDERECOEMISSOR 0x03
#define ENDERECORECETOR 0x01
#define FLAG 0x7e
#define SET 0x03
#define UA 0x07

volatile int STOP = FALSE;

int flag = 1, conta = 1;
int fd;
unsigned char buffer[6];

void atende() // atende alarme
{

	buffer[0] = FLAG;
	buffer[1] = ENDERECOEMISSOR;
	buffer[2] = SET;
	buffer[3] = 0x01;
	buffer[4] = FLAG;
	buffer[5] = '\0';

	write(fd, buffer, 6);
	printf("write # %d\n", conta);

	flag = 1;
	conta++;
}

int main(int argc, char **argv)
{
	int c, res;
	struct termios oldtio, newtio;
	char buf[255];
	int i, sum = 0, speed = 0;

	if ((argc < 2) ||
		((strcmp("/dev/ttyS10", argv[1]) != 0) &&
		 (strcmp("/dev/ttyS11", argv[1]) != 0)))
	{
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	/*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

	fd = open(argv[1], O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		perror(argv[1]);
		exit(-1);
	}

	if (tcgetattr(fd, &oldtio) == -1)
	{ /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 5;	/* blocking read until 5 chars received */

	/*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) == -1)
	{
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	(void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao

	buffer[0] = FLAG;
	buffer[1] = ENDERECOEMISSOR;
	buffer[2] = SET;
	buffer[3] = FLAG ^ ENDERECOEMISSOR ^ SET;
	buffer[4] = FLAG;
	buffer[5] = '\0';

	write(fd, buffer, 6);

	printf("write # %d\n", conta);
	conta++;

	while (conta < 5)
	{
		if (flag)
		{
			alarm(3); // activa alarme de 3s
			flag = 0;
			res = read(fd, buffer, 6);
			printf("Read %d bytes, %x %x %x %x %x\n", res, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);

			if (buffer[0] == FLAG && buffer[4] == FLAG && buffer[1] == ENDERECORECETOR)
			{
				break;
			}
		}
	}

	if (buffer[0] != FLAG || buffer[4] != FLAG || buffer[1] != ENDERECORECETOR)
	{
		printf("Couldn't receive response\n");
	}

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
	{
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
	return 0;
}
