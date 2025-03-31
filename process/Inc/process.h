#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define BUF_SIZE  1024

#define CMD_LEN		(2+2+4)
#define STA_LEN		(2+4)

extern uint16_t status;

extern uint8_t rx_buffer[];
extern uint8_t tx_buffer[];
extern uint32_t payload_len;

void process_init(void);
void process_cmd(void);
void process_deinit(void);

#endif /* process.h */
