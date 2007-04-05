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
 iconv -f LATIN1 -t UTF-8 $file.orig > $file
 rm $file.orig
else
 echo "Cannot read $file"
fi

