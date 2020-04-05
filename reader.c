#include "reader.h"

/* Flag de salida */
volatile sig_atomic_t exitFlag;

/* Handler para SIGINT, actualizo un flag para indicar la salida del programa */
void readerExitHandler(int sig);

/* Proceso el buffer recibido, divido la salida en archivos según Datos o -señales*/
void processBuffer(FILE* sigFd, FILE* logFd, char* buffer);

/* Reader: Proceso hijo */
void reader(const char* fifoName){
    
    /* Inicialización de señal para salida del proceso, usando SIGINT */
    exitFlag = 0;

    struct sigaction exitSignal;
	
    exitSignal.sa_handler = readerExitHandler;
	exitSignal.sa_flags = 0;
	sigemptyset(&exitSignal.sa_mask);

	sigaction(SIGINT,&exitSignal,NULL);

    /* Inicialización de fifo */
    char receivedBuffer[BUFFER_SIZE];
	
	int32_t returnCode, fd;

    /* Creo fifo */
    if ( (returnCode = mknod(fifoName, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("READER: Error creando fifo: %d\n", returnCode);
        exit(1);
    }

    /* Abro file descriptor de la fifo en modo lectura */
    if ( (fd = open(fifoName, O_RDONLY) ) < 0 )
    {
        printf("READER: Error abriendo fifo: %d\n", fd);
        exit(1);
    }
    
    FILE* sgnFile;
    FILE* logFile;
    
    sgnFile = fopen("Sign.txt","a+");
    logFile = fopen("Log.txt","a+");

    uint32_t bytesRead;

    while (!exitFlag)
    {
        if ((bytesRead = read(fd, receivedBuffer,BUFFER_SIZE)) == -1)
        {
            // perror("read");
        }
        else if (bytesRead != 0)
        {
            printf("Reader: Leí %d bytes\n", bytesRead);
            // printf("%s\n",receivedBuffer);
            // sgnFile = fopen("Sign.txt","a+");
            // logFile = fopen("Log.txt","a+");
            processBuffer(sgnFile, logFile, receivedBuffer);
  
        }
        else
        {
            printf("%d bytes\n", bytesRead);
        }
        memset(receivedBuffer,0,BUFFER_SIZE);
        
    }
    
    /* Si estoy acá es que recibí la señal SIGINT desde el Writer, proceso padre */
    /* Antes de salir cierro los archivos */
    fclose(sgnFile);
    fclose(logFile); 
    printf("Reader: Exit signal\n");
    
    exit(0);
}

/* Handler para SIGINT, actualizo un flag para indicar la salida del programa */
void readerExitHandler(int sig)
{
	exitFlag = 1;
}

/* Proceso el buffer recibido, divido la salida en archivos según Datos o -señales*/
void processBuffer(FILE* sigFd, FILE* logFd, char* buffer)
{
    const char* data = "DATA:";
    const char* signal = "SIGN:";

    if (0 == strncmp(data,buffer,strlen(data))) // Si recibo DATA:xxxxx
    {
        fwrite(buffer, 1, strlen(buffer), logFd); // Escribo al descriptor del log
        fwrite("\n", 1, 1, logFd);
    }
    else if (0 == strncmp(signal,buffer,strlen(signal))) // Si recibo SIGN:x
    {
        fwrite(buffer, 1, strlen(buffer), sigFd); // Escribo al descriptor de sign
        fwrite("\n", 1, 1, sigFd);
    }
}