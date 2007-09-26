#line 1 "tino/sbrk.h"
/* $Header$
 *
 * Reliable SBRK handling.
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 *
 * $Log$
 * Revision 1.1  2007-09-26 13:24:02  tino
 * sbrk usage fixed.  As sbrk() isn't POSIX implementation seem to vary ..
 *
 */

#ifndef tino_INC_sbrk_h
#define tino_INC_sbrk_h

#include <unistd.h>

/* Let all the various sbrk() implementations reliably return 0 on OOM
 * situations
 */
static void *
tino_sbrk(int mem)
{
  void	*p;

  p	= sbrk(mem);
  if (p && p!=(void *)-1 && (!mem || p!=sbrk(0)))
    return p;
  errno	= ENOMEM;
  return 0;
}

#endif
