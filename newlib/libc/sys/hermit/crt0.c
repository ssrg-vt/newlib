/*
 * Copyright (c) 2016, Stefan Lankes, RWTH Aachen University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the University nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int libc_sd = -1;

extern int main(int argc, char** argv);
extern void __libc_init_array(void);
extern void __libc_fini_array (void);
extern int _init_signal(void);
extern char** environ;

/* Used by the stack transformation runtime to identify the 'base' of the stack,
 * i.e. parts that should not be rewritten */
void *__popcorn_stack_base;

#define PHYS	0x800000ULL

int libc_start(int argc, char** argv, char** env)
{
   int ret;

  /* Setup the stack base location for popcorn - this needs to be done before
        * the constructors are called as the stack transformation runtime actually
        * looks for it in a constructor */
   __popcorn_stack_base = argv;
   if (env) {
      environ = env;
          __popcorn_stack_base = env;
   }

    /* call init function */
   __libc_init_array();

   /* register a function to be called at normal process termination */
   atexit(__libc_fini_array);

   /* optind is the index of the next element to be processed in argv */
   optind = 0;

  /* initialize simple signal handling */
   _init_signal();

   ret = main(argc, argv);

   /* call exit from the C library so atexit gets called, and the
      C++ destructors get run. This calls our exit routine below    
      when it's done. */
   exit(ret);

   /* we should never reach this point */
   return 0;
}

#ifdef __x86_64__
int get_cpufreq(void)
{
   return *((int*) (PHYS + 0x18));
}

int isle_id(void)
{
   return *((int*) (PHYS + 0x34));
}

int get_num_cpus(void)
{
   return *((int*) (PHYS + 0x20));
}
#elif defined(__aarch64__)
int get_num_cpus(void)
{
   return *((int*) (PHYS + 0x120));
}
#endif
