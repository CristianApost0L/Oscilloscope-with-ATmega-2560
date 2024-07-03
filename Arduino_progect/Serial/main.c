#include "serial_linux.h"

// FIle su cui stampare il risultato
FILE * file_pointer[9];
const char *file_name1 = "./data/datafile1.dat";
const char *file_name2 = "./data/datafile2.dat";
const char *file_name3 = "./data/datafile3.dat";
const char *file_name4 = "./data/datafile4.dat";
const char *file_name5 = "./data/datafile5.dat";
const char *file_name6 = "./data/datafile6.dat";
const char *file_name7 = "./data/datafile7.dat";
const char *file_name8 = "./data/datafile8.dat";
const char *file_name9 = "./data/data.dat";
const char **filename[8] = {&file_name1, &file_name2, &file_name3, &file_name4, &file_name5, &file_name6, &file_name7, &file_name8};

int fd;

// Struct per l'invio dei dati all'Arduino
__serial_data__ send;

// Funzioni dichiarate
void populate_files();
void handle_sigint(int sig);

// Funzione per gestire il segnale SIGINT
void handle_sigint(int sig) {
  printf("\nCaught signal %d (SIGINT). Save sampling on files...\n", sig);
  close(fd);


  // Popola i file con i dati ricevuti
  populate_files();

  // Chiusura dei file di output e dei file dati
  for(int i=0; i<send.channels; i++){
    fclose(file_pointer[i]);
  }

  fclose(file_pointer[8]);

  exit(0);
}

// Funzione che popola i singoli file con i dati ricevuti
void populate_files() {

  for(int i=0; i<send.channels; i++){
    file_pointer[i] = fopen(*filename[i], "w");
      // Verifica se l'apertura dei file è avvenuta correttamente
    if(file_pointer[i] == NULL) {
      char * message = "Errore durante l'apertura dei file.\n";
      printf("%s",message);
      return;
    }
  }

  fclose(file_pointer[8]);
  file_pointer[8] = fopen(file_name9, "r");

  printf("Populating files...\n");

  char line[100];
  while (fgets(line, sizeof(line), file_pointer[8])) {
    int file_number, number;
    sscanf(line, "%d %d\n", &file_number, &number); // Scansiono la riga per ottenere il numero del canale e il valore del campionamento
    if(file_number < 0 || file_number >= send.channels) {
      continue; // Ignora i campioni non validi
    }
    float value = (float) number / 1024.0 * 5.0; // Conversione del valore del campione in tensione
    memset(line, 0, sizeof(line)); // Pulizia del buffer
    sprintf(line, "%f\n", value); // Scrittura del valore della tensione nel buffer
    // Scrittura del campione nel file del canale corrispondente
    int bytes_write = fputs(line,file_pointer[file_number]);
    if (bytes_write == EOF) {
      perror("Error writing to output file");
      return;
    }
  }
}

/*
  main <serial_file> <baudrate> <sampling_frequency> <channel> <mode>
*/

int main(int argc, const char** argv) {
  if (argc<6) {
    printf("serial_linux <serial_file> <baudrate> <sampling_frequency> <channel> <bufferd_mode 1 : 2>\n");
  }

  const char* serial_device=argv[1];
  int baudrate=atoi(argv[2]);
  int sampling_frequency=atoi(argv[3]);
  int channel=atoi(argv[4]);
  int buffer_mode=atoi(argv[5]);

  // Verifica che i parametri siano corretti
  assert(channel>0 && channel<=8);
  assert(sampling_frequency>0 && sampling_frequency<=65536);
  assert(buffer_mode==1 || buffer_mode==2);

  // Apre il file che raccoglierà tutti i dati in modalità scrittura ("w")
  file_pointer[8] = fopen(file_name9, "w");

  // Verifica se l'apertura dei file è avvenuta correttamente
  if(file_pointer[8] == NULL) {
    char * message = "Errore durante l'apertura dei file.\n";
    printf("%s",message);
    return 1;
  }

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
  send = (__serial_data__) {0};
  send.sample_frequency = (uint16_t) sampling_frequency;
  send.channels = (uint8_t) channel;
  send.mode = (uint8_t) buffer_mode;

  // Variabile per controllare se l'invio è già stato effettuato
  bool start=false;

  // Variabile per controllare se è stato inviato il carattere per iniziare il sampling nella buffer mode
  bool start_sampling = false;

  while(1) {

    // Preparazione del buffer di ricezione
    uint8_t buf[1024] = {0};

    if(!start){
      // Dorme per 1s
      usleep(1000000);

      printf("Sending data to Arduino...\n");

      memcpy(buf, &send, sizeof(__serial_data__));

      // Invio della struct all'Arduino
      size_t written = write(fd, &buf, sizeof(__serial_data__)+1);
      if (written != sizeof(__serial_data__)+1) {
        perror("Error writing to file");
        return -1;
      }
      printf("Send data...\n");
      start=true;
    }

    // Se il buffer mode è attivo, aspetta che l'utente invii un carattere per iniziare il sampling
    if(send.mode==1 && !start_sampling){
      printf("Press Enter to start sampling: ");
      char c = getchar();
      size_t written = write(fd, &c, 1);
      if (written != 1) {
        perror("Error writing to file");
        return -1;
      }
      start_sampling = true;

    }else{
      // Dorme per 5ms
      usleep(5000);

      // Lettura dei bytes provenienti dall'Arduino
      size_t bytes_read = read(fd, &buf,sizeof(buf));
      if (bytes_read == 0) {
        perror("Error reading from file");
        return -1;
      }
      
      // Scrittura dei bytes provenienti dall'Arduino sul file di output 
      int bytes_write = fputs((char*)buf,file_pointer[8]);
      if (bytes_write == EOF) {
        perror("Error writing to output file");
        return -1;
      }
    }
  }
  
  exit(EXIT_SUCCESS);
}
