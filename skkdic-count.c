/* SKK JISYO TOOLS (SKK dictionary handling tools)
Copyright (C) 1994, 1996, 1999, 2000
      Hironobu Takahashi, Masahiko Sato, Kiyotaka Sakai

Author: Hironobu Takahashi, Masahiko Sato, Kiyotaka Sakai, Kenji Yabuuchi
Maintainer: Mikio Nakajima <minakaji@osaka.email.ne.jp>
Version: $Id: skkdic-count.c,v 1.1.1.1 2000/03/12 12:17:40 minakaji Exp $
Keywords: japanese
Last Modified: $Date: 2000/03/12 12:17:40 $

This file is part of Daredevil SKK

SKK JISYO TOOLS are free software; you can redistribute them and/or modify
them under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

SKK JISYO TOOLS are distributed in the hope that they will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SKK; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* skkjisyo-entries.c
   $B$3$N%W%m%0%i%`$O(B SKK $B$N<-=q$K4^$^$l$k%(%s%H%j!<?t$r%+%&%s%H$7$^$9!#(B
 */

#include <config.h>
#include <stdio.h>

/* 1994 $BG/HG$N(B SKK $B<-=q$G$O:GD9$N9T$G$b(B 656 $BJ8;z$G$9$,0BA4$N$?$a$K(B
   $B0J2<$NCM$H$7$F$$$^$9!#(B*/

#define BUFSIZE 2048

/* $B3F%U%!%$%k$N%(%s%H%j$r%+%&%s%H$7$^$9(B */
static void
count_entry(filename, fp)
     char *filename;
     FILE *fp;
{
  unsigned char buffer[BUFSIZE], *p, *q;
  int count;

  count = 0;
  while(fgets(buffer, BUFSIZE, fp) != NULL) {
    if ((buffer[0] == ';') || (buffer[0] == '\0')) continue;
    for(p = buffer; *p != '\0'; ++ p) {
      if ((p[0] == ' ') && (p[1] == '/')) {
	for (q = p+2; *q != '\0'; ++ q) {
	  if (*q == '[')
	    while((*q != ']') && (*q != '\0')) q ++;
	  if (*q == '/') count++;
	}
	break;
      }
    }
  }
  if (count == 1)
    printf("%s %d entry\n", filename, count);
  else
    printf("%s %d entries\n", filename, count);
}

/* $B%a%$%s%W%m%0%i%`$G$O0z?t$rH=JL$7$^$9!#$b$7$b$J$1$l$PI8=`F~NO(B
   $B$+$i$N%G!<%?$,F@$i$l$k$b$N$H$7$^$9(B */

int main(argc, argv)
     int argc;
     char **argv;
{
  int i;
  FILE *fp;

  if (argc <= 1) {
    count_entry("", stdin);
  } else {
    for (i = 1; i < argc; ++ i) {
      if ((fp = fopen(argv[i], "r")) != NULL) {
	count_entry(argv[i], fp);
	fclose(fp);
      } else {
	perror(argv[i]);
      }
    }
  }
  return 0;
}

/* end of skkdic-count.c */
