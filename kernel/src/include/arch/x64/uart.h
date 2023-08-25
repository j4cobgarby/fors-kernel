#ifndef __INCLUDE_X64_UART_H__
#define __INCLUDE_X64_UART_H__

#include <stdlib.h>
#include <stdint.h>

// Status of the com port.
#define COM_PORT_STATUS_READY   0
#define COM_PORT_STATUS_BROKEN  1
#define COM_PORT_STATUS_BUSY    2

// Offsets to add to the com port base to access the various registers.
#define COM_PORT_OFFSET_INT_ID      2
#define COM_PORT_OFFSET_LINE_CTRL   3
#define COM_PORT_OFFSET_MODEM_CTRL  4
#define COM_PORT_OFFSET_LINE_STAT   5
#define COM_PORT_OFFSET_MODEM_STAT  6
#define COM_PORT_OFFSET_SCRATCH     7
// These are correct iff DLAB = 0
#define COM_PORT_OFFSET_DATA        0   
#define COM_PORT_OFFSET_INT_ENABLE  1
// These are correct iff DLAB = 1
#define COM_PORT_OFFSET_BAUD_DIVISOR_LOW    0
#define COM_PORT_OFFSET_BAUD_DIVISOR_HIGH   1

// Bit meanings in LINE_STAT register
#define LINE_STAT_DR 0x01 // 1 if data is ready to be read on the data register
#define LINE_STAT_OE 0x02 // 1 if data sent has been lost
#define LINE_STAT_PE 0x04 // 1 if there was a parity error
#define LINE_STAT_FE 0x08 // 1 if a stop bit was missing
#define LINE_STAT_BI 0x10 // 1 if there is a break in data input
#define LINE_STAT_THRE 0x20 // 1 if data can be sent
#define LINE_STAT_TEMT 0x40 // 1 if transmitter not doing anything
#define LINE_STAT_ERR 0x80 // 1 if error with a word in the input buffer

#define STOP_BITS_1 0
#define STOP_BITS_2 1

#define PARITY_NONE 0x0
#define PARITY_ODD  0x1
#define PARITY_EVEN 0x3
#define PARITY_MARK 0x5
#define PARITY_SPACE 0x7

// Creates an 8 bit value that can be used to send to the LINE_CTRL register, to
// set serial parameters.
// char_len may be 5, 6, 7, or 8, and sets the amount of bits in each word sent.
// stop may be one of STOP_BITS_x, and will set to use x stop bits after each transmission.
// parity may be one of PARITY_x, and will set the parity scheme to use.
#define SET_LINE_CTRL(char_len, stop, parity) ((char_len - 5) & 0x3) | (stop << 2) | (parity << 3)

#define COM_PORT_COUNT 8
#define COM_PORT_COM1 0x3f8
#define COM_PORT_COM2 0x2f8

struct uart_com_port {
    uint16_t io_port_base;
    uint8_t status;
};

/*  Initialise a com_port struct. Does not set the com port up. Use the function 
    com_port_setup for that */
#define INIT_COM_PORT(base) ((struct uart_com_port){ \
    .io_port_base = base, \
    .status = COM_PORT_STATUS_BUSY \
})

extern struct uart_com_port *uart_port;

int x64_uart_init();

/*  Attempts to set up a com port with a given IO port base and baud rate
    divisor. Without any dividing, the baud rate of a com port will be 115200.
    This function also will attempt to use the loopback mode of the com port to
    check if it's working properly.

    If the com port is setup without a problem, its status will be set to
    COM_PORT_STATUS_READY. If there's an issue, it will instead be set to
    COM_PORT_STATUS_BROKEN.

    Returns 0 if there are no problems, otherwise a negative value. */
int com_port_setup(struct uart_com_port*, uint16_t divisor);

/*  Send a single character out of a given com port. */
void com_send(struct uart_com_port, char c);

/*  Send an entire null terminated string out of a given com port. */
void com_sends(struct uart_com_port, const char *s);

/*  Blocking, wait for and read a character from a given com port. */
char com_read(struct uart_com_port);

/*  Blocking, read chars from a given com port into a buffer, until a delimeter is read. */
void com_reads(struct uart_com_port, char *buff, char delim);

#endif /* __INCLUDE_X64_UART_H__ */