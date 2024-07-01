#include "serial_linux.h"

// FIle su cui stampare il risultato
FILE * file_pointer;
const char *file_name1 = "output.plt";
int fd;

// Funzione per stampare la percentuale rimanente per il sampling
void printProgressBar(int currentIteration, int totalIterations) {
    int barWidth = 20; // Larghezza della barra di caricamento
    float progress = (float)currentIteration / totalIterations;
    int pos = barWidth * progress;

    printf("[");
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %d%%\r", (int)(progress * 100));
    fflush(stdout);
}

// Funzione per gestire il segnale SIGINT
void handle_sigint(int sig) {
  printf("\nCaught signal %d (SIGINT). Exiting gracefully...\n", sig);

  // Invio del terminatore al file di output
  char * terminator = "\0";
  size_t written = write(fd, &terminator, sizeof(char));
  
  if (written != sizeof(char)) {
    perror("Error writing to file");
  }

  // Chiusura del file di output
  fprintf(file_pointer,"e\n\n"); 
  fclose(file_pointer);
  exit(0);
}

/*
  main <serial_file> <baudrate> <sampling_frequency> <cannel> <time_sampling>
*/

int main(int argc, const char** argv) {
  if (argc<6) {
    printf("serial_linux <serial_file> <baudrate> <sampling_frequency> <cannel> <time_sampling>\n");
  }
  const char* serial_device=argv[1];
  int baudrate=atoi(argv[2]);
  int sampling_frequency=atoi(argv[3]);
  int channel=atoi(argv[4]);
  int time_sampling=atoi(argv[5]);
  int current_time = time_sampling;

  if(channel>8){
    printf("Il numero di canali deve essere minore di 8\n");
    return 1;
  }
  if(sampling_frequency>65536){
    printf("La frequenza di campionamento deve essere minore di 65536\n");
    return 1;
  }

  // Struct utilizzata per mandare i dati all'arduino 
  __serial_data__ send = {0};
  send.sample_frequency = (uint16_t) sampling_frequency;
  send.channels = (uint8_t) channel;

  // Apre il file in modalità scrittura ("w")
  file_pointer = fopen(file_name1, "w");

  // Verifica se l'apertura del file è avvenuta correttamente
  if (file_pointer == NULL) {
      char * message = "Errore durante l'apertura dei file.\n";
      printf("%s",message);
      return 1;
  }

  fprintf(file_pointer,"plot '-' w l\n"); // Preparazione del file di output

  // Apre la porta seriale e setta i parametri
  fd=serial_open(serial_device);
  serial_set_interface_attribs(fd, baudrate, 0);
  serial_set_blocking(fd, 1);

  // Gestione del segnale SIGINT
  if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    perror("signal install failed");
    exit(EXIT_FAILURE);
  }

  while(1) {

    // Preparazione del buffer di ricezione
    char buf[1024];
    memset(buf, 0, 1024);

    if(current_time == time_sampling){
      // Invio della struct all'Arduino
      size_t written = write(fd, &send, sizeof(__serial_data__));
      if (written != sizeof(__serial_data__)) {
        perror("Error writing to file");
        return -1;
      }
    }else{
      // In modalità streaming vengono salvati i dati direttamente sul file output.plt
      printProgressBar(current_time,time_sampling);
    }

    // Lettura dei bytes provenienti dall'Arduino
    size_t bytes_read = read(fd, buf,1024);
    if (bytes_read == 0) {
      perror("Error reading from file");
      return -1;
    }
    
    // Scrittura dei bytes provenienti dall'Arduino sul file di output 
    int bytes_write = fputs(buf,file_pointer);
    if (bytes_write == EOF) {
      perror("Error writing to output file");
      return -1;
    }

    current_time--;
  }
  
  exit(EXIT_SUCCESS);
}
