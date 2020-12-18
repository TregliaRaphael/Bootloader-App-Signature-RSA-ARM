#ifndef MY_BOOTLOADER_H
#define MY_BOOTLOADER_H

#define APP_ADDR (0x8040000)
#define APP_ADDR_P (0x8040004)

#define ERASE_FLASH_MEMORY "#$ERASE_MEM"
#define FLASHING_START "#$FLASH_START"
#define FLASHING_FINISH "#$FLASH_FINISH"
#define FLASHING_ABORT "#$FLASH_ABORT"


#include "main.h"
#include "usbd_cdc_if.h"
#include "crypto_hash_sha256.h"
#include <string.h>
#include <stdbool.h>

typedef enum
{
    JumpMode,
	FlashMode
} BootloaderMode;

typedef enum
{
    Unerased,
    Erasing,
	Erased,
	Unlocked,
    Flashed,
	Locked
} FlashStatus;

uint32_t Flashed_offset;
FlashStatus flashStatus;
extern USBD_HandleTypeDef hUsbDeviceFS;     //it is defined in the usb_device.c

void bootloaderInit(void);
void goToApp(void);
void checkAndJump(void);

uint32_t readWord(uint32_t address);
void flashWord(uint32_t dataToFlash);

void eraseMemory(void);
void messageHandler(uint8_t* Buf);

void blinkLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, int nbrblink, int delayms);

#endif // MY_BOOTLOADER_H
