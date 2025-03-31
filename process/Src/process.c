#include "process.h"
#include "mbedtls/aes.h"
#include "mbedtls/des.h"

uint16_t status;

uint8_t rx_buffer[BUF_SIZE];
uint8_t tx_buffer[BUF_SIZE];
uint32_t payload_len;

unsigned char deskey[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
unsigned char input[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
unsigned char output[8];
unsigned char backward[8];

mbedtls_aes_context aes_ctx;
mbedtls_des_context des_ctx;
int key_length = 128; // 密钥长度，AES支持128�?????????????????????????192�?????????????????????????256位密�?????????????????????????

void process_init(void) {
	mbedtls_aes_init(&aes_ctx);
	mbedtls_des_init(&des_ctx);
}
void process_cmd(void) {
	if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x00) { // Set AES encrypt Key
		mbedtls_aes_setkey_enc(&aes_ctx, rx_buffer + CMD_LEN, key_length);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x01) { // Set AES decrypt Key
		mbedtls_aes_setkey_dec(&aes_ctx, rx_buffer + CMD_LEN, key_length);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x02) { // AES Encryption
		mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT,
				rx_buffer + CMD_LEN, tx_buffer + STA_LEN);
		status = 0;
		payload_len = 16;
	} else if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x03) { // AES Decryption
		mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT,
				rx_buffer + CMD_LEN, tx_buffer + STA_LEN);
		status = 0;
		payload_len = 16;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x00) { // Set DES encryption key
		mbedtls_des_setkey_enc(&des_ctx, rx_buffer + CMD_LEN);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x01) { // Set DES decryption key
		mbedtls_des_setkey_dec(&des_ctx, rx_buffer + CMD_LEN);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x02) { // DES Encryption
		mbedtls_des_crypt_ecb(&des_ctx, rx_buffer + CMD_LEN,
				tx_buffer + STA_LEN);
		status = 0;
		payload_len = 8;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x03) {
		mbedtls_des_crypt_ecb(&des_ctx, rx_buffer + CMD_LEN,
				tx_buffer + STA_LEN);
		status = 0;
		payload_len = 8;
	} else {
		status = 0x8000;
		payload_len = 0;
	}
	return;
}

void process_deinit(void) {
	mbedtls_aes_free(&aes_ctx);
	mbedtls_des_free(&des_ctx);
}
