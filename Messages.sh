#!/bin/bash
#
# Based on extract-messages.sh
# Azael Avalos <coproscefalo@gmail.com>
#

LANGS="es"

EXTRACTRC=$(command -v extractrc)
if [[ -z "$EXTRACTRC" ]]; then
  echo "extractrc not found"
  exit -1
fi

XGETTEXT=$(command -v xgettext)
if [[ -z "$XGETTEXT" ]]; then
  echo "xgettext not found"
  exit -1
fi

MSGMERGE=$(command -v msgmerge)
if [[ -z "$MSGMERGE" ]]; then
  echo "msgmerge not found"
  exit -1
fi

PODIR="$PWD/po"
DIRS="kcm src"
XGETTEXTOPT="--from-code=UTF-8 -C --kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
	      -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kI18N_NOOP2_NOSTRIP:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 \
	      -kki18np:1,2 -kki18ncp:1c,2,3 --msgid-bugs-address=http://sourceforge.net/p/ktoshiba/support-requests/ \
	      --copyright-holder=Azael_Avalos --package-name=ktoshiba --package-version=5.0"

echo "Extracting messages"
for subdir in $DIRS; do
  echo "- Processing $subdir"
  # Additional string for KAboutData
  echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> rc.cpp
  echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> rc.cpp
  # Parse .ui files
  $EXTRACTRC `find $subdir -name \*.ui -print` >> rc.cpp
  # Extract messages
  if [[ $subdir == "src" ]]; then
    $XGETTEXT $XGETTEXTOPT rc.cpp `find $subdir -name \*.cpp -print` -o ${PODIR}/ktoshiba.pot
  elif [[ $subdir == "kcm" ]]; then
    $XGETTEXT $XGETTEXTOPT rc.cpp `find $subdir -name \*.cpp -print` -o ${PODIR}/kcm_ktoshibam.pot
  fi
  # Clean
  rm -f rc.cpp
done

echo "Merging translations"
cd $PODIR
for lang in $LANGS; do
  echo "- Processing $lang"
  $MSGMERGE -o $lang/tmp.po $lang/ktoshiba.po ktoshiba.pot
  mv $lang/tmp.po $lang/ktoshiba.po
  mv $lang/ktoshiba.po $lang/koshiba.po.orig
  sed -e 's,^"Content-Type: text/plain; charset=CHARSET\\n"$,"Content-Type: text/plain; charset=UTF-8\\n",' < $lang/ktoshiba.po.orig > $lang/ktoshiba.po
  rm -f $lang/koshiba.po.orig
  $MSGMERGE -o $lang/tmp.po $lang/kcm_ktoshibam.po kcm_ktoshibam.pot
  mv $lang/tmp.po $lang/kcm_ktoshibam.po
  mv $lang/kcm_ktoshibam.po $lang/kcm_ktoshibam.po.orig
  sed -e 's,^"Content-Type: text/plain; charset=CHARSET\\n"$,"Content-Type: text/plain; charset=UTF-8\\n",' < $lang/kcm_ktoshibam.po.orig > $lang/kcm_ktoshibam.po
  rm -f $lang/kcm_koshibam.po.orig
done

echo "Done"
