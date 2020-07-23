#!/bin/bash

set -euxo pipefail

L10NDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )"
SCRIPTDIR="$L10NDIR/bin"
PODIR="$L10NDIR/pofiles"

# messages.pot will be used as English translation
cp -f $PODIR/messages.pot $PODIR/en.po

# generate all the languages nsh files
python $SCRIPTDIR/build_locale_nsi.py -o $L10NDIR -p $PODIR -l "English"

# convert file to proper content
cd $L10NDIR
iconv -t CP1252 French.nsh > French2.nsh; rm French.nsh; mv French2.nsh French.nsh
iconv -t CP1252 German.nsh > German2.nsh; rm German.nsh; mv German2.nsh German.nsh
iconv -t CP1252 English.nsh > English2.nsh; rm English.nsh; mv English2.nsh English.nsh
iconv -t CP1252 Italian.nsh > Italian2.nsh; rm Italian.nsh; mv Italian2.nsh Italian.nsh
iconv -t CP1252 Spanish.nsh > Spanish2.nsh; rm Spanish.nsh; mv Spanish2.nsh Spanish.nsh

