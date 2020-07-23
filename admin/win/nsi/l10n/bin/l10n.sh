#!/bin/bash

set -euxo pipefail

L10NDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )"
SCRIPTDIR="$L10NDIR/bin"
PODIR="$L10NDIR/pofiles"

# messages.pot will be used as English translation
cp -f $PODIR/messages.pot $PODIR/en.po

# generate all the languages nsh files
python $SCRIPTDIR/build_locale_nsi.py -o $L10NDIR -p $PODIR -l "English"

# for future references: the windows code pages
# 874 — Thai
# 932 — Japanese
# 936 — Chinese (simplified) (PRC, Singapore)
# 949 — Korean
# 950 — Chinese (traditional) (Taiwan, Hong Kong)
# 1200 — Unicode (BMP of ISO 10646, UTF-16LE)
# 1201 — Unicode (BMP of ISO 10646, UTF-16BE)
# 1250 — Latin (Central European languages)
# 1251 — Cyrillic
# 1252 — Latin (Western European languages, replacing Code page 850)
# 1253 — Greek
# 1254 — Turkish
# 1255 — Hebrew
# 1256 — Arabic
# 1257 — Latin (Baltic languages)
# 1258 — Vietnamese

# convert file to proper content
#cd $L10NDIR
#iconv -c -t CP1252 German.nsh > German2.nsh
#iconv -c -t CP1252 English.nsh > English2.nsh
#iconv -c -t CP1252 Italian.nsh > Italian2.nsh
#iconv -c -t CP1252 Spanish.nsh > Spanish2.nsh

