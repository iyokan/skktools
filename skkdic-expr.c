/* SKK JISYO TOOLS (SKK dictionary handling tools)
Copyright (C) 1994, 1996, 1999, 2000
      Hironobu Takahashi, Masahiko Sato, Kiyotaka Sakai, Kenji Yabuuchi

Author: Hironobu Takahashi, Masahiko Sato, Kiyotaka Sakai, Kenji Yabuuchi
Maintainer: Mikio Nakajima <minakaji@osaka.email.ne.jp>
Version: $Id: skkdic-expr.c,v 1.1.1.1 2000/03/12 12:17:42 minakaji Exp $
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

/* skkjisyo-expr.c
   ���Υץ����� SKK �μ���Υޡ���������Ԥ��ޤ���
 */

#include <config.h>
#include <stdio.h>

#ifdef HAVE_LIBNDBM 
#if defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ > 0 
#include <db1/ndbm.h>
#else
#include <ndbm.h>
#endif
#endif

#include <errno.h>
#include <signal.h>

#ifdef HAVE_FCNTL_H 
#include <fcntl.h>
#endif

#ifndef errno
extern int errno;
#endif

/* ���ΰ���ե�����ξ��˽�ʬ�ʶ������ʤ���С��ѹ��������������Ǥ��礦 */
/* �� #define TMPDIR "." */

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

/* 1994 ǯ�Ǥ� SKK ����ǤϺ�Ĺ�ιԤǤ� 656 ʸ���Ǥ��������Τ����
   �ʲ����ͤȤ��Ƥ��ޤ���*/

#ifdef MAXLINE
#define BLEN MAXLINE
#else
#define BLEN 4096
#endif

/* ����ѥե�����̾ */
char file_name[256];
char okuri_tail_name[256];
char okuri_head_name[256];

/* ����ѥǡ����١��� */
DBM *db;
DBM *okuriheaddb;
DBM *okuritaildb;
FILE *dbcontent;

/* ���꤬�ʤ��Ĥ�������ȥ��ޤ�ƽ�����Ԥ碌�뤫�ɤ��� */
int	okurigana_flag;

static int add_content_line();
static void subtract_content_line();

/* Part1: ����ǡ����١����������ץ���� */

/* �ե�������
   ���Υե����뤬�ʤ��Ƥ⤫�ޤ�ʤ�������ʳ������꤬����Хץ�������� */
static void db_remove_file(fname)
     char *fname;
{
    if (unlink(fname) == -1)
	if (errno != ENOENT) {
	    perror(fname);
	    exit(1);
	}
}

/* ����ѥǡ����١����ե��������
   file_name �ˤ� content ����Ǽ����� */
static void db_remove_files()
{
    char pag_name[256];
    char dir_name[256];

    db_remove_file(file_name);
    sprintf(pag_name, "%s.pag", file_name);
    db_remove_file(pag_name);
    sprintf(dir_name, "%s.dir", file_name);
    db_remove_file(dir_name);
    sprintf(dir_name, "%s.db", file_name);
    db_remove_file(dir_name);

    if (okurigana_flag) {
	db_remove_file(okuri_head_name);
	sprintf(pag_name, "%s.pag", okuri_head_name);
	db_remove_file(pag_name);
	sprintf(dir_name, "%s.dir", okuri_head_name);
	db_remove_file(dir_name);
	sprintf(dir_name, "%s.db", okuri_head_name);
	db_remove_file(dir_name);

	db_remove_file(okuri_tail_name);
	sprintf(pag_name, "%s.pag", okuri_tail_name);
	db_remove_file(pag_name);
	sprintf(dir_name, "%s.dir", okuri_tail_name);
	db_remove_file(dir_name);
	sprintf(dir_name, "%s.db", okuri_tail_name);
	db_remove_file(dir_name);
    }
}

/* �ǡ����١����ե��������� */
static void db_make_files()
{
    db_remove_files();
    if ((db = dbm_open(file_name, O_RDWR|O_CREAT, 0600)) == NULL){
	perror(file_name);
	exit(1);
    }
    if ((dbcontent = fopen(file_name, "w+")) == NULL){
	perror(file_name);
	exit(1);
    }
    if (okurigana_flag) {
	if ((okuriheaddb = dbm_open(okuri_head_name, O_RDWR|O_CREAT, 0600)) 
	    == NULL){
	    perror(okuri_head_name);
	    exit(1);
	}
	if ((okuritaildb = dbm_open(okuri_tail_name, O_RDWR|O_CREAT, 0600)) 
	    == NULL){
	    perror(okuri_tail_name);
	    exit(1);
	}
    }
}

/* �ǡ����١����ե�����˿������ɲä��� */
static void db_append(key, new, dbm)
     datum key;
     unsigned char *new;
     DBM *dbm;
{
    long pos;
    datum content;

    fseek(dbcontent, 0L, 2);
    pos = ftell(dbcontent);
    fwrite(new, strlen(new)+1, 1, dbcontent);
    content.dptr = (char *)&pos;
    content.dsize = sizeof pos;
    dbm_store(dbm, key, content, DBM_REPLACE);
}

/* �ǡ����١�����Υݥ�������׻����� (alignment ������) */
static long getpos(ptr)
     unsigned char *ptr;
{
    long i;
    union long_dat {
	long pos;
	unsigned char c[sizeof i];
    } dat;
    for (i = 0; i < (sizeof i); ++ i)
	dat.c[i] = ptr[i];
    return dat.pos;
}

/* �ǡ����١����ե���������Ƥ�����꾮������Τ��ִ����� */
static void db_replace(ptr, new)
     unsigned char *ptr;
     unsigned char *new;
{
    fseek(dbcontent, getpos(ptr), 0);
    fwrite(new, strlen(new)+1, 1, dbcontent);
}

/* �ǡ����١����ե����뤫��ʸ������� */
static void db_gets(ptr, len, fp)
     unsigned char *ptr;
     int len;
     FILE *fp;
{
    while((*(ptr++) = getc(fp)) != '\0');
}

/* Part2: ��������� */

RETSIGTYPE
signal_handler (signo)
     int signo;
{
    db_remove_files();
    signal(signo, SIG_DFL);
    kill(getpid(), signo);
}

RETSIGTYPE
set_signal_handler()
{
#ifdef SIGQUIT
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, signal_handler);
#endif
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, signal_handler);
#ifdef SIGALRM
    if (signal(SIGALRM, SIG_IGN) != SIG_IGN)
	signal(SIGALRM, signal_handler);
#endif
#ifdef SIGHUP
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	signal(SIGHUP, signal_handler);
#endif
    if (signal(SIGSEGV, SIG_IGN) != SIG_IGN)
	signal(SIGSEGV, signal_handler);
#ifdef SIGBUS
    if (signal(SIGBUS, SIG_IGN) != SIG_IGN)
	signal(SIGBUS, signal_handler);
#endif
}

/* Part3: ����ե�����κ��� */

/* base ��ʸ����� *s �� *e �˶��ޤ줿ʸ���󤬤��뤫�ɤ���Ĵ�٤�
   0- �ʤ�  1- ���� */
static int match_item(base, s, e)
     unsigned char *base;
     unsigned char *s;
     unsigned char *e;
{
    unsigned char *p, *q1, *q2;

    for (p = base; *p != '\0'; ++ p) {
	if (*p == *s) {
	    for(q1 = p+1, q2 = s+1; q2 <= e; ++ q1, ++ q2)
		if (*q1 != *q2) goto next;
	    return 1; /* matched */
	}
      next:;
    }
    return 0;
}

/* base ��ʸ����� *s �� *e �˶��ޤ줿ʸ������ɲ� */
static void append_item(base, s, e)
     unsigned char *base;
     unsigned char *s;
     unsigned char *e;
{
    unsigned char *p, *q;

    for (p = base; *p >= 0x20; ++ p);
    for (q = s+1; q <= e; ++ q, ++ p)
	*p = *q;
    *p = '\0';
}

/* �����꤬�ʥ���ȥ�ϡ�"��ꤳm��"�Τ褦�ʥ����������
 * �����ͤϡ�"/����/���/"�Ȥʤ�
 */
static char *add_okuri_item(key, s)
     datum 		*key;
     unsigned char 	*s;
{
    unsigned char	*p, *headtop;
    unsigned char	keybuf[BLEN];
    unsigned char	content[BLEN];
    unsigned char	new[BLEN];

    int			len;
    datum		otails, oheads;
    datum		tkey, hkey;

    /* ���Ф��򥳥ԡ� */
    strncpy(keybuf, key->dptr, key->dsize);
    tkey.dptr = (char *)keybuf;
    headtop = keybuf + key->dsize;	/* ������ɤ��˥��ԡ����뤫 */

    /* ������tkey�˥��ԡ� */
    for (p = headtop; *s != '/'; s++, p++) {
	if (*s < 0x20) return;
	*p = *s;
    }
    tkey.dsize = p - keybuf;

    /* �촴��ʬ��content�˥��ԡ����� */
    p = content; 
    for( ; *s != ']'; s++, p++) {
	if (*s < 0x20) return;
	*p = *s;
    }
    *p = '\0';
    if (*++s != '/') 
	return ;		/* �ե����ޥåȥ��顼 */
    
    /* �Ť���Τ���٤�ɬ�פʤ�append */
    otails = dbm_fetch(okuriheaddb, tkey);
    if (otails.dptr == NULL)  {
	db_append(tkey, content, okuriheaddb);
    } else {
	fseek(dbcontent, getpos(otails.dptr), 0);
	db_gets(new, BLEN, dbcontent);
	if (add_content_line(new, content, NULL))
	    db_append(tkey, new, okuriheaddb);
    }
}

/* tails�ǻؤ��Ƥ���ʸ�����line�����Ƥ��������������ɲ�
 * ���ΤȤ�ʸ�����"/[������/[������/"�Τ褦�ʷ����ȤʤäƤ���
 */
static int add_okuri_tail_line(tails, line)
     unsigned char 	*tails, *line;
{
    unsigned char     	*s, *e;
    int n_add;

    n_add = 0;
    for(s = line; (0x20 <= *s); ++ s) {
	if (*s == '/') {
	    if (s[1] == '[') {
		for(e = s+2; (0x20 <= *e); ++e) {
		    if (*e == '/') {
			if (!match_item(tails, s, e)) {
			    append_item(tails, s, e);
			    n_add ++;
			}
			s = e-1;
			goto next;
		    }
		}
		return n_add;
	    }
	}
      next:;
    }
    return n_add;
}

/* base �������Ƥ���ʸ����� new �����Ƥǽ�ʣ���ʤ���Τ�ä���
   ����ä��ʤ���� 0 �򡢤���ʳ��ϲä����ϸ�����֤� */
static int add_content_line(base, new, key)
     unsigned char *base;
     unsigned char *new;
     datum   	   *key;
{
    unsigned char *s, *e;
    int n_add;

    n_add = 0;
    for(s = new; (0x20 <= *s); ++ s) {
	if (*s == '/') {
	    if (s[1] == '[') {
		if (okurigana_flag) 
		    add_okuri_item(key, s + 2);
		for(s++ ; *s != ']'; ++s)
		    if (*s < 0x20) return n_add;
		goto next;
	    }
	    for(e = s+1; (0x20 <= *e); ++ e) {
		if (*e == '/') {
		    if (!match_item(base, s, e)) {
			append_item(base, s, e);
			n_add ++;
		    }
		    s = e-1;
		    goto next;
		}
	    }
	    return n_add;
	}
      next:;
    }
    return n_add;
}

/* �����ĤΡֽ����פ����뤫������ */
static int item_number(p)
     unsigned char *p;
{
    int n = 0;
    for (p++ ; *p >= 0x20; ++ p)
	if (*p == '/') ++ n;
    return n;
}

static int line_process(buffer, key, kanji, tails)
     unsigned char *buffer;
     datum *key;
     unsigned char *kanji;
     unsigned char *tails;
{
    int key_len;

    if ((buffer[0] == ';') || (buffer[0] == '\0')) return 0;
    for (key_len = 1;
	(buffer[key_len] != ' ') || (buffer[key_len+1] != '/');
	++ key_len)
	if (buffer[key_len] == '\0') return 0;
    key->dptr = buffer;
    key->dsize = key_len;

    kanji[0] = tails[0] = '/';
    kanji[1] = tails[1] = '\0';
    add_content_line(kanji, buffer+key_len+1, key);
    if (okurigana_flag) 
	add_okuri_tail_line(tails, buffer+key_len+1);
    return item_number(kanji);
}

/* ���ꤵ�줿̾���Υե���������Ƥ�ä��� */
static void add_file(srcname)
     char *srcname;
{
    static unsigned char buffer[BLEN], kanji[BLEN], new[BLEN];
    static unsigned char tails[BLEN], tkeybuf[BLEN];
    datum key, old, tkey;
    FILE *fp;

    if ((fp = fopen(srcname, "r")) == NULL) {
	perror(srcname);
	return;
    }
    while(fgets(buffer, BLEN, fp) != NULL) {
	if (line_process(buffer, &key, kanji, tails) > 0) {
	    if (okurigana_flag) {
		/* ��������Ͽ */
		if (tails[1] != '\0') {
		    old = dbm_fetch(okuritaildb, key);
		    if (old.dptr == NULL) {
			db_append(key, tails, okuritaildb);
		    } else {
			fseek(dbcontent, getpos(old.dptr), 0);
			db_gets(new, BLEN, dbcontent);
			if (add_okuri_tail_line(new, tails))
			    db_append(key, new, okuritaildb);
		    }
		}
	    }

	    old = dbm_fetch(db, key);
	    if (old.dptr == NULL) {
		db_append(key, kanji, db);
	    } else {
		fseek(dbcontent, getpos(old.dptr), 0);
		db_gets(new, BLEN, dbcontent);
		if (add_content_line(new, kanji, NULL))
		    db_append(key, new, db);
	    }
	}
    }
    fclose(fp);
}

static void
delete_item(base, s, e)
     unsigned char *base;
     unsigned char *s;
     unsigned char *e;
/* base ��ʸ������� *s �� *e �˶��ޤ줿ʸ���󤬤���к�� */
{
    unsigned char *p, *q1, *q2;

    for (p = base; *p != '\0'; ++ p) {
	if (*p == *s) {
	    for(q1 = p+1, q2 = s+1; q2 <= e; ++ q1, ++ q2)
		if (*q1 != *q2) goto next;
	    /* matched */
	    for(q2 = p+1; *q1 != '\0'; ++ q1, ++ q2)
		*q2 = *q1;
	    *q2 = '\0';
	    return;
	}
      next:;
    }
}

/* �����꤬�ʥ���ȥ�ϡ�"��ꤳm��"�Τ褦�ʥ�������Ĥ��Ȥˤʤ�
 * �����ͤϡ�"/����/���/"�Ȥʤ�
 */
static char *subtract_okuri_item(key, s)
     datum 		*key;
     unsigned char 	*s;
{
    unsigned char	*p, *headtop;
    unsigned char	keybuf[BLEN];
    unsigned char	content[BLEN];
    unsigned char	new[BLEN];

    int			len;
    datum		otails, oheads;
    datum		tkey, hkey;

    /* ���Ф��򥳥ԡ� */
    strncpy(keybuf, key->dptr, key->dsize);
    tkey.dptr = (char *)keybuf;
    headtop = keybuf + key->dsize;	/* ������ɤ��˥��ԡ����뤫 */

    /* ������tkey�˥��ԡ� */ 
    for (p = headtop; *s != '/'; s++, p++) {
	if (*s < 0x20) return;
	*p = *s;
    }
    tkey.dsize = p - keybuf;

    /* �촴��ʬ��content�˥��ԡ����� */
    p = content; 
    for( ; *s != ']'; s++, p++) {
	if (*s < 0x20) return;
	*p = *s;
    }
    *p = '\0';
    if (*++s != '/') 
	return ;		/* �ե����ޥåȥ��顼 */
    
    /* �Ť���Τ���٤�ɬ�פʤ�replace/delete */
    otails = dbm_fetch(okuriheaddb, tkey);
    if (otails.dptr != NULL)  {
	fseek(dbcontent, getpos(otails.dptr), 0);
	db_gets(new, BLEN, dbcontent);
	subtract_content_line(new, content, NULL);
	if (strlen(new) >= 3)
	    db_replace(otails.dptr, new);
	else
	    dbm_delete(okuriheaddb, tkey);
    }
}

/* tail�ǻؤ��Ƥ���ʸ���󤫤�line�˴ޤޤ���������
 */
static int subtract_okuri_tail_line(tails, line)
     unsigned char 	*tails, *line;
{
    unsigned char     	*s, *e;

    for(s = line; (0x20 <= *s); ++ s) {
	if (*s == '/') {
	    if (s[1] == '[') {
		for(e = s+2; (0x20 <= *e); ++e) {
		    if (*e == '/') {
			delete_item(tails, s, e);
			s = e-1;
			goto next;
		    }
		}
		return;
	    }
	}
      next:;
    }
    return;
}

static void
subtract_content_line(base, new, key)
     unsigned char *base;
     unsigned char *new;
     datum	   *key;
/* base ��ʸ�����椫�� new ���ʸ������� */
{
    unsigned char *s, *e;

    for(s = new; (0x20 <= *s); ++ s) {
	if (*s == '/') {
	    if (s[1] == '[') {
		if (okurigana_flag)
		    subtract_okuri_item(key, s + 2);
		for(s++; *s != ']'; ++s)
		    if (*s < 0x20) return;
		goto next;
	    }
	    for(e = s+1; (0x20 <= *e); ++ e) {
		if (*e == '/') {
		    delete_item(base, s, e);
		    s = e-1;
		    goto next;
		}
	    }
	    return;
	}
      next:;
    }
}

/* Ϳ����̾���μ�������Ƥ򸽺ߤμ��񤫤������� */
static void subtract_file(srcname)
     char *srcname;
{
    static unsigned char buffer[BLEN], kanji[BLEN], new[BLEN];
    static unsigned char tails[BLEN], tkeybuf[BLEN];
    datum key, old, tkey;
    FILE *fp;

    if ((fp = fopen(srcname, "r")) == NULL) {
	perror(srcname);
	return;
    }
    while(fgets(buffer, BLEN, fp) != NULL) {
	/* �Ԥ�����ɤ�(key)�פȡֽ���(content)�פ���Ф���
	   �⤷�⥳���ȹ����Ǥ�������Ф� */
	if (line_process(buffer, &key, kanji, tails) > 0) {
	    /* ����ˤ��Ǥˤ���Ф��줫�������롣�ʤ���в��⤷�ʤ� */

	    if (okurigana_flag) {
		if (tails[1] != '\0') {
		    old = dbm_fetch(okuritaildb, key);
		    if (old.dptr != NULL) {
			fseek(dbcontent, getpos(old.dptr), 0);
			db_gets(new, BLEN, dbcontent);
			subtract_okuri_tail_line(new, tails);
			if (strlen(new) >= 3)
			    db_replace(old.dptr, new);
			else
			    dbm_delete(okuritaildb, key);
		    }
		}
	    }

	    old = dbm_fetch(db, key);
	    if (old.dptr != NULL) {
		fseek(dbcontent, getpos(old.dptr), 0L);
		db_gets(new, BLEN, dbcontent);
		subtract_content_line(new, kanji, &key);
		if (strlen(new) >= 3)
		    db_replace(old.dptr, new);
		else 
		    dbm_delete(db, key);
	    }
	}
    }
    fclose(fp);
}

static int	okuri_type_out(key, output)
     datum	*key;
     FILE	*output;
{
    unsigned char	*s, *e, *headtop;
    unsigned char	keybuf[BLEN];
    unsigned char	tail_content[BLEN];
    unsigned char	head_content[BLEN];

    datum	tails, tkey;
    datum	one;

    /* ���Ф��򥳥ԡ� */
    strncpy(keybuf, key->dptr, key->dsize);
    tkey.dptr = (char *)keybuf;
    tkey.dsize = key->dsize;
    headtop = keybuf + tkey.dsize;	/* ������ɤ��˥��ԡ����뤫 */

    tails = dbm_fetch(okuritaildb, tkey);
    if (tails.dptr == NULL)  {
	return;
    } else {
	fseek(dbcontent, getpos(tails.dptr), 0);
	db_gets(tail_content, BLEN, dbcontent);

	s = tail_content + 2; 		/* '/'��'['��ȤФ� */
	for(e = s; e[1] != '\0'; s = e + 2) {
	    for (e = s; *e != '/'; e++)
		if (*e < 0x20) 
		    return;

	    strncpy(headtop, s, e - s);
	    tkey.dsize = (headtop - keybuf) + (e - s);
	    one = dbm_fetch(okuriheaddb, tkey);
	    if (one.dptr == NULL) {
		continue;
	    } else {
		fseek(dbcontent, getpos(one.dptr), 0);
		db_gets(head_content, BLEN, dbcontent);

		putc('[', output);
		while (s != e)
		    putc(*s++, output);		    
		fputs(head_content, output);
		fputs("]/", output);
	    }
	}
    }
}

/* ��̤����
   ����Ϥޤä����ΤǤ����ˤʤ�Τǡ��ǽ�Ū�ʷ�̤�����ˤ�
   skkjisyo-sort ���Ѥ���
   */
static void type_out(output)
     FILE *output;
{
    datum key, content;
    int i;
    unsigned char kanji[BLEN];

    for (key = dbm_firstkey(db); key.dptr !=  NULL;  key = dbm_nextkey(db)) {
	content = dbm_fetch(db, key);
	for(i = 0; i < key.dsize; ++ i)
	    putc((key.dptr)[i], output);
	putc(' ', output);
	fseek(dbcontent, getpos(content.dptr), 0);
	db_gets(kanji, BLEN, dbcontent);
	fputs(kanji, output);
	if (okurigana_flag)
	    okuri_type_out(&key, output);
	putc('\n', output);
    }

    if (okurigana_flag) {
	datum 	entry;
	for (key = dbm_firstkey(okuritaildb); 
	     key.dptr !=  NULL;  key = dbm_nextkey(okuritaildb)) {
	    entry = dbm_fetch(db, key);
	    if (entry.dptr != NULL) continue;

	    for(i = 0; i < key.dsize; ++ i)
		putc((key.dptr)[i], output);
	    putc(' ', output);
	    putc('/', output);
	    okuri_type_out(&key, output);
	    putc('\n', output);
	}
    }
}

/* ����ˡ��ɽ�� */
static void print_usage(argc, argv)
     int argc;
     char **argv;
{
    fprintf(stderr,
	    "Usage: %s [-d workdir] [-o output] [-O] jisyo1 [[+-] jisyo2]...\n",
	    argv[0]);
}

/* �ᥤ��ץ����  ������������� */
int main(argc, argv)
     int argc;
     char **argv;
{
    int negate, i;
    FILE *output;
    char *tmpdir;

    output = stdout;
    tmpdir = TMPDIR;

    /* �����ν��� */
    for (i = 1; i < argc; ++ i) {
	if (argv[i][0] == '-') {
	    if (argv[i][1] == 'd') { /* -d ��ȥǥ��쥯�ȥ� */
		tmpdir=argv[i+1];
		i ++;
	    } else if (argv[i][1] == 'o') { /* -o ���ϥե����� */
		if ((output = fopen(argv[i+1], "w")) == NULL) {
		    perror(argv[i+1]);
		    exit(1);
		}
		i ++;
	    } else if (argv[i][1] == 'O') { /* ���꤬�ʤν����λ���  */
		okurigana_flag++;
	    } else {
		print_usage(argc, argv);
		exit(1);
	    }
	} else {
	    break;
	}
    }

    if (i >= argc) { /* ���񤬻��ꤵ��Ƥ��ʤ� */
	print_usage(argc, argv);
	exit(1);
    }

    sprintf(file_name, "%s/skkjisyo.%d", tmpdir, getpid());
    if (okurigana_flag) {
	sprintf(okuri_head_name, "%s/skkhead.%d", tmpdir, getpid());
	sprintf(okuri_tail_name, "%s/skktail.%d", tmpdir, getpid());
    }
    set_signal_handler();
    db_make_files();
    
    negate = 0;
    for (; i < argc; ++ i) {
	if (argv[i][0] == '+') {
	    negate = 0;
	    if (strlen(argv[i]) > 1)
		add_file(argv[i]+1);
	} else if (argv[i][0] == '-') {
	    negate = 1;
	    if (strlen(argv[i]) > 1) {
		subtract_file(argv[i]+1);
		negate = 0;
	    }
	} else {
	    if (negate == 0)
		add_file(argv[i]);
	    else
		subtract_file(argv[i]);
	}
    }
    type_out(output);
    db_remove_files();
    return 0;
}

/* end of skkdic-expr.c */
