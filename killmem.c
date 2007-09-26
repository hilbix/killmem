/* $Header$
 *
 * Usage: killmem MB
 *
 * Locks memory until you press a key.  Runs as root.
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 *
 * $Log$
 * Revision 1.3  2007-09-26 12:38:07  tino
 * CLL + sbrk
 *
 * Revision 1.2  2006/10/21 02:07:02  tino
 * CVS comment corrected
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "killmem_version.h"

#define	USE_SBRK

/* little routines to spare some memory
 */

static void
on(const char *s, int fd)
{
  int	e, i;

  e	= errno;
  for (i=0; s[i]; i++);
  if (i)
    write(fd, s, i);
  errno	= e;
}

static void
o(const char *s)
{
  on(s, 0);
}

static void
oint5(int n)
{
  char	buf[100];
  int	i;

  i		= sizeof buf;
  buf[--i]	= 0;
  while (i>=0 && n)
    {
      buf[--i]	= '0'+(n%10);
      n		/= 10;
    }
  while (i>sizeof buf-6)
    buf[--i]	= '0';
  o(buf+i);
}

static void
o2(const char *s)
{
  on(s, 2);
}

static void
err(const char *s)
{
  o2("error: ");
  o2(s);
  o2(": ");
  o2(strerror(errno));
  o2("\n");
}

static void
ex(const char *s)
{
  err(s);
  exit(1);
}

/* Lock memory
 *
 * Returns 1 when there is a problem.
 */
static int
mem_lock(char *p, int mem)
{
  while (mlock(p, mem))
    {
      int	d;

      /* Getting ENOMEM means, that the memory area is too big.
       */
      if (errno!=ENOMEM)
	return 1;

      /* Half the size and try again.
       */
      d	= (mem>>13)<<12;
      if (!d)
	return 1;

      if (mem_lock(p, d))
	return 1;

      /* Show this situation
       */
      o("o");

      /* Now try the other half
       */
      p		+= d;
      mem	-= d;
    }
  return 0;
}

#define	MEM_PAGESIZE	4096

/* Allocate memory
 */
static void *
mem_get(int mem)
{
#ifdef USE_SBRK
  int	d;
  char	*p;

  d	= 0;
  p	= sbrk(d);
  d	= p-(char *)0;
  if (d&(MEM_PAGESIZE-1))
    {
      o("align");
      sbrk(MEM_PAGESIZE-(d&(MEM_PAGESIZE-1)));
      p	= sbrk(d);
      d	= p-(char *)0;
      if (d&(MEM_PAGESIZE-1))
	o(" failed");
      o(" .. ");
    }
  o("sbrk");
  return sbrk(mem);
#else
  o("alloc");
  return malloc(mem);
#endif
}

int
main(int argc, char **argv)
{
  int	mem, i, nolock;
  char	*p, c;

  if (argc!=2)
    {
      o2("Usage: killmem MB\n"
	 "\t\tVersion " KILLMEM_VERSION " compiled " __DATE__ "\n"
	 "\tRun as root to lock memory against paging\n"
	 "\texample: killmem 64\n");
      return 2;
    }

  mem	= atoi(argv[1])*1024*1024;
  if (mem<=0)
    ex("funny arg");

  /* Get the memory
   */
  p	= mem_get(mem);
  if (!p)
    {
      o(" failed\r\n");
      ex("no memory");
    }

  /* Lock the memory
   */
  o(" .. lock ");
  nolock	= mem_lock(p, mem);
  if (nolock)
    {
      o("failed\r\n");
      err("mlock");
    }
  o(".. ");

  /* Initialize the memory, so it is paged into memory
   */
  for (i=0; i<mem; i+=MEM_PAGESIZE)
    {
      if ((i&0xfffff)==0)
        {
	  oint5(i>>20);
	  o("\b\b\b\b\b");
        }
      p[i]	= i;
    }

  /* When locking did not work return immediately as we would only hog
   * the swapspace.
   */
  if (nolock)
    return 3;

  /* Wait for a keypress
   */
  o("press key to continue: ");
  read(1, &c, 1);

  /* Before I unlocked the memory.  However this might trigger
   * swapping.  So now killmem just terminates and thus tears down the
   * lock.
   */
  return 0;
}
