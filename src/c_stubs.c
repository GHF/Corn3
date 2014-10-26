/*
 * Copyright (C) 2013-2014 Xo Wang
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "ch.h"
#include "hal.h"

#include "config.h"

/* These should be ChibiOS sequential streams. */
#define STDIN_SERIAL    DEBUG_SERIAL
#define STDOUT_SERIAL   DEBUG_SERIAL
#define STDERR_SERIAL   DEBUG_SERIAL

/**
 * @brief Read from file.
 *
 * @note Only stdin is supported.
 *
 * @param r Reentrancy context.
 * @param file File to read from.
 * @param ptr Pointer to output buffer.
 * @param len Number of bytes to read.
 * @return Number of bytes read.
 */
int _read_r(struct _reent *r, int file, char *ptr, int len) {
  switch (file) {
  case STDIN_FILENO:
    return sdRead(&STDIN_SERIAL, (uint8_t *)ptr, len);
  default:
    __errno_r(r) = EBADF;
    return -1;
  }
}

/**
 * @brief Write to file.
 *
 * @note Only stdout and stderr are supported.
 *
 * @param r Reentrancy context.
 * @param file File to write to.
 * @param ptr Pointer to input buffer.
 * @param len Number of bytes to write.
 * @return Number of bytes successfully written.
 */
int _write_r(struct _reent *r, int file, char *ptr, int len) {
  switch (file) {
  case STDOUT_FILENO:
    return sdWrite(&STDOUT_SERIAL, (uint8_t *)ptr, len);
  case STDERR_FILENO:
    return sdWrite(&STDERR_SERIAL, (uint8_t *)ptr, len);
  default:
    __errno_r(r) = EBADF;
    return -1;
  }
}

/**
 * @brief Closes a file.
 *
 * @param r Reentrancy context.
 * @param file File to close.
 * @return 0 for success, -1 for error (errno is set).
 */
int _close_r(struct _reent *r, int file) {
  (void) file;
  __errno_r(r) = EBADF;
  return -1;
}

/**
 * @brief Get information about file.
 *
 * @brief Only stdin, stdout, and stderr are supported.
 *
 * @param r Reentrancy context.
 * @param file File to retrieve information about.
 * @param st Output structure of file information.
 * @return 0 for success, -1 for error (errno is set).
 */
int _fstat_r(struct _reent *r, int file, struct stat *st) {
  (void) file;
  switch (file) {
  case STDIN_FILENO:
  case STDOUT_FILENO:
  case STDERR_FILENO:
    st->st_mode = S_IFCHR;
    return 0;
  default:
    __errno_r(r) = EBADF;
    return -1;
  }
}

/**
 * @brief Seek within a file.
 *
 * @note System has no seekable files, so this returns an error.
 *
 * @param r Reentrancy context.
 * @param file File to seek in.
 * @param ptr No effect.
 * @param dir No effect.
 * @return -1 for error.
 */
int _lseek_r(struct _reent *r, int file, int ptr, int dir) {
  (void) file;
  (void) ptr;
  (void) dir;
  __errno_r(r) = EBADF;
  return -1;
}

/**
 * @brief Report whether connected to terminal device.
 *
 * @note System is always connected to terminal device.
 *
 * @param r Reentrancy context.
 * @param file File to report on.
 * @return 1 if connected to terminal device, 0 if not.
 */
int _isatty_r(struct _reent *r, int file) {
  (void) r;
  (void) file;
  return 1;
}

static inline bool IsIsrActive(void) {
  /* ISR_NUMBER is bits 8:0 (0 for thread mode). */
  return (__get_IPSR() & 0x1ff) != 0;
}

/**
 * @brief Change program break (end of heap segment).
 *
 * @note In theory sbrk can reduce segment size, but that isn't supported.
 *
 * @param r Reentrancy context.
 * @param increment Amount to change heap segment size by.
 * @return Start of newly allocated memory (guaranteed to be followed by at
 *         least @p increment bytes of heap).
 */
void *_sbrk_r(struct _reent *r, intptr_t increment) {
  (void) r;
  void *p;
  /* Do nothing if the system wants to decrease heap size. */
  if (increment < 0)
    increment = 0;
  /* Use the ChibiOS-managed heap segment. */
  if (IsIsrActive()) {
    /* Call originated from an interrupt context. This check allows library
       functions that use malloc to be called from ISRs. */
    chSysLockFromIsr();
    p = chCoreAllocI(increment);
    chSysUnlockFromIsr();
  } else {
    chSysLock();
    p = chCoreAllocI(increment);
    chSysUnlock();
  }
  if (p == NULL) {
    __errno_r(r) = ENOMEM;
    return (void *)-1;
  }
  return p;
}
