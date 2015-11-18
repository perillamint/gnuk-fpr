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
#include "gnuk.h"

uint8_t pin_input_buffer[MAX_PIN_CHARS];

int
pinpad_getline (int msg_code, uint32_t timeout)
{
  //TODO: Timeout stuff
  //TODO: Return length
  //TODO: Talk to FPR and identify finger!
}

void
fpr_init (void)
{
  //TODO: Hello, world! from UART!
  //TODO: Handshake with FPR (and open it!).
}
