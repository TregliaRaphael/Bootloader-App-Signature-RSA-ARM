#ifndef MY_BOOTLOADER_H
#define MY_BOOTLOADER_H

#define APP_ADDR (0x8040000)
#define APP_ADDR_P (0x8040004)



#include "main.h"
#include "usbd_cdc_if.h"
#include <string.h>

typedef enum
{
    JumpMode,
	FlashMode
} BootloaderMode;

typedef enum
{
    App1,
	App2
} AppSelection;

typedef enum
{
    Unerased,
	Erased,
	Unlocked,
	Locked
} FlashStatus;

uint32_t Flashed_offset;
FlashStatus flashStatus;
extern USBD_HandleTypeDef hUsbDeviceFS;     //it is defined in the usb_device.c

uint32_t readWord(uint32_t address);
void flashWord(uint32_t dataToFlash);
void bootloaderInit(void);
void goToApp(void);
void checkAndJump(void);
void eraseMemory(void);


#endif // MY_BOOTLOADER_H
