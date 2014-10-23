/*
 * Copyright (C) 2014 Xo Wang
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

#include "base/log.h"

#include <stdarg.h>

#include "config.h"
#include "base/utility.h"

#if LOGGING_USE_CHPRINTF
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/* Default file descriptor to log to. */
#ifndef LOGGING_FILE
#define LOGGING_FILE ((BaseSequentialStream *)&DEBUG_SERIAL)
#endif
#define LOG_FPRINTF(file, ...)              chprintf(file, __VA_ARGS__)
#define LOG_VFPRINTF(file, format, va_list) chvprintf(file, format, va_list)
#else
#include <stdio.h>

/* Default file descriptor to log to. */
#ifndef LOGGING_FILE
#define LOGGING_FILE stderr
#endif
#define LOG_FPRINTF(file, ...)              fprintf(file, __VA_ARGS__)
#define LOG_VFPRINTF(file, format, va_list) vfprintf(file, format, va_list)
#endif  /* #if LOGGING_USE_CHPRINTF */

#ifndef STATIC_LOGGING_LEVEL
static LoggingLevel g_logging_level = LOGGING_DEFAULT_LEVEL;

LoggingLevel GetLoggingLevel(void) {
  return g_logging_level;
}

void SetLoggingLevel(LoggingLevel level) {
  if (level > LOGGING_NUM_LEVELS)
    g_logging_level = LOGGING_NUM_LEVELS;
  else
    g_logging_level = level;
}
#endif  /* STATIC_LOGGING_LEVEL */

void vLogAtLevel(LoggingLevel level,
                 const char *func,
                 const char *format,
                 va_list args) {
  const char * const level_prefixes[] = { "",
                                          ANSI_BOLD,
                                          ANSI_BOLD ANSI_COLOR_YELLOW,
                                          ANSI_BOLD ANSI_COLOR_RED,
                                          ANSI_BOLD ANSI_BGCOL_RED,
                                          ANSI_REVERSE };

  const char * const level_names[] = { "DEBUG",
                                       "INFO",
                                       "WARNING",
                                       "ERROR",
                                       "CRITICAL",
                                       "INVALID_LEVEL" };

  const char * const level_suffixes[] = { "",
                                          ANSI_BOLD_OFF,
                                          ANSI_COLOR_OFF ANSI_BOLD_OFF,
                                          ANSI_COLOR_OFF ANSI_BOLD_OFF,
                                          ANSI_BGCOL_OFF ANSI_BOLD_OFF,
                                          ANSI_REVERSE_OFF };

  if (level < GetLoggingLevel())
    return;

  if (level > LOGGING_NUM_LEVELS)
    level = LOGGING_NUM_LEVELS;

  LOG_FPRINTF(LOGGING_FILE,
              "%s%s%s:%s:",
              level_prefixes[level],
              level_names[level],
              level_suffixes[level],
              func);

  LOG_VFPRINTF(LOGGING_FILE, format, args);
  LOG_FPRINTF(LOGGING_FILE, "\r\n");
}

void LogAtLevel(LoggingLevel level, const char *func, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vLogAtLevel(level, func, format, args);
  va_end(args);
}
