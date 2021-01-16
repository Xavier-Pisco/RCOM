all: emissor recetor

emissor: emissor.c
	gcc -o emissor emissor.c utils.c protocolo_emissor.c -Wall -Werror

recetor: recetor.c
	gcc -o recetor recetor.c utils.c protocolo_recetor.c -Wall -Werror

clean:
		rm emissor recetor
