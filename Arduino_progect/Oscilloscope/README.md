# ADC Configuration and Sampling on ATmega 2560

## Introduction
This guide explains how to configure the ADC (Analog-to-Digital Converter) on an ATmega microcontroller, set up sampling with interrupt service routines (ISRs), and control the sampling frequency and interval.

## ADC Initialization
To initialize the ADC, use the following function:

```c
void adc_init(void) {
    ADMUX = (1 << REFS0); // Use AVCC as the reference voltage
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC and set prescaler to 128 (125 kHz sampling frequency)
    ADCSRA |= (1 << ADIE); // Enable ADC interrupt
}
```

## Explanation of the Chosen Values for ADC

ADMUX (ADC Multiplexer Selection Register):

```c
ADMUX = (1 << REFS0);
```

- `REFS0`: If set, uses AVCC as the reference voltage for the ADC. AVCC is typically connected to a stable power supply voltage, usually 5V or 3.3V, depending on the microcontroller and system configuration.

ADCSRA (ADC Control and Status Register A):

```c
ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
```
- `ADEN` (ADC Enable): Enables the ADC.
- `ADPS2`, `ADPS1`, `ADPS0`: These bits set the ADC prescaler, which divides the CPU clock frequency to obtain a suitable ADC sampling frequency.
- `ADPS2 | ADPS1 | ADPS0 = 111` means the prescaler is 128.
- `ADCSRA |= (1 << ADIE)` enables the ADC interrupt. This allows the microcontroller to execute an interrupt service routine (ISR) when an ADC conversion is complete.

You can change the sampling frequency by modifying the prescaler value. Here are the possible prescaler values and the corresponding sampling frequencies if the CPU frequency is 16 MHz:

| Prescaler | ADPS2 | ADPS1 | ADPS0 | Sampling Frequency  |
|-----------|-------|-------|-------|---------------------|
| 2         | 0     | 0     | 1     | 8 MHz               |
| 4         | 0     | 1     | 0     | 4 MHz               |
| 8         | 0     | 1     | 1     | 2 MHz               |
| 16        | 1     | 0     | 0     | 1 MHz               |
| 32        | 1     | 0     | 1     | 500 kHz             |
| 64        | 1     | 1     | 0     | 250 kHz             |
| 128       | 1     | 1     | 1     | 125 kHz             |


ADC Frequency = CPU Frequency / Prescaler
                            = 16 MHz / 128
                            = 125 kHz

## Timer2 Initialization
The `timer2_init` function configures Timer2 to generate interrupts at a specified sampling frequency.

```c
void timer2_init(void) {
    // Set Timer2 to CTC mode
    TCCR2A |= (1 << WGM21);
    // Calculate the compare value for the sampling frequency
    OCR2A = (F_CPU / (1024 * rcv.sample_frequency) - 1);
    // Enable compare match A interrupt
    TIMSK2 |= (1 << OCIE2A);
    // Enable Timer2 with prescaler 1024
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
}
```

## Explanation of Chosen Values for Timer2
TCCR2A (Timer/Counter Control Register A):

```c
TCCR2A |= (1 << WGM21);
```
- `WGM21`: Sets Timer2 to CTC (Clear Timer on Compare Match) mode. In this mode, the timer resets when the counter reaches the value in the OCR2A register.

OCR2A (Output Compare Register A):

```c
OCR2A = (F_CPU / (1024 * rcv.sample_frequency) - 1);
```
- `OCR2A`: This register holds the compare value that determines the interrupt frequency. The value is calculated based on the CPU clock frequency, the prescaler, and the desired sampling frequency (`rcv.sample_frequency`).
- `F_CPU`: CPU frequency.
- `1024`: The prescaler used.
- `rcv.sample_frequency`: The desired sampling frequency.

TIMSK2 (Timer/Counter Interrupt Mask Register):

```c
TIMSK2 |= (1 << OCIE2A);
```
- `OCIE2A`: Enables the compare match A interrupt for Timer2.

TCCR2B (Timer/Counter Control Register B):

```c
TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
```
- `CS22`, `CS21`, `CS20`: Set the prescaler for Timer2 to 1024. This prescaler divides the CPU clock frequency by 1024.

You can change the interrupt frequency by modifying the prescaler value. Here are the possible prescaler values and the corresponding interrupt frequencies if the CPU frequency is 16 MHz:

| CS22 | CS21 | CS20 | Prescaler | Interrupt Frequency       |
|------|------|------|-----------|---------------------------|
| 0    | 0    | 0    | 1         | 16 MHz                    |
| 0    | 0    | 1    | 8         | 2 MHz                     |
| 0    | 1    | 0    | 32        | 500 kHz                   |
| 0    | 1    | 1    | 64        | 250 kHz                   |
| 1    | 0    | 0    | 128       | 125 kHz                   |
| 1    | 0    | 1    | 256       | 62.5 kHz                  |
| 1    | 1    | 0    | 1024      | 15.625 kHz                |


## Interrupt Service Routines (ISRs)

### Timer2 Compare Match ISR
This ISR is called every time Timer2 reaches the value in the OCR2A register. It changes the ADC channel and starts a new ADC conversion.

```c
ISR(TIMER2_COMPA_vect) {
  // Change the ADC channel
  ADMUX = (ADMUX & 0xF8) | (current_channel & 0x07);
  ADCSRA |= (1 << ADSC); // Start a new ADC conversion
}
```
### ADMUX register
ADMUX is an 8-bit register that contains various configuration bits for the ADC. The most significant bits (bits 6 and 7) are used to select the voltage reference, while the least significant bits (bits 0-3) are used to select the ADC channel.

#### ADMUX Registry Layout

```c
+---+---+---+---+---+---+---+---+
| REFS1 | REFS0 | ADLAR |  MUX4  | MUX3 | MUX2 | MUX1 | MUX0 |
+---+---+---+---+---+---+---+---+
```

- `REFS1` and `REFS0`: They select the voltage reference.
- `ADLAR`: If enabled, the ADC result will be left-aligned.
- `MUX4`, `MUX3`, `MUX2`, `MUX1`, `MUX0`: They select the ADC input channel.

### Explaning

- `ADMUX & 0xF8`: This masks the three least significant bits of the ADMUX register. Basically, it keeps all bits unchanged except `MUX0-MUX2`, which are set to 0.
- `current_channel & 0x07`: This masks the current channel number bits. current_channel should be a value between 0 and 7 (for `ADC0-ADC7` channels). The mask 0x07 (binary 00000111) ensures that only the three least significant bits are considered, thus limiting the channels to 8 possibilities.

### ADC Conversion Complete ISR
This ISR is called when an ADC conversion is complete. It reads the converted value and advances to the next channel.

```c
ISR(ADC_vect) {
  adc_values[current_channel] = ADC; // Read the converted value
  current_channel = (current_channel + 1) % rcv.channels; // Advance to the next channel
}