#!/bin/bash
rm -rf file_mod
mkdir file_mod

for file in $1/*.txt    
do
    cat -n $file|sed -r "s/^\s+//" |tr -s [:blank:] ',' >./file_mod/$(basename $file)
done
