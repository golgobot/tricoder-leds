#include <iostream>
#include <pigpio.h>
#include <thread>
#include <chrono>
#include <signal.h>
#include <array>

#define PI_INPUT 0
#define PI_OUTPUT 1
#define PI_LOW 0
#define PI_HIGH 1
#define SPEED 1000000
#define SPI_CS 8 // GPIO for slave select.

bool run = true;

void stop(int signum) {
    run = false;
}

unsigned MOSI = 10;
unsigned SCLK = 11;

int handle = 0;

rawSPI_t rawSPI = {
    .clk     =  11, // GPIO for SPI clock.
    .mosi    = 10, // GPIO for SPI MOSI.
    .ss_pol  =  1, // Slave select resting level.
    .ss_us   =  1, // Wait 1 micro after asserting slave select.
    .clk_pol =  0, // Clock resting level.
    .clk_pha =  0, // 0 sample on first edge, 1 sample on second edge.
    .clk_us  =  1, // 2 clocks needed per bit so 500 kbps.
};

void write(char reg, char value) {
    auto result = spiWrite(handle, new char[2]{reg, value}, 2);
    switch(result) {
        case PI_BAD_HANDLE:
            std::cout << "Can't write: bad handle." << std::endl;
            return exit(1);
        case PI_BAD_SPI_COUNT:
            std::cout << "Can't write: bad SPI count." << std::endl;
            return exit(1);
        case PI_SPI_XFER_FAILED:
            std::cout << "Can't write: SPI xfer failed." << std::endl;
            return exit(1);
        default:
            // no op
            break;
    }
}

int main() {
    if (gpioInitialise() < 0) {
        return 1;
    }
    gpioSetSignalFunc(SIGINT, stop);
    gpioSetMode(rawSPI.clk,  PI_OUTPUT);
    gpioSetMode(rawSPI.mosi, PI_OUTPUT);

    handle = spiOpen(0, SPEED, 0);

    switch(handle) {
        case PI_BAD_SPI_CHANNEL:
            std::cout << "Bad SPI Channel" << std::endl;
            return 1;
        case PI_BAD_SPI_SPEED:
            std::cout << "Bad SPI Speed" << std::endl;
            return 1;
        case PI_BAD_FLAGS:
            std::cout << "Bad Flags" << std::endl;
            return 1;
        case PI_NO_AUX_SPI:
            std::cout << "No Aux SPI" << std::endl;
            return 1;
        case PI_SPI_OPEN_FAILED:
            std::cout << "Open Failed" << std::endl;
            return 1;
        default:
            std::cout << "Handle Opened" << std::endl;
    }
    if(handle < 0) {
        std::cerr << "Could not open SPI interface" << std::endl;
        return 1;
    }

    // zero everything
    for(int i = 0; i < 16; i++) {
        write(i, 0);
    }

    // Set scan limit register
    write(0b1011,0b00000111);
    // Set shutdown register
    write(0b1100,1);
    while(run) {
        for(int i = 1; i <= 8 && run; i++) {
            for (int j = 0; j < 8 && run; j++) {
                unsigned one = 1;
                write(i, one << j);
                time_sleep(1.0 / 20);
            }
            write(i, 0);
        }
    }

    gpioTerminate();
    return 0;
}
