# Serial Communication Program

This program is designed to facilitate serial communication between a computer and an external device, such as an Arduino. It is written in C and provides a simple interface for sending and receiving data.

## Features

- **Buffer Mode Activation**: The program includes a buffer mode that waits for the user to press Enter before starting data receiving samples from Arduino on channels secified in argv.
- **Streaming Mode**: After sending the struct that include all parameters specified on args, the program became ready to receive samples.
- **Data Processing**: After reading the data, it populates files, ones per channels, with the received data, allowing for further processing or analysis.

## Usage

1. Ensure that the device you wish to communicate with is properly connected to your computer.
2. Compile the `main.c` file using a C compiler, ensuring that any necessary libraries for serial communication are included.
3. Run the compiled program. 
    ```bash
        ./main <serial_file> <baudrate> <sampling_frequency> <channel> <bufferd_mode 1 : streaming_mode 2>
    ```
4. The program will notify you when it starts receiving data.
5. Press Ctrl+C to terminate the sampling and save all data.

## Requirements

- A C compiler (e.g., GCC)
- Access to serial communication libraries and headers appropriate for your operating system
- A connected device capable of serial communication (e.g., Arduino)