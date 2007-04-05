#!/bin/sh

if [ $# -ne 1 ]; then
        echo "Usage: $0 file"
        exit 1
fi

file=$1;

if [ -d $file ]; then
 echo "$file is a dir";
 exit 1
fi

if [ -s $file ]; then
 echo "Converting $file..."
 cp $file $file.orig
 iconv -f UTF-8 -t LATIN1 $file.orig > $file
 rm $file.orig
else
 echo "Cannot read $file"
fi

