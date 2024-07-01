#include "serial_linux.h"

// FIle su cui stampare il risultato
FILE * file_pointer;
const char *file_name1 = "output.plt";
int fd;

// Funzione per gestire il segnale SIGINT
void handle_sigint(int sig) {
  printf("\nCaught signal %d (SIGINT). Exiting gracefully...\n", sig);
  close(fd);
  // Chiusura del file di output
  fprintf(file_pointer,"e\n\n"); 
  fclose(file_pointer);
  exit(0);
}

/*
  main <serial_file> <baudrate> <sampling_frequency> <channel> <mode>
*/

int main(int argc, const char** argv) {
  if (argc<6) {
    printf("serial_linux <serial_file> <baudrate> <sampling_frequency> <channel> <bufferd_mode 1 : 0>\n");
  }

  const char* serial_device=argv[1];
  int baudrate=atoi(argv[2]);
  int sampling_frequency=atoi(argv[3]);
  int channel=atoi(argv[4]);
  int buffer_mode=atoi(argv[5]);

  // Verifica che i parametri siano corretti
  assert(channel>0 && channel<=8);
  assert(sampling_frequency>0 && sampling_frequency<=65536);
  assert(buffer_mode==0 || buffer_mode==1);

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

  // Struct utilizzata per mandare i dati all'arduino 
  __serial_data__ send = {0};
  send.sample_frequency = (uint16_t) sampling_frequency;
  send.channels = (uint8_t) channel;
  send.mode = (uint8_t) buffer_mode;

  // Variabile per controllare se l'invio è già stato effettuato
  bool start=false;

  // Variabile per controllare se è stato inviato il carattere per iniziare il sampling
  bool start_sampling = false;

  while(1) {

    // Preparazione del buffer di ricezione
    char buf[1024] = {0};

    if(!start){
      usleep(5000);
      memcpy(buf,&send,sizeof(__serial_data__));
      // Invio della struct all'Arduino
      size_t written = write(fd, &buf, sizeof(__serial_data__)+1);
      if (written != sizeof(__serial_data__)+1) {
        perror("Error writing to file");
        return -1;
      }
      start=true;
    }

    // Se il buffer mode è attivo, aspetta che l'utente invii un carattere per iniziare il sampling
    if(send.mode && !start_sampling){
      printf("Enter a character to start sampling: ");
      char c = getchar();
      size_t written = write(fd, &c, 1);
      if (written != 1) {
        perror("Error writing to file");
        return -1;
      }
      start_sampling = true;

    }else{

      usleep(5000);

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
    }
  }
  
  exit(EXIT_SUCCESS);
}
