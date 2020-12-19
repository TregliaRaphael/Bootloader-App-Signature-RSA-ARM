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

void blinkLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, int nbrblink, int delayms);

void mbedRsaInit(UART_HandleTypeDef *uart);
void genKey(void);
void sendPriv(void);
void sendPub(void);

#endif //MY_RSA_GENK_H
