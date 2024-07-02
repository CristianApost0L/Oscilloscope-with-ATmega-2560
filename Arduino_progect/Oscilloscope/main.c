#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../avr_common/uart.h"

// Struct per la ricezione dei dati
typedef struct {
  uint16_t sample_frequency;
  uint8_t channels;
  uint8_t mode;
} __attribute__((packed)) __serial_data__;
volatile __serial_data__ rcv;

// Variabili globali
volatile uint8_t current_channel = 0;
volatile uint16_t adc_values[8];
volatile bool start_sampling = false;
volatile bool arrived_struct = false;

void timer1_init(void);
void adc_init(void);
ISR(TIMER1_COMPA_vect);
ISR(ADC_vect);

void adc_init(void) {
  ADMUX = (1 << REFS0); // Usa AVCC come riferimento di tensione
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Abilita ADC e prescaler 128, 125 kHz di frequenza di campionamento
  ADCSRA |= (1 << ADIE); // Abilita l'interrupt ADC
}

// Function to initialize Timer1, used update current reading every second
void timer1_init(void) {
  // Set CTC mode (Clear Timer on Compare Match)
  TCCR1B |= (1 << WGM12);
  // Set compare value for 1 Hz interrupt (assuming 16 MHz clock and 1024 prescaler)
  OCR1A = (F_CPU / (1024 * rcv.sample_frequency) - 1);
  // Enable Timer1 compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  // Start Timer1 with 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
}

// Ogni interrupt del Timer1 cambia il canale ADC e avvia una conversione
ISR(TIMER1_COMPA_vect) {
  // Cambia il canale ADC
  ADMUX = (ADMUX & 0xF8) | (current_channel & 0x07);
  ADCSRA |= (1 << ADSC); // Avvia una conversione ADC
}

// Lettura ADC una volta che la conversione è completata
ISR(ADC_vect) {
  adc_values[current_channel] = ADC; // Leggi il valore convertito
  current_channel = (current_channel + 1) % rcv.channels; // Passa al prossimo canale
}

// In buffer mode tramite UART viene avviato il sampling
ISR(USART0_RX_vect) {
  start_sampling = true;
}

int main(void){
  uint8_t buffer[1000];
  uint8_t buffer_2[10];
  int size = 0;

  UART_init(); // Inizializzazione per la UART

  while (1) {

    if(!arrived_struct){
      // Ricezione della struct
      UART_getString((uint8_t*)buffer);
      rcv.sample_frequency = *((uint16_t*)&buffer);
      rcv.channels = buffer[2];
      rcv.mode = buffer[3];

      adc_init(); // Inizializzazione dell'ADC
      timer1_init(); // Inizializzazione del Timer1
      sei();
      arrived_struct = true;
      memset(buffer, 0, sizeof(buffer));
    }

    // Se il buffer mode è attivo, aspetta che l'utente invii qualcosa per iniziare il sampling
    if(rcv.mode != 0) {
      start_sampling = false;
    }

    //Streaming mode
    if(!rcv.mode) {
      // Invio i valori ADC via UART
      for (uint8_t i = 0; i < rcv.channels; i++) {
        sprintf(buffer, "%d %u\n", i, adc_values[i]);
        UART_putString((uint8_t*)buffer);
        memset(buffer, 0, sizeof(buffer));
      }
    }else{
    // Buffer mode
      if(start_sampling) {
        // Salvataggio dei valori ADC in un buffer per poi inviarli via UART quando pieno
        for (uint8_t i = 0; i < rcv.channels; i++) {
          if(size>1000) {
            break;
          }
          // Aggiungo i valori al buffer_2
          sprintf(buffer_2, "%d %u\n", i, adc_values[i]);
          buffer_2[6] = '\0';
          // Aggiungo i valori al buffer
          size+=strlen(buffer_2)+1;
          strcat(buffer, buffer_2);
          memset(buffer_2, 0, sizeof(buffer_2));
        }
        // Invio del buffer via UART quando pieno
        UART_putString((uint8_t*)buffer);
        memset(buffer, 0, sizeof(buffer));
        size = 0;
      }
    }
  }
}