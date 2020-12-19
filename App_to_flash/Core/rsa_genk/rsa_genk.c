#include "rsa_genk.h"

#define SHA256_SIZE 64

unsigned char val = 0;
mbedtls_hmac_drbg_context cont;
mbedtls_rsa_context rsa_cont;
bool keyGenerated = false;

UART_HandleTypeDef *huart;


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

static void UART_RECEIVE(unsigned char* buffer, size_t buffer_len, char* instruction) {
  HAL_StatusTypeDef state;
  while ((state = HAL_UART_Receive (huart, buffer, buffer_len, 300)) != HAL_OK) {
    if (instruction) {
      UART_SEND(instruction);
    } else {
      if (state == HAL_BUSY)
        UART_SEND("errorBUSY\n");
      else if (state == HAL_TIMEOUT)
        UART_SEND("errorTIMEOUT\n");
      else if (state == HAL_ERROR)
        UART_SEND("errorERROR\n");
    }
  }
  UART_SEND("FINISHED\n");
}


void mbedRsaInit(UART_HandleTypeDef *uart){
  mbedtls_hmac_drbg_init(&cont);
  mbedtls_rsa_init(&rsa_cont, MBEDTLS_RSA_PKCS_V15, 0);
  huart = uart;
}


void genKey(void) {
  const char *personalization = "dfajenFNXOmdfjacnI>ndfN";
  const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  int error = mbedtls_hmac_drbg_seed_buf(&cont,
                                    md_info,
                                    (unsigned char *) personalization,
                                    strlen(personalization));

  /*if (error != 0) {
    if (error == MBEDTLS_ERR_MD_BAD_INPUT_DATA) {
      UART_SEND("MD bad input data\n");
    }
    else if (error == MBEDTLS_ERR_MD_ALLOC_FAILED) {

      UART_SEND("MD Alloc failed\n");
    }
    else {
      UART_SEND("Err idk\n");
    }

  }*/
  error = mbedtls_rsa_gen_key(&rsa_cont, mbedtls_hmac_drbg_random, &cont, 1024, 65537);
  if (!error) {
    keyGenerated = true;
  } 
  /*else {
    UART_SEND("rsa generation failed\n");
  }*/
}


void sendPriv(void) {
  unsigned char sha256[SHA256_SIZE] = "d03a7ba834457c81580617b90aac6f6505232f556952fcb7fabb3a740b1c2170";
  unsigned char signedSHA[500];
  int error;

  //UART_RECEIVE(sha256, SHA256_SIZE, NULL);

  error = mbedtls_rsa_rsassa_pkcs1_v15_sign(&rsa_cont, mbedtls_hmac_drbg_random, &cont,
      MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256, SHA256_SIZE, sha256, signedSHA);

  if (error) {

    if (error == MBEDTLS_ERR_RSA_BAD_INPUT_DATA)
      UART_SEND("error: signature failed input\n");
    else
      UART_SEND("error: signature failed\n");

  } else {
    HAL_UART_Transmit (huart, signedSHA, sizeof(signedSHA) , 300);
    UART_SEND("\n");
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
  volatile int len = strlen(buffer);
  UART_SEND(buffer);
}

void sendPub(void){
  send_mpi_buffer_UART(&rsa_cont.N);
  send_mpi_buffer_UART(&rsa_cont.E);
  UART_SEND("\n");
}
