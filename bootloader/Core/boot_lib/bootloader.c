#include "bootloader.h"


void bootloaderInit()
{
	Flashed_offset = 0;
	flashStatus = Unerased;
	//BootloaderMode bootloaderMode;

  
   /* if (HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin) == GPIO_PIN_SET) //Enable flash mod
    {
        //do nothing and wait flash
    }
    else
        checkAndJump();
*/
}



static void deinitEverything(void)
{
    //-- reset peripherals to guarantee flawless start of user application

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
  deinitEverything();
  uint32_t stack = *((uint32_t *) APP_ADDR);
  uint32_t prog = *((uint32_t *) APP_ADDR_P);
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
            __NOP();

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
          blinkLed(LD3_GPIO_Port, LD3_Pin, 4, 50);
      else
	  {//Word Flash Successful
		  Flashed_offset += 4;
          //blinkLed(LD2_GPIO_Port, LD2_Pin, 2, 20);
	  }
	}
    else
        blinkLed(LD3_GPIO_Port, LD3_Pin, 2, 50);
}

void eraseMemory()
{
    flashStatus = Erasing;

    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

#ifdef USE_F767ZI
    EraseInitStruct.Sector = FLASH_SECTOR_5;
    EraseInitStruct.NbSectors = 1; //to check
#else
    EraseInitStruct.PageAddress = APP_ADDR;
    EraseInitStruct.NbPages = 1; //to check
#endif

    blinkLed(LD2_GPIO_Port, LD2_Pin, 2, 200);

    uint32_t PageError;
    volatile HAL_StatusTypeDef status_erase;
    status_erase = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    if (status_erase != HAL_OK) //blink red bad
    {
        uint32_t error = (FLASH->SR & FLASH_FLAG_ALL_ERRORS);
        //printf("Write failed: %x \r\n", error);
        blinkLed(LD3_GPIO_Port, LD3_Pin, 2, 200);
    }
    else //blink Green good
        blinkLed(LD1_GPIO_Port, LD1_Pin, 2, 200);

    HAL_FLASH_Lock();

    flashStatus = Erased;
	Flashed_offset = 0;
}


static uint8_t string_compare(char array1[], char array2[], uint16_t length)
{
	 uint8_t comVAR=0, i;
	 for(i=0;i<length;i++)
	   	{
	   		  if(array1[i]==array2[i])
	   	  		  comVAR++;
	   	  	  else comVAR=0;
	   	}
	 if (comVAR==length)
		 	return 1;
	 else 	return 0;
}

static void lockFlash(void)
{
	/* Lock the Flash to enable the flash control register access *************/
	HAL_FLASH_Lock();

	flashStatus = Locked;
}

static void unlockFlash(void)
{
    HAL_FLASH_Unlock();
	flashStatus = Unlocked;

}

void messageHandler(uint8_t* Buf)
{
	if(string_compare((char*)Buf, ERASE_FLASH_MEMORY, strlen(ERASE_FLASH_MEMORY))
			&& flashStatus != Unlocked)
	{
		eraseMemory();

	}
    else if(string_compare((char*)Buf, FLASHING_START, strlen(FLASHING_START)))
	{
        if (flashStatus != Erased)
            eraseMemory();
        unlockFlash();
        blinkLed(LD2_GPIO_Port, LD2_Pin, 3, 200);
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

	}
    else if(string_compare((char*)Buf, FLASHING_FINISH, strlen(FLASHING_FINISH))
			  && flashStatus == Unlocked)
	{
		lockFlash();
        
        blinkLed(LD1_GPIO_Port, LD1_Pin, 3, 200);
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

        checkAndJump();

	}
    else if(string_compare((char*)Buf, FLASHING_ABORT, strlen(FLASHING_ABORT))
			  && flashStatus == Unlocked)
	{
		lockFlash();
		eraseMemory();
        blinkLed(LD3_GPIO_Port, LD3_Pin, 2, 1000);
	}
    else
	{
        blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 500);
	}
}



void blinkLed(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, int nbrblink, int delayms)
{
  for (int i = 0; i < nbrblink; i++)
  {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET); 
    HAL_Delay(delayms/2); 
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET); 
    HAL_Delay(delayms/2); 
  }
}
