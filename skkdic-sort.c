/* SKK JISYO TOOLS (SKK dictionary handling tools)
Copyright (C) 1994, 1996, 1999, 2000
      Hironobu Takahashi, Masahiko Sato, Kiyotaka Sakai

Author: Hironobu Takahashi, Masahiko Sato, Kiyotaka Sakai, Kenji Yabuuchi
Maintainer: Mikio Nakajima <minakaji@osaka.email.ne.jp>
Version: $Id: skkdic-sort.c,v 1.1.1.1 2000/03/12 12:17:42 minakaji Exp $
Keywords: japanese
Last Modified: $Date: 2000/03/12 12:17:42 $

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

/* skkjisyo-sort.c
   ���Υץ����� SKK �μ���������Ԥ��ޤ�
 */

#include <config.h>
#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

/* ���Υץ����Ǥϼ���򤹤٤ƥ��������ɤ߹��ߤޤ���
   �����ͤϤ��餫�����Ѱդ������ȡ���­���������ɲä�
   �������̤��ꤷ�ޤ������ޤ꾮���������ˤ� realloc()
   ��ȯ�����ޤ���*/

#define STEP (256*1024)

/* ����ϼ��Υݥ��󥿤ˤ�äƺ����줿����˽񤭹��ޤ�ޤ� */
static unsigned char *dict;
static unsigned int dictsize;

/* ����Υ���ǥ����ꥹ�ȤǤ� */
static unsigned int *index;

/* �����Ȥǻ��Ѥ���ݥ��󥿤Ǥ� */
static unsigned int *list;
static unsigned int n_line;

/* ������ɤ߹��ߥץ���� */
static void
readdict()
{
  unsigned int bufsize, incsize;

  dictsize = 0;

  /* �����ѤΥ���Ȥ��� STEP �Х��Ȥ��Ѱդ��ޤ� */
  dict = malloc(STEP);
  bufsize = STEP;
  incsize = fread(dict+dictsize, 1, bufsize-dictsize, stdin);
  dictsize += incsize;

  /* �⤷����դʤ�� STEP �Х��Ȥ������䤷���ɤ߹��ߤ򷫤��֤��ޤ� */
  while (dictsize == bufsize) {
    bufsize += STEP;
    dict = realloc(dict, bufsize);
    incsize = fread(dict+dictsize, 1, bufsize-dictsize, stdin);
    dictsize += incsize;
  }
}

/* ����ǥ�������� */
static void
make_index()
{
  int i, n;

  n = 0;
  for (i = 0; i < dictsize; ++ i)
    if (dict[i] == '\n')
      ++ n;
  n_line = n;

  index = malloc((sizeof *index)*(n_line+1));
  n = 0;
  index[0] = 0;
  for (i = 0; i < dictsize; ++ i)
    if (dict[i] == '\n')
      index[++n] = i+1;

  list = malloc((sizeof *list)*n_line);
  for(i = 0; i < n_line; ++ i)
    list[i] = i;
}

/* �����Ȥ����ʸ������ӥ롼����
   ����ʤ��Υ���ȥ�ǰ��� a ����ˤ���٤����� -1 ���֤���
   ����ʳ��λ��� 1 ���֤� */
static int
hexcomp(a, b)
     unsigned char *a;
     unsigned char *b;
{
  while(*a == *b) {
    ++a; ++ b;
  }
  if (*a > *b) return 1;
  return -1;
}

/* ���꤬�ʤ����뤫�ɤ���Ƚ�Ǥ���  0-�ʤ� 1-���� */
static int okuriari(p)
     unsigned char *p;
{
  if ((p[0] & 0x80) == 0) return 0;
  while(*p != ' ')
    if (*p == '\0') return 0; /* ���򤬤ʤ��Ԥ������ϰ۾�Ȥ��ư����٤� */
    else            ++ p;
  if (('a' <= p[-1]) && (p[-1] <= 'z')) return 1;
  return 0;
}

/* �����Ȥ������ӥ롼����
   ���� a ����ˤ���٤����� -1 ���֤�������ʳ��λ��� 1 ���֤� */
int skkcompar(a, b)
     int *a;
     int *b;
{
  unsigned char *ptra, *ptrb;
  ptra = dict+index[*a];
  ptrb = dict+index[*b];

  if (okuriari(ptra)) {
    if (okuriari(ptrb))
      /* ����������겾̾������Τǡ�a b ��򴹤�����Ӥ��� */
      return hexcomp(ptrb, ptra);
    else
      return -1;  /* a ���� */
  } else {
    if (okuriari(ptrb))
      return 1;  /* a ���� */
    else
      /* ����������겾̾���ʤ��Τǡ�a b �򤽤Τޤ���Ӥ�����̤��֤� */
      return hexcomp(ptra, ptrb);
  }
}

/* ��̤���� */
static void printout()
{
  int i;
  unsigned char *ptr, *line;
  int okuriflag;

  puts(";; okuri-ari entries.");
  okuriflag = 1;
  for(i = 0; i < n_line; ++ i) {
    line = dict+index[list[i]];
    if (line[0] == ';') continue; /* �����ȹԤϽ��Ϥ��ʤ� */
    if (okuriflag) {
      if (!okuriari(line)) {
	puts(";; okuri-nasi entries.");
	okuriflag = 0;
      }
    }
    for (ptr = line; *ptr != '\n'; ++ ptr)
      putchar(*ptr);
    putchar('\n');
  }
}

/* �ᥤ��ץ���� �����ϻ��Ѥ��ޤ��� */
int main(argc, argv)
     int argc;
     char **argv;
{
  readdict();
  make_index();
  qsort((char *)list, n_line, sizeof *list, skkcompar);
  printout();
  return 0;
}

/* end of skkdic-sort.c */
