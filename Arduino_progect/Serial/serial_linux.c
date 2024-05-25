#include "serial_linux.h"

// FIle su cui stampare il risultato
FILE * file_pointer;
const char *file_name1 = "output.plt";


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



int serial_set_interface_attribs(int fd, int speed, int parity) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
    printf ("error %d from tcgetattr", errno);
    return -1;
  }
  switch (speed){
  case 19200:
    speed=B19200;
    break;
  case 57600:
    speed=B57600;
    break;
  case 115200:
    speed=B115200;
    break;
  case 230400:
    speed=B230400;
    break;
  case 576000:
    speed=B576000;
    break;
  case 921600:
    speed=B921600;
    break;
  default:
    printf("cannot sed baudrate %d\n", speed);
    return -1;
  }
  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);
  cfmakeraw(&tty);
  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);               // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;      // 8-bit chars

  if (tcsetattr (fd, TCSANOW, &tty) != 0) {
    printf ("error %d from tcsetattr", errno);
    return -1;
  }
  return 0;
}

void serial_set_blocking(int fd, int should_block) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
      printf ("error %d from tggetattr", errno);
      return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    printf ("error %d setting term attributes", errno);
}

int serial_open(const char* name) {
  int fd = open (name, O_RDWR | O_NOCTTY | O_SYNC );
  if (fd < 0) {
    printf ("error %d opening serial, fd %d\n", errno, fd);
  }
  return fd;
}

/*
  serial_linux <serial_file> <baudrate> <sampling_frequency> <cannel> <time_sampling>
*/

int main(int argc, const char** argv) {
  if (argc<6) {
    printf("serial_linux <serial_file> <baudrate> <sampling_frequency> <cannel> <time_sampling>\n");
  }
  const char* serial_device=argv[1];
  int baudrate=atoi(argv[2]);
  int sampling_frequenzy=atoi(argv[3]);
  int cannel=atoi(argv[4]);
  int time_sampling=atoi(argv[5]);
  int current_time = time_sampling;

  // Apre il file in modalità scrittura ("w")
  file_pointer = fopen(file_name1, "w");

  // Verifica se l'apertura del file è avvenuta correttamente
  if (file_pointer == NULL) {
      char * message = "Errore durante l'apertura dei file.\n";
      printf("%s",message);
      return 1;
  }

  fprintf(file_pointer,"plot '-' w l\n"); // Preparazione del file di output

  int fd=serial_open(serial_device);
  serial_set_interface_attribs(fd, baudrate, 0);
  serial_set_blocking(fd, 1);

  while(current_time>0) {

    // Preparazione del buffer di ricezione/invio
    char buf[1024];
    memset(buf, 0, 1024);

    if(current_time == time_sampling){
      fgets(buf, sizeof(buf), stdin);
      int l=strlen(buf);
      buf[l]='\0';
      ++l;
      write(fd, buf, l);
    }else{
      printProgressBar(current_time,time_sampling);
    }

    read(fd, buf,1024);
    fprintf(file_pointer,buf);

    current_time--;
  }

  fprintf(file_pointer,"e\n\n"); // Chiusura del file di output
  
}
