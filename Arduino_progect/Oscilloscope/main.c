#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../avr_common/uart.h"

// configuration bits for PWM
// fast PWM, 8 bit, non inverted
// output compare set low
#define TCCRA_MASK (1<<WGM10)|(1<<COM1C0)|(1<<COM1C1)
#define TCCRB_MASK ((1<<WGM12)|(1<<CS10))   


// our interrupt routine installed in
// interrupt vector position
// corresponding to output compare
// of timer 5
ISR(TIMER5_COMPA_vect) {

}

// Inizializzazione del timer5
void timer5_init(int timer_duration_ms){
  // configure timer
  // set the prescaler to 1024
  TCCR5A = 0;
  TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52); 
  
  // at this count rate
  // 1 ms will correspond do 15.62 counts
  // we set the output compare to an appropriate
  // value so that when the counter reaches that value
  // the interrupt will be triggered
  uint16_t ocrval=(uint16_t)(((F_CPU / 1024) * (timer_duration_ms/1000)) - 1);

  OCR5A = ocrval;

  // clear int
  cli();
  TIMSK5 |= (1 << OCIE5A);  // enable the timer interrupt
  // enable int
  sei();
}

void timer1_init(){
  // we will use timer 1
  TCCR1A=TCCRA_MASK;
  TCCR1B=TCCRB_MASK;
  // clear all higher bits of output compare for timer
  OCR1AH=0;
  OCR1BH=0;
  OCR1CH=0;
  OCR1CL=1;
}


int main(void){
  printf_init(); // Inizializzazione per la UART
  timer1_init(); // Inizializzazione del Timer1

  // the LED is connected to pin 13
  // that is the bit 5 of port b, we set it as output
  const uint8_t mask=(1<<7);
  // we configure the pin as output
  DDRB |= mask;//mask;

  int sampling_frequenzy;
  int cannel;
  int total_time;

  // Preparazione del buffer di ricezione/invio
  char buf[1024];
  memset(buf, 0, 1024);
  int l=0;
  do{
    buf[l] = usart_getchar();
  }while(buf[l++]!=0);

  // Vengono letti e salvati i valori trasmessi dal PC
  sscanf(buf, "%d %d %d", &sampling_frequenzy, &cannel);

  uint8_t intensity=0;
  while(1){

    // we write on the output compare register a value
    // that will be proportional to the opposite of the
    // duty_cycle

    OCR1CL=intensity; 

    memset(buf, 0, 1024);
    sprintf(buf,"%d\n",(int) OCR1CL);
    usart_pstr((uint8_t*)buf);

    _delay_ms(100); // from delay.h, wait 1 sec
    intensity+=2;
  }
}
  