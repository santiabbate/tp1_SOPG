#include "writer.h"

extern const char* fifoName;

/* Variables globales para flags de SIGUSR1 y SIGUSR2 */
volatile sig_atomic_t siguser1;
volatile sig_atomic_t siguser2;

/* Flag de salida */
volatile sig_atomic_t exitFlag;

/* Handler para SIGINT, actualizo un flag para indicar la salida del programa */
static void writerExitHandler(int sig);

/* Handler para SIGUSR1 y SIGUSR2 */
static void siguser_handle(int sig);

/* Escritura de un string por file descripor de la FIFO */
static void fifoWrite(int32_t fd, char* buffer);

/* Agrega el header a los datos según corresponda (Datos o Señal) */
static void appendHeader(char* receivedInput, char* outputBuffer, inputType_e type);

/* Inicializo la estrctura del signal handler */
void sigActionInit(struct sigaction * sa, int signum);

/* Writer: Llamado desde el proceso padre */
void writer(const char* fifoName, pid_t readerPid)
{    
    /* Inicialización de señal para salida del proceso, usando SIGINT */
    exitFlag = 0;

    struct sigaction exitSignal;
	
    exitSignal.sa_handler = writerExitHandler;
	exitSignal.sa_flags = 0;
	sigemptyset(&exitSignal.sa_mask);

	sigaction(SIGINT,&exitSignal,NULL);


    /* Inicialización de señales SIGUSR1 y SIGUSR2 */
    siguser1 = 0;
    siguser2 = 0;

    struct sigaction saUser1;
    struct sigaction saUser2;

    sigActionInit(&saUser1, SIGUSR1);
    sigActionInit(&saUser2, SIGUSR2);

    int readerExit;

    /* Inicialización de fifo */
    char receivedInput[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE];
	
	int32_t returnCode, fd;

    /* Creo fifo */
    if ( (returnCode = mknod(fifoName, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("WRITER: Error creando fifo: %d\n", returnCode);
        exitFlag = 1;
    }

    /* Abro file descriptor de la fifo en modo escritura */
    if ( (fd = open(fifoName, O_WRONLY) ) < 0 )
    {
        printf("WRITER: Error abriendo fifo: %d\n", fd);
        exit(1);
    }

    /* Loop hasta recibir flag de salida */
	while (!exitFlag)
	{
        /* Lectura del caraceteres por consola */
        /* Descarto si la lectura retornó error por llegada de una señal */
		if (NULL != fgets(receivedInput, BUFFER_SIZE, stdin))
        {
            appendHeader(receivedInput, outputBuffer, TEXT_E);
            fifoWrite(fd,outputBuffer);
        }

        if (siguser1)
        {
            appendHeader("1\n", outputBuffer, SIGNAL_E);
            fifoWrite(fd,outputBuffer);
            siguser1 = 0;
        }

        if (siguser2)
        {
            appendHeader("2\n", outputBuffer, SIGNAL_E);
            fifoWrite(fd,outputBuffer);
            siguser2 = 0;
        }        
	}

    printf("WRITER: Exit signal\n");

    /**
     * El Writer, proceso padre, toma la señal SIGINT (Ctrl+C) por consola,
     * termina la ejecución del loop principal y reenvía SIGINT al Reader,
     * proceso hijo. Se queda esperando con wait a la salida del reader     * 
     */
    kill(readerPid, SIGINT);
    wait(&readerExit);
    exit(1);
}


/* Handler para SIGINT, actualizo un flag para indicar la salida del programa */
void writerExitHandler(int sig)
{
	exitFlag = 1;
}

/* Handler para SIGUSR1 y SIGUSR2 */
void siguser_handle(int sig)
{
    if (sig == SIGUSR1)
    {
        siguser1 = 1;
    }
    else if (sig == SIGUSR2)
    {
        siguser2 = 1;
    } 
}

/* Escritura de un string por file descripor de la FIFO */
void fifoWrite(int32_t fd, char* buffer)
{    
    uint32_t bytesWrote;
    if ((bytesWrote = write(fd, buffer, strlen(buffer)-1)) == -1)
    {
        perror("write");
    }
    else
    {
        printf("Writer: Escribí %d bytes\n", bytesWrote);
    }
}

/* Agrega el header a los datos según corresponda (Datos o Señal) */
void appendHeader(char* receivedInput, char* outputBuffer, inputType_e type){
    const char* data = "DATA:";
    const char* signal = "SIGN:";
    memset(outputBuffer,0,BUFFER_SIZE);
    if (type == TEXT_E)
    {
        strncat(outputBuffer, data, BUFFER_SIZE);
    }
    else
    {
        strncat(outputBuffer, signal, BUFFER_SIZE);
    }
    strncat(outputBuffer, receivedInput, BUFFER_SIZE);
    // printf("%s\n",outputBuffer);
}


/* Inicializo la estrctura del signal handler */
void sigActionInit(struct sigaction * sa, int signum)
{
    sa->sa_handler = siguser_handle;
    /* Configuro en cero para no bloquear en la recepción de caracteres con fgets */
	sa->sa_flags = 0; //SA_RESTART; //0;
	sigemptyset(&sa->sa_mask);

	sigaction(signum,sa,NULL);
}