# edict2skk.awk -- convert EDICT dictionary to SKK-JISYO format.
#
# Copyright (C) 1998, 1999, 2000 Mikio Nakajima <minakaji@osaka.email.ne.jp>
#
# Author: Mikio Nakajima <minakaji@osaka.email.ne.jp>
# Created: Dec. 5, 1998
# Last Modified: $Date: 2000/03/12 12:17:51 $
# Version: $Id: edict2skk.awk,v 1.1.1.1 2000/03/12 12:17:51 minakaji Exp $

# This file is part of Daredevil SKK.

# SKK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either versions 2, or (at your option)
# any later version.
#
# SKK is distributed in the hope that it will be useful
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SKK, see the file COPYING.  If not, write to the Free
# Software Foundation Inc., 59 Temple Place - Suite 330, Boston,
# MA 02111-1307, USA.
#
# Commentary:
# This file encoding should be Ja/EUC.
#
# ���Υ�����ץȤϡ�James William Breen �ˤ�� EDICT �� SKK-JISYO �ե���
# �ޥåȤ��Ѵ������ΤǤ����Ǥ������ä�����ϡ�SKK 10.x ���դ��Ƥ���
# skk-look.el ��Ȥ���ͭ�����ѤǤ���ΤǤϤʤ����ȹͤ��Ƥ��ޤ���
#
# EDICT �ϡ�
#   ftp://ftp.u-aizu.ac.jp:/pub/SciEng/nihongo/ftp.cc.monash.edu.au/
# ���� get �Ǥ��ޤ���
#
# ���� edict �� edict2skk.awk �� SKK jisyo-tools �Υ��ޥ�ɤ�ȤäƲ�
# �����ޤ���
#
#   % jgawk -f edict2skk.awk edict > temp
#   % skkdic-expr temp | skkdic-sort > SKK-JISYO.E2J
#   % rm temp
# 
# SKK-JISYO.E2J �λȤ����Ͽ����ͤ����ޤ�����
#   % skkdic-expr SKK-JISYO.E2J + /usr/local/share/skk/SKK-JISYO.L | skkdic-sort > SKK-JISYO.L
# �ʤɤȤ��� SKK Large ����ȥޡ������ƻȤ��Τ���ñ�Ǥ���
# 
# EDICT �ڤӤ��Υ��֥��å� (�ܥ�����ץȤˤ�� EDICT ��ȴ�褷����Τ�
# ���֥��åȤ�������Ǥ��礦) �ϡ�GPL �Ȥϰۤʤ����۾�郎�դ��Ƥ����
# �ǡ��ܺ٤ϡ�EDICT �ե��������Ƭ��ʬ�⤷���ϡ�EDICT ź�դ� edict.doc
# �򻲾Ȥ��Ʋ�������
#
# Code
BEGIN{
  print ";; okuri-ari entry";
  # all entries are `okuri-nasi'.
  print ";; okuri-nasi entry";
}
$1 !~ /^��/ {
    alt_yomi = 0; # initialize
    # plural words that contain spaces cannot be yomi.
    if (match($0, /\/[^ ][^ ]*\/$/) != 0) {
	entries = substr($0, RSTART + 1, RLENGTH - 2);
	num = split(entries, yomi, "/");
	for (i = 1; i <= num; i++) {
	    gsub(/\"/, "", yomi[i]);
	    if (match(yomi[i], /\(-*[a-z]*[a-z]*-*\)/) != 0) {
		head = substr(yomi[i], 1, RSTART - 1);
		middle = substr(yomi[i], RSTART + 1 , RLENGTH - 2);
		tail = substr(yomi[i], RSTART + RLENGTH);
		yomi[i] = head tail;
		gsub(/-/, "", middle);
		if (((middle != "") ||
                     # ����
                     (middle != "d") ||
		     # ʣ����
		     (middle != "s") ||
		     # ʣ����
		     (middle != "es") ||
		     # �ʹԷ�
		     (middle != "ing") ||
		     # �����ػ��Ѹ�
		     (middle != "X") ||
		     # ����
		     (match(middle, /.ed/) == 0) ) &&
		    (yomi[i] != head middle tail) ) {
		    alt_yomi = head middle tail;
		    gsub(/\"/, "", alt_yomi);
		} else {
		    alt_yomi = 0;
		}
		printf("%s /%s/\n", yomi[i], $1);
		if (alt_yomi) {
		    printf("%s /%s/\n", alt_yomi, $1);
		}
	    } else {
		printf("%s /%s/\n", yomi[i], $1);
	    }
	}
    }
}
# end of edict2skk.awk
