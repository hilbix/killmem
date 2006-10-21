/* $Header$
 *
 * Usage: killmem MB
 *
 * Locks memory until you press a key.  Runs as root.
 *
 * Copyright (C)2006 by Valentin Hilbig
 * Public Domain as long as Copyright is retained.
 * Use at own risk, no warranty!
 *
 * $log$
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "killmem_version.h"

/* basically to spare some mempory
 */

static void
on(const char *s, int fd)
{
  int	i;

  for (i=0; s[i]; i++);
  if (i)
    write(fd, s, i);
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
  int	e;

  e	= errno;
  o2("error: ");
  o2(s);
  o2(": ");
  o2(strerror(e));
  o2("\n");
}

static void
ex(const char *s)
{
  err(s);
  exit(1);
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

  o("alloc .. ");
  p	= malloc(mem);
  if (!p)
    ex("malloc");

  o("locking .. ");

  if ((nolock=mlock(p, mem))!=0)
    err("not running as root? memlock failed");

  for (i=0; i<mem; i+=4096)
    {
      if ((i&0xfffff)==0)
        {
	  oint5(i>>20);
	  o("\b\b\b\b\b");
        }
      p[i]	= 1;
    }
  if (!nolock)
    {
      o("press key to continue: ");
      read(1, &c, 1);
      munlock(p, mem);
      free(p);
    }
  return 0;
}
