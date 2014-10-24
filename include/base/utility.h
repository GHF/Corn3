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

#ifndef BASE_UTILITY_H_
#define BASE_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

// Compiler-specific attribute definitions.
#ifdef __GNUC__
#define ALIGNED(x) __attribute__((aligned(x)))
#define FORMAT(archetype, format_index, first_arg) \
    __attribute__((__format__(archetype, format_index, first_arg)))
#ifdef __cplusplus
#define NORETURN [[noreturn]]
#else
#define NORETURN __attribute__((noreturn))
#endif  // #ifdef __cplusplus
#define UNUSED __attribute__((unused))
#define USED __attribute__((used))
#define UNREACHABLE() __builtin_unreachable()
#else
#define ALIGNED(x)
#define FORMAT(archetype, format_index, first_arg)
#define NORETURN
#define UNUSED
#define USED
#define UNREACHABLE() do { } while (0)
#endif

/**
 * @brief  Expands arguments, then passes them to a macro or function.
 *
 * @note   Useful for passing multiple arguments to a macro as a single macro,
 *         e.g.
 *
 *         #define FN(a, b)  (a + b)
 *         #define ARGS      1, 2
 *
 *         FN(ARGS);          // this doesn't work
 *         INVOKE(FN, ARGS);  // this works
 *
 * @param invokable  Macro or function with zero or more arguments.
 * @param ...        Arguments to pass to invokable.
 */
#define INVOKE(invokable, ...) invokable(__VA_ARGS__)

/**
 * @brief Performs an immediate software reset on the system.
 */
NORETURN USED void SystemReset(void);

/**
 * @brief Logs a critical error then halts the system.
 *
 * @param format Message to display.
 * @param ... Arguments to pass to logging.
 */
#define CriticalHalt(...) _CriticalHalt(__func__, __VA_ARGS__)

/**
 * @brief Logs a critical error then halts the system.
 *
 * @param func Name of the function the error originated from.
 * @param format Message to display.
 * @param ... Arguments to pass to logging.
 */
NORETURN void _CriticalHalt(const char *func, const char *message, ...)
    FORMAT(__printf__, 2, 3);

/**
 * @brief Checks that a condition is true. If not, logs a critical error with
 *        the location and condition checked, then halts.
 */
#define CHECK(condition) do {                               \
    if (!(condition)) {                                     \
      CriticalHalt("CHECK(" #condition ") failed. (%s:%d)", \
                   __FILE__,                                \
                   __LINE__);                               \
    }                                                       \
  } while(0)

/* ANSI color codes. */
#define ANSI_RESET            "\x1b[0m"
#define ANSI_BOLD             "\x1b[1m"
#define ANSI_UNDERLINE        "\x1b[4m"
#define ANSI_REVERSE          "\x1b[7m"
#define ANSI_BOLD_OFF         "\x1b[22m"
#define ANSI_UNDERLINE_OFF    "\x1b[24m"
#define ANSI_REVERSE_OFF      "\x1b[27m"
#define ANSI_COLOR_BLACK      "\x1b[30m"
#define ANSI_COLOR_RED        "\x1b[31m"
#define ANSI_COLOR_GREEN      "\x1b[32m"
#define ANSI_COLOR_YELLOW     "\x1b[33m"
#define ANSI_COLOR_BLUE       "\x1b[34m"
#define ANSI_COLOR_MAGENTA    "\x1b[35m"
#define ANSI_COLOR_CYAN       "\x1b[36m"
#define ANSI_COLOR_WHITE      "\x1b[37m"
#define ANSI_COLOR_OFF        "\x1b[39m"
#define ANSI_BGCOL_BLACK      "\x1b[40m"
#define ANSI_BGCOL_RED        "\x1b[41m"
#define ANSI_BGCOL_GREEN      "\x1b[42m"
#define ANSI_BGCOL_YELLOW     "\x1b[43m"
#define ANSI_BGCOL_BLUE       "\x1b[44m"
#define ANSI_BGCOL_MAGENTA    "\x1b[45m"
#define ANSI_BGCOL_CYAN       "\x1b[46m"
#define ANSI_BGCOL_WHITE      "\x1b[47m"
#define ANSI_BGCOL_OFF        "\x1b[49m"

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* BASE_UTILITY_H_ */
