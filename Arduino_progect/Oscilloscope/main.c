#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <util/delay.h>
#include "../avr_common/uart.h"

// Struct per la ricezione dei dati
typedef struct {
  uint16_t sample_frequency;
  uint8_t channels;
  uint8_t mode;
} __attribute__((packed)) __serial_data__;

// Variabili globali
volatile __serial_data__ rcv = {0};
volatile uint8_t current_channel = 0;
volatile uint16_t adc_values[8];
volatile bool start_sampling = false;
volatile bool arrived_struct = false;
volatile uint8_t intensity = 0;

void timer1_init(void);
void adc_init(void);
ISR(TIMER1_COMPA_vect);
ISR(ADC_vect);

void adc_init(void) {
  ADMUX = (1 << REFS0); // Usa AVCC come riferimento di tensione
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Abilita ADC e prescaler 128, 125 kHz di frequenza di campionamento
  ADCSRA |= (1 << ADIE); // Abilita l'interrupt ADC
}

// Inizializzazione del Timer2 per generare interrupt alla sample frequency
void timer2_init(void) {
    // Imposta Timer2 in modalità CTC
    TCCR2A |= (1 << WGM21);
    // Calcola il valore di comparazione per la frequenza di campionamento
    OCR2A = (F_CPU / (1024 * rcv.sample_frequency) - 1);
    // Abilita l'interrupt di confronto A
    TIMSK2 |= (1 << OCIE2A);
    // Abilita Timer2 con prescaler 1024
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
}

// Init Timer1 per generare interrupt variabi
void timer1_init(void) {
  // Imposta il Timer1 in modalità CTC
  TCCR1A = (1<<WGM10)|(1<<COM1C0)|(1<<COM1C1);
  // Pulisce i registri di confronto
  OCR1AH=0;
  OCR1BH=0;
  OCR1CH=0;
  OCR1CL=1;
  // Abilita gli interrupt di confronto C
  TIMSK1 |= (1 << OCIE1C);
  // Abilita il Timer1 con prescaler 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
}

// Interrupt del Timer1 per cambiare il duty cycle
ISR(TIMER1_COMPC_vect) {
  intensity+=1;
}

// Ogni interrupt del Timer2 cambia il canale ADC e avvia una conversione
ISR(TIMER2_COMPA_vect) {
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
  uint8_t buffer[7*20];
  int size=0;

  // Il LED è collegato al pin 13 dell'Arduino
  // è il bit 7 della porta B
  const uint8_t mask=(1<<7);
  // pin 13 come output
  DDRB |= mask;//mask;

  UART_init(); // Inizializzazione per la UART

  while (1) {
    OCR1CL=intensity; // vario il duty cycle

    if(!arrived_struct){
      // Ricezione della struct
      UART_getString((uint8_t*)buffer);
      rcv.sample_frequency = *((uint16_t*)&buffer);
      rcv.channels = buffer[2];
      rcv.mode = buffer[3];

      adc_init(); // Inizializzazione dell'ADC
      timer1_init(); // Inizializzazione del Timer1
      timer2_init(); // Inizializzazione del Timer2
      sei();
      arrived_struct = true;
      memset(buffer, 0, sizeof(buffer));
      start_sampling = false;
    }

    //Streaming mode
    if(rcv.mode==2) {
      // // Invio i valori ADC via UART
      for (uint8_t i = 0; i < rcv.channels; i++) {
        sprintf(buffer, "%u %u\n", i, adc_values[i]);
        UART_putString((uint8_t*)buffer);
        memset(buffer, 0, sizeof(buffer));
      }
    }else{
      // Buffer mode
      // Si attende un comando via UART per avviare il sampling
      if(!start_sampling){
        UART_getString((uint8_t*)buffer);
        memset(buffer, 0, sizeof(buffer));
        start_sampling=true;
      }
      if(size+(int)rcv.channels*7<sizeof(buffer)){
        // Salvataggio dei valori ADC in un buffer per poi inviarli via UART quando pieno
        for (uint8_t i = 0; i < rcv.channels; i++) {
          // Aggiungo i valori al buffer
          size += sprintf(&buffer[size], "%u %u\n", i, adc_values[i]);
        }
      }else{
        // Invio del buffer via UART quando pieno, ressentando il buffer appena inviato
        UART_putString((uint8_t*)buffer);
        memset(buffer, 0, sizeof(buffer));
        size = 0;
      }
    }
  }
}