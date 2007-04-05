#!/bin/sh

outdir="fake_out"
rm -rf $outdir

function mycopy {
  base=`dirname $1`;
  mkdir -p $outdir/$base
  cp -pv $1 $outdir/$1
  ./utf2latin.sh $outdir/$1
};

mycopy "Doxyfile"

ifiles=`find -name "*.c" -exec echo {} \;`

for f in $ifiles;
do
  mycopy $f;
done

ifiles=`find -name "*.cpp" -exec echo {} \;`

for f in $ifiles;
do
  mycopy $f;
done


ifiles=`find -name "*.h" -exec echo {} \;`

for f in $ifiles;
do
  mycopy $f;
done


ifiles=`find -name "*.py" -exec echo {} \;`

for f in $ifiles;
do
  mycopy $f;
done

cd $outdir

doxygen

cd docs/latex

make
pdflatex refman.tex
makeindex refman.tex
pdflatex refman.tex
pdflatex refman.tex
pdflatex refman.tex
pdflatex refman.tex
pdflatex refman.tex
pdflatex refman.tex
