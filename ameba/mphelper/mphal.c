/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Chester Tseng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*****************************************************************************
 *                              Header includes
 * ***************************************************************************/
#include <stdio.h>

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mpstate.h"
#include "extmod/misc.h"
#include "lib/utils/pyexec.h"
#include "py/obj.h"
#include "py/ringbuf.h"

#include <stdio.h>
#include "osdep_api.h"

int mp_hal_stdin_rx_chr(void) {
    return  mp_term_rx_chr();

}

void mp_hal_stdout_tx_chr(char c) {
    mp_term_tx_strn(&c, 1);
}

void mp_hal_stdout_tx_str(const char *str) {
    while (*str) {
        mp_hal_stdout_tx_chr(*str++);
    }
}

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    while (len--) {
        mp_hal_stdout_tx_chr(*str++);
    }
}

void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    while (len--) {
        if (*str == '\n') {
            mp_hal_stdout_tx_chr('\r');
        }
        mp_hal_stdout_tx_chr(*str++);
    }
}

void mp_hal_delay_ms(uint32_t ms) {
    mdelay(ms);
}

void mp_hal_delay_us(uint32_t us) {
    udelay(us);
}

uint32_t mp_hal_ticks_ms(void) {
    return xTaskGetTickCount();
}
