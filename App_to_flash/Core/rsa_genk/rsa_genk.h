#ifndef MY_RSA_GENK_H
#define MY_RSA_GENK_H

#include "main.h"
#include "mbedtls.h"

#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "platform.h"
#include "md_internal.h"
#include "hmac_drbg.h"
#include "rsa.h"

//PROCESS script
//script send sha (if no sha leave script)
//APP store sha
//script ask password then send it to app to be loaded
//APP store pwd
//script ask pubkey
	//script send password
	//if pwdgood and app ready, APP send pubkey
	//else none
//script ask privkey
	//script send password
	//if pwdgood and app ready, APP send privkey
	//else none


typedef enum
{
  dead,		// Si nosha given
  nosha,	// Start state
  nopwd,  	// nopwd = shaok
  ready,	// shaok & pwd ok
  
  
} PStatus;

PStatus pStatus;

void blinkLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, int nbrblink, int delayms);

void mbedRsaInit(UART_HandleTypeDef *uart);
void genKey(void);
void sendPriv(void);
void sendPub(void);
void message_handler(uint8_t *buff, uint32_t len);

#endif //MY_RSA_GENK_H
