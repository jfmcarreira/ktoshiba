
PACKAGE=ktoshiba
PACKAGE2=kcmktoshiba

catalogs="es fr hu"
for cat in $catalogs; do
  msgmerge -o ./$cat/$PACKAGE.new ./$cat/$PACKAGE.po $PACKAGE.pot
  if test -s ./$cat/$PACKAGE.new; then
    grep -v "\"POT-Creation" ./$cat/$PACKAGE.new > ./$cat/$PACKAGE.new.2
    grep -v "\"POT-Creation" ./$cat/$PACKAGE.po >> ./$cat/$PACKAGE.new.1
    if diff ./$cat/$PACKAGE.new.1 ./$cat/$PACKAGE.new.2; then
	rm ./$cat/$PACKAGE.new
    else
	mv ./$cat/$PACKAGE.new ./$cat/$PACKAGE.po
    fi
    rm -f ./$cat/$PACKAGE.new.1 ./$cat/$PACKAGE.new.2
  fi
done

for cat in $catalogs; do
  msgmerge -o ./$cat/$PACKAGE2.new ./$cat/$PACKAGE2.po $PACKAGE2.pot
  if test -s ./$cat/$PACKAGE2.new; then
    grep -v "\"POT-Creation" ./$cat/$PACKAGE2.new > ./$cat/$PACKAGE2.new.2
    grep -v "\"POT-Creation" ./$cat/$PACKAGE2.po >> ./$cat/$PACKAGE2.new.1
    if diff ./$cat/$PACKAGE2.new.1 ./$cat/$PACKAGE2.new.2; then
	rm ./$cat/$PACKAGE2.new
    else
	mv ./$cat/$PACKAGE2.new ./$cat/$PACKAGE2.po
    fi
    rm -f ./$cat/$PACKAGE2.new.1 ./$cat/$PACKAGE2.new.2
  fi
done
