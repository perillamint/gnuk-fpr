/*
 * pin-fpr.c -- PIN input device support (Consumer Infra-Red)
 *
 * Copyright (C) 2010, 2011, 2013 Free Software Initiative of Japan
 * Author: perillamint <perillamint@gentoo.moe>
 *
 * This file is a part of Gnuk, a GnuPG USB Token implementation.
 *
 * Gnuk is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gnuk is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>
#include <string.h>
#include <chopstx.h>

#include "config.h"
#include "stm32f103.h"
#include "random.h"
#include "board.h"
#include "gnuk.h"

#include "gt511c1r.h"

#define USART_PORT USART1

#define ADMIN_PASSWD_MINLEN 8

uint8_t pin_input_buffer[MAX_PIN_CHARS];
uint8_t pin_input_len;

int pin_verify = 0;
uint8_t random_pin = 0;

char pin_str[ADMIN_PASSWD_MINLEN];

struct gt511c1r fpr;

int
pow(int a, int b)
{
  int i;
  int ret = 1;

  for(i = 0; i < b; i++)
    {
      ret *= a;
    }

  return ret;
}

void
usart_write (char *data, int len)
{
  int i;

  for (i = 0; i < len; i++)
  {
    while (!(USART_PORT -> SR & USART_SR_TXE));
    USART_PORT -> DR = data[i];
  }
}

void
usart_read (char *data, int len)
{
  int i;

  for (i = 0; i < len; i++)
  {
    while (!(USART_PORT -> SR & USART_SR_RXNE));
    data[i] = USART_PORT -> DR & 0xFF;
  }
}

void
do_usrt (void *tx, int txlen, void *rx, int rxlen)
{
  usart_write(tx, txlen);
  usart_read(rx, rxlen);
}

int
itoa_passwd (char *buf, int pass)
{
  int i;
  int cnt = 0;

  for(i = 0; i < ADMIN_PASSWD_MINLEN; i++)
    {
      int pwr = pow(10, ADMIN_PASSWD_MINLEN - i - 1);
      int digit = pass / pwr;
      pass -= digit * pwr;

      if(digit != 0)
        {
          buf[cnt++] = digit + '0';
        }
    }

  while(cnt < ADMIN_PASSWD_MINLEN)
    {
      buf[cnt++] = '0';
    }

  return cnt;
}

int
usart_readline (char *data, int len)
{
  int i;

  for (i = 0; i < len; i++)
  {
    while (!(USART_PORT -> SR & USART_SR_RXNE));
    data[i] = USART_PORT -> DR & 0xFF;

    if(data[i] == 0x0D || data[i] == 0x0A) break;

    while (!(USART_PORT -> SR & USART_SR_TXE));
    USART_PORT -> DR = '*';
  }

  return i;
}

void
fpr_firstrun ()
{
  gt511c1r_delete_all_fingerprint(&fpr);
}

int
pinpad_getline (int msg_code, uint32_t timeout)
{
  //TODO: Timeout stuff
  //TODO: Return length
  //TODO: Talk to FPR and identify finger!

  //DEBUG: SEND D over USART
  char codechar = msg_code + '0';
  int id;

  if (msg_code == PIN_INPUT_CURRENT)
    {
      //Verify FPR.
      id = gt511c1r_identify_fingerprint(&fpr);

      //Return ID. (Handle exceptional case: -1)

      if(id == -1)
        {
          pin_verify = 0;
          id = 255;
        }
      else
        {
          pin_verify = 1;
        }

      pin_input_len = itoa_passwd(pin_input_buffer, id);
      return pin_input_len;
    }
  else if (msg_code == PIN_INPUT_NEW)
    {
      /************
       * How to handle new PIN request:
       * 1. Verify and store it to global variable.
       * 2. If verify stored valid PIN, reset it
       * 3. If verify stored not valid PIN, return with dummy WITHOUT reset
       *
       * This will resist tampered FPR with 95% probability.
       ************/

      if(pin_verify == 1)
        {
          //Clear FPR.
          gt511c1r_delete_all_fingerprint(&fpr);

          //Generate random PIN (0-19).
          uint32_t random;
          random_get_salt (&random);
          random_pin = random % 20;

          //TODO: Register fingerprint.
          int fpr_ret = 1;

          while(fpr_ret != 0)
            {
              fpr_ret = gt511c1r_enroll_fingerprint(&fpr, random_pin);
            }
        }
      else
        {
          // Verification failed.
          // Return 255.
          random_pin = 255;
        }

      pin_input_len = itoa_passwd(pin_input_buffer, random_pin);
      return pin_input_len;
    }
  else if (msg_code == PIN_INPUT_CONFIRM)
    {
      //Verify FPR.
      id = gt511c1r_identify_fingerprint(&fpr);

      //Return ID. (Handle exceptional case: -1)

      if(id == -1)
        {
          pin_verify = 0;
          id = 255;
        }
      else
        {
          pin_verify = 1;
        }

      pin_input_len = itoa_passwd(pin_input_buffer, id);
      return pin_input_len;
    }

  //TODO: Error handling
  return 0;
}

void
fpr_init (void)
{
  //TODO: Hello, world! from UART!
  //TODO: Handshake with FPR (and open it!).

  //Supply clock to USART.
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  RCC->APB2RSTR = RCC_APB2RSTR_USART1RST;
  RCC->APB2RSTR = 0;

  //Set baud -- 9600bps
  USART_PORT -> BRR = STM32_HSECLK * STM32_PLLMUL_VALUE / 9600;

  // Enable the UART, TX, and RX circuit
  USART_PORT -> CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

  //INIT DEBUG MSG
  //usart_write("FPRINIT", 7);

  gt511c1r_init(&fpr, do_usrt);
  gt511c1r_open(&fpr);

  //gt511c1r_set_led(&fpr, 0x00000001);
}
