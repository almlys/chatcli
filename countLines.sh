#!/bin/sh

one=`find -name "*.cpp" -exec cat \{\} \; | wc -l`
onee=`find -name "*.c" -exec cat \{\} \; | wc -l`
oness=`find -name "*.h" -exec cat \{\} \; | wc -l`
let one=$one+$ones+$oness
two=`find -name "*.py" -exec cat \{\} \; | wc -l`
three=`find -name "*.sh" -exec cat \{\} \; | wc -l`
let res=$one+$two+$three
echo "C/C++ lines: $one"
echo "Python lines: $two"
echo "Script lines: $three"
echo "Total: $res"
