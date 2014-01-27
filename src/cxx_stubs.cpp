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

#include <cstdlib>

#include "ch.h"

#include "base/logging.h"
#include "base/utility.h"

/**
 * @brief Base of the dynamic shared object.
 *
 * @note There are no dynamic shared objects in this system and this is only
 *       used by __cxa_atexit, so it isn't initialized.
 */
void *__dso_handle;

/**
 * @brief Implementation of new using ::malloc.
 *
 * @param n Size of memory to allocate for object.
 * @return Pointer to allocated memory. Halts system if out of space.
 */
void * operator new(size_t n) {
  void * const p = ::malloc(n);
  if (p == nullptr) {
    CriticalHalt("Heap exhausted; halting.");
  }
  return p;
}

/**
 * @brief Implementation of delete using ::free.
 *
 * @param p Pointer to memory of destroyed object.
 */
void operator delete(void *p) {
  ::free(p);
}

/**
 * @brief Handler for calling a pure virtual function.
 */
extern "C" NORETURN void __cxa_pure_virtual() {
  CriticalHalt("Pure virtual called; halting.");
}

#if DISABLE_FLOAT_TO_STRING
// Forward declaration. Not including <sys/reent.h> to avoid bloat.
struct _reent;
/**
 * @brief Stub for dtoa (double-precision floating point to string).
 *
 * @note Replaces the newlib implementation, which is 4.7K in .text.
 *
 * @note Will halt system with a critical error if called.
 */
extern "C" NORETURN char *_dtoa_r(_reent *,
                                  double,
                                  int,
                                  int,
                                  int *,
                                  int *,
                                  char **) {
  CriticalHalt("Double to string conversion disabled; halting.");
}
#endif  // #if DISABLE_DOUBLE_TO_STRING
