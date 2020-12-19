#include "rsa_genk.h"

#define SHA256_SIZE 64
#define PWD_SIZE 50

unsigned char passwd[PWD_SIZE];
mbedtls_hmac_drbg_context cont;
mbedtls_rsa_context rsa_cont;
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


static void UART_SEND(char *str) {
  HAL_UART_Transmit (huart, (unsigned char *)str,
      strlen(str), 300);
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
  const char *personalization = "dfajenFNXOmdfjacnI>ndfN";
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
void sendPriv(void) {
  blinkLed(LD2_GPIO_Port, LD2_Pin, 2, 300);
  unsigned char sha256[SHA256_SIZE] = "d03a7ba834457c81580617b90aac6f6505232f556952fcb7fabb3a740b1c2170";
  unsigned char signedSHA[500];
  int error;

  error = mbedtls_rsa_rsassa_pkcs1_v15_sign(&rsa_cont, mbedtls_hmac_drbg_random, &cont,
      MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256, SHA256_SIZE, sha256, signedSHA);

  if (error) {
    if (error == MBEDTLS_ERR_RSA_BAD_INPUT_DATA)
      __NOP();
    blinkLed(LD3_GPIO_Port, LD3_Pin, 3, 50);
  } else {
    HAL_UART_Transmit (huart, signedSHA, sizeof(signedSHA) , 300);
    UART_SEND("\n");
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
  UART_SEND("\n");
  blinkLed(LD1_GPIO_Port, LD1_Pin, 3, 50);
}

void message_handler(uint8_t *buff, uint32_t len){
  
}
