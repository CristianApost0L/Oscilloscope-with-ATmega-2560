## Project : Oscilloscope

### Objective.

 To make an oscilloscope.

### PC side.

   It must be possible to:
   - [X] Send information such as sampling rate and number of channels.
   - [X] Receive data from the Arduino via serial.

### ATmega2560 side

   It must be possible to operate in two modes:
   - [X] continuous sampling 
   - [X] buffering the data to then be sent to burst

All communication will be done asynchronously via UART.
