#include "rsa_genk.h"

#define SHA256_SIZE 64
#define PWD_SIZE 10
#define BUFF_SIZE 100
#define BSHALEN 300

unsigned char passwd[PWD_SIZE];
uint8_t buff[BUFF_SIZE];
unsigned char shasha256[SHA256_SIZE];
unsigned char bsha[BSHALEN];

mbedtls_hmac_drbg_context cont;
mbedtls_rsa_context rsa_cont;
char *personalization = "azefjsigphazejhfiupazea";
bool keyGenerated = false;

UART_HandleTypeDef *huart;

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


static char convertToHexa(uint32_t u) {
  if (u <= 9)
    return u + '0';
  else if (u <= 15)
    return u - 10 + 'A';
  else {
    return '0';
  }
}

static void meltPwdAndKey(){
  
}

static void UART_SEND(char *str) {
  HAL_UART_Transmit (huart, (unsigned char *)str,
      strlen(str), 300);
}

static void UART_RECEIVE(uint8_t* buffer, size_t buffer_len, uint8_t *mess) {
  while (1)
  {
    if (HAL_UART_Receive(huart, buffer, buffer_len, 100) == HAL_OK)
    {
      if (mess != NULL)
        UART_SEND((char *)mess);
      else
        UART_SEND("OK\n");
      blinkLed(LD2_GPIO_Port, LD2_Pin, 1, 50);
      return ;
    }

  }
}

void mbedRsaInit(UART_HandleTypeDef *uart){
  mbedtls_hmac_drbg_init(&cont);
  mbedtls_rsa_init(&rsa_cont, MBEDTLS_RSA_PKCS_V15, 0);
  huart = uart;
  pStatus = nosha;
}


//one blue blink for this fct
void genKey(void) {
  blinkLed(LD2_GPIO_Port, LD2_Pin, 1, 300);
  const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  int error = mbedtls_hmac_drbg_seed_buf(&cont,
                                    md_info,
                                    (unsigned char *) personalization,
                                    strlen(personalization));

  if (error)
  {
    if (error == MBEDTLS_ERR_MD_BAD_INPUT_DATA)
      __NOP();
    else if (error == MBEDTLS_ERR_MD_ALLOC_FAILED)
      __NOP();
    blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 50);
    return;
  }
  if (!mbedtls_rsa_gen_key(&rsa_cont, mbedtls_hmac_drbg_random, &cont, 1024, 65537)){
    keyGenerated = true;
    blinkLed(LD1_GPIO_Port, LD1_Pin, 3, 50);
  }
  else
    blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 50);
}

//two blue blink for this fct
void sendSign(void) {
  blinkLed(LD2_GPIO_Port, LD2_Pin, 2, 300);
  unsigned char signedSHA[500];
  int error;
  
  error = mbedtls_rsa_rsassa_pkcs1_v15_sign(&rsa_cont, mbedtls_hmac_drbg_random, &cont,
      MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256, BSHALEN, bsha, signedSHA);
  
  if (error) {
    if (error == MBEDTLS_ERR_RSA_BAD_INPUT_DATA)
      __NOP();
    UART_SEND("SIGNED CREATE FAIL\n");
    UART_SEND(" end\n");
    blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 50);
  } else {
    unsigned char b64tosend[500];
    int lenWritten;
    if(!mbedtls_base64_encode(b64tosend, 500, &lenWritten, signedSHA, 32))
    	UART_SEND(b64tosend);
    else
    	UART_SEND("B64 ENCODE SHA ERR");
    UART_SEND(" end\n");
    blinkLed(LD1_GPIO_Port, LD1_Pin, 3, 50);
  }
}

static void send_mpi_buffer_UART(mbedtls_mpi *mpi) {
  size_t n = mpi->n * 8 + 2;
  char buffer[n];
  size_t buffer_index = 0;


  for (size_t index = 0; index < mpi->n; index++) {
    uint32_t val = mpi->p[index];

    for (unsigned shift = 0; shift < 32; shift+=4) {
      buffer[buffer_index++] = convertToHexa( (val << shift) >> 28);
    }

  }

  for (; buffer_index < n; buffer_index++)
    buffer[buffer_index++] = '\0';
  UART_SEND(buffer);
}

//three blue blink for this fct
void sendPub(void){
  blinkLed(LD2_GPIO_Port, LD2_Pin, 3, 300);
  send_mpi_buffer_UART(&rsa_cont.N);
  send_mpi_buffer_UART(&rsa_cont.E);
  UART_SEND(" end\n");
  blinkLed(LD1_GPIO_Port, LD1_Pin, 3, 50);
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
         else   return 0;
}

int checkStatus(PStatus stat, char *fail, int nbr_send){
  if (pStatus != stat){
    for (int i = 0; i < nbr_send; i++)
      UART_SEND(fail);
    blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 50);
    return 1;
  }
  return 0;
}

void message_handler(void){
  UART_RECEIVE(&buff, 1, NULL);
  switch(buff[0])
  {
    case '1': //SHA INIT
      
      UART_RECEIVE(&shasha256, SHA256_SIZE, (uint8_t *)"Sha stored\n");
      if (checkStatus(nosha, "Sha already init\n",1))
        return;
      int wlen;
      if(mbedtls_base64_decode(bsha, BSHALEN, &wlen, shasha256, 64)){
        UART_SEND("Sha to byte fail => sha size need to be 64 characters\n");
        blinkLed(LD3_GPIO_Port, LD3_Pin, 1, 50);
      }
      else{
        UART_SEND("Sha to byte Success\n");
        blinkLed(LD1_GPIO_Port, LD1_Pin, 1, 50);
        pStatus = nopwd;
      }
      break;

    case '2': //PWD INIT
      if (checkStatus(nopwd, "KOO\n",1))
        return;
      UART_SEND("OOK\n");
      UART_RECEIVE(&passwd, PWD_SIZE, (uint8_t *)"Password init successfull\n");
      blinkLed(LD1_GPIO_Port, LD1_Pin, 2, 50);
      meltPwdAndKey();
      genKey();
      UART_SEND("RSA and Melting KeyGen successfull\n");
      pStatus = ready;
      break;

    case '3': //ASK PUBKEY
      if (checkStatus(ready, "Can't ask pubkey\n",3))
        return;

      UART_RECEIVE(&buff, PWD_SIZE, NULL);
      if (string_compare((char *)buff, (char *)passwd, PWD_SIZE))
      {
          UART_SEND("PWD OK\n");
          sendPub();
          blinkLed(LD1_GPIO_Port, LD1_Pin, 3, 50);
      }
      else
      {
          UART_SEND("PWD KO\n");
          UART_SEND("PWD KO\n");
          blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 50);
      }
      break;

    case '4': //ASK PRIVKEY
      if (checkStatus(ready, "Can't ask signedKey end\n",3))
        return;

      UART_RECEIVE(&buff, PWD_SIZE, NULL);
      if (string_compare((char *)buff, (char *)passwd, PWD_SIZE))
      {
          UART_SEND("PWD OK\n");
          sendSign();
          blinkLed(LD1_GPIO_Port, LD1_Pin, 3, 50);
      }
      else
      {
          UART_SEND("PWD KO\n");
          UART_SEND("PWD KO end\n");
          blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 50);
      }
      break;

    default:
      break;
  }
}
