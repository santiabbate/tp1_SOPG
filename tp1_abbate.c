#include "common.h"

#include "reader.h"
#include "writer.h"


int main(void)
{
	pid_t pid;
	  
    const char* fifoName = "tp1_fifo";
       
    switch(pid = fork())
	{
		case -1:
		perror("fork");
		exit(1);
		break;

		/* Proceso hijo: Reader */
		case 0:
		printf("Proceso Reader\n");
        reader(fifoName);
		break;

        /* Proceso padre: Writer */ 
		default:
		printf("Proceso Writer\n");
        writer(fifoName, pid);

	}
}