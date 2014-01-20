/*
 * Copyright (C) 2013 Xo Wang
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

#include "ch.h"
#include "hal.h"

#include "config.h"

#include <stdint.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/unistd.h>

/* These should be ChibiOS sequential streams. */
#define STDIN_SERIAL    DEBUG_SERIAL
#define STDOUT_SERIAL   DEBUG_SERIAL
#define STDERR_SERIAL   DEBUG_SERIAL

/* The #undef strips the reentrant wrapper around errno. See newlib docs. */
#undef errno
extern int errno;

/**
 * @brief Read from file.
 *
 * @note Only stdin is supported.
 *
 * @param file File to read from.
 * @param ptr Pointer to output buffer.
 * @param len Number of bytes to read.
 * @return Number of bytes read.
 */
int _read(int file, char *ptr, int len) {
  switch (file) {
  case STDIN_FILENO:
    return chSequentialStreamRead(&STDIN_SERIAL, (uint8_t * )ptr, len);
  default:
    errno = EBADF;
    return -1;
  }
}

/**
 * @brief Write to file.
 *
 * @param file File to write to.
 * @param ptr Pointer to input buffer.
 * @param len
 * @return
 */
int _write(int file, char *ptr, int len) {
  switch (file) {
  case STDOUT_FILENO:
    return chSequentialStreamWrite(&STDOUT_SERIAL, (uint8_t * )ptr, len);
  case STDERR_FILENO:
    return chSequentialStreamWrite(&STDERR_SERIAL, (uint8_t * )ptr, len);
  default:
    errno = EBADF;
    return -1;
  }
}

/**
 * @brief Closes a file.
 *
 * @param file File to close.
 * @return 0 for success, -1 for error (errno is set).
 */
int _close(int file) {
  (void) file;
  errno = EBADF;
  return -1;
}

/**
 * @brief Get information about file.
 *
 * @brief Only stdin, stdout, and stderr supported.
 *
 * @param file File to retrieve information about.
 * @param st Output structure of file information.
 * @return 0 for success, -1 for error (errno is set).
 */
int _fstat(int file, struct stat *st) {
  (void) file;
  switch (file) {
  case STDIN_FILENO:
  case STDOUT_FILENO:
  case STDERR_FILENO:
    st->st_mode = S_IFCHR;
    return 0;
  default:
    errno = EBADF;
    return -1;
  }
}

/**
 * @brief Seek within a file.
 *
 * @note System has no seekable files, so this returns an error.
 *
 * @param file File to seek in.
 * @param ptr No effect.
 * @param dir No effect.
 * @return -1 for error.
 */
int _lseek(int file, int ptr, int dir) {
  (void) file;
  (void) ptr;
  (void) dir;
  errno = EBADF;
  return -1;
}

/**
 * @brief Report whether connected to terminal device.
 *
 * @note System is always connected to terminal device.
 *
 * @param file File to report on.
 * @return 1 if connected to terminal device, 0 if not.
 */
int _isatty(int file) {
  (void) file;
  return 1;
}

/**
 * @brief Change program break (end of heap segment).
 *
 * @note In theory sbrk can reduce segment size, but that isn't supported.
 *
 * @param increment Amount to change heap segment size by.
 * @return Start of newly allocated memory (guaranteed to be followed by at
 *         least @p increment bytes of heap).
 */
void *_sbrk(intptr_t increment) {
  /* Do nothing if the system wants to decrease heap size. */
  if (increment < 0)
    increment = 0;
  /* Use the ChibiOS-managed heap segment. */
  return chCoreAlloc(increment);
}

/**
 * @brief Halts the system.
 *
 * @param status Status code.
 */
void _exit(int status) {
  (void) status;
  chSysHalt();
  while (1)
    ;
}

/**
 * @brief Retrieve process ID.
 *
 * @note System supports only one process, so always returns 1.
 *
 * @return pid.
 */
pid_t _getpid(void) {
  return 1;
}

/**
 * @brief Kills process by PID.
 *
 * @note System supports only one process, so does nothing.
 *
 * @param id pid.
 */
void _kill(pid_t id) {
  (void) id;
}
