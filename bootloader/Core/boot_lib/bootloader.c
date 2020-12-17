#include "bootloader.h"


void bootloaderInit()
{
	Flashed_offset = 0;
	flashStatus = Unerased;
	BootloaderMode bootloaderMode;

  
   /* if (HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin) == GPIO_PIN_SET) //Enable flash mod
    {
        //do nothing and wait flash
    }
    else
        checkAndJump();
*/
}



void deinitEverything(void)
{
    //-- reset peripherals to guarantee flawless start of user application

    //Mandatory for usb
    HAL_GPIO_DeInit(LD1_GPIO_Port, LD1_Pin);
    HAL_GPIO_DeInit(LD2_GPIO_Port, LD2_Pin);
    HAL_GPIO_DeInit(LD3_GPIO_Port, LD3_Pin);
    HAL_GPIO_DeInit(USER_Btn_GPIO_Port, USER_Btn_Pin);
      __HAL_RCC_GPIOC_CLK_DISABLE();
      __HAL_RCC_GPIOD_CLK_DISABLE();
      __HAL_RCC_GPIOB_CLK_DISABLE();
      __HAL_RCC_GPIOA_CLK_DISABLE();
      __HAL_RCC_GPIOH_CLK_DISABLE();
      __HAL_RCC_GPIOG_CLK_DISABLE();
    HAL_RCC_DeInit();
    HAL_DeInit(); 
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
}

void goToApp(void)
{
  uint32_t stack = *((uint32_t *) 0x8040000);
  uint32_t prog = *((uint32_t *) 0x8040004);
  void (*jump)(void);
  jump = prog;
  SCB->VTOR = (uint32_t)APP_ADDR;
  __set_MSP (stack);
  jump();

}

void checkAndJump(void)
{
    //check bin signature
		//little Check if the application is there
		uint8_t emptyCellCount = 0;
		for(uint8_t i=0; i<10; i++)
		{
			if(readWord(APP_ADDR + (i*4)) == -1)
				emptyCellCount++;
		}
		if(emptyCellCount != 10)
			goToApp();
		else
			errorBlink();

    //else do nothing
}

uint32_t readWord(uint32_t address)
{
	uint32_t read_data;
	read_data = *(uint32_t*)(address);
	return read_data;
}

void flashWord(uint32_t dataToFlash)
{
	if(flashStatus == Unlocked)
	{
	  volatile HAL_StatusTypeDef status;
	  uint8_t flash_attempt = 0;
	  uint32_t address;
	  do
	  {
		  address = APP_ADDR + Flashed_offset;
		  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, dataToFlash);
		  flash_attempt++;
	  }while(status != HAL_OK && flash_attempt < 10 && dataToFlash == readWord(address));
	  if(status != HAL_OK)
	  {
		  //CDC_Transmit_FS((uint8_t*)&"Flashing Error!\n", strlen("Flashing Error!\n"));
	  }else
	  {//Word Flash Successful
		  Flashed_offset += 4;
		  //CDC_Transmit_FS((uint8_t*)&"Flash: OK\n", strlen("Flash: OK\n"));
	  }
	}else
	{
	  CDC_Transmit_FS((uint8_t*)&"Error: Memory not unlocked nor erased!\n",
			  strlen("Error: Memory not unlocked nor erased!\n"));
	}
}

void eraseMemory()
{

    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;

//#ifdef USE_F767ZI
    EraseInitStruct.Sector = APP_ADDR;
    EraseInitStruct.NbSectors = 1; //to check
/*#else
    EraseInitStruct.PageAddress = APP_ADDR;
    EraseInitStruct.NbPages = 1; //to check
#endif
*/
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    uint32_t PageError;
    volatile HAL_StatusTypeDef status_erase, status_write;
    status_erase = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    if (status_erase != HAL_OK) //blink red bad
    {
        uint32_t error = (FLASH->SR & FLASH_FLAG_ALL_ERRORS);
        //printf("Write failed: %x \r\n", error);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
    }
    else //blink Green good
    {
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
    }


    HAL_FLASH_Lock();

    flashStatus = Erased;
	Flashed_offset = 0;
}
