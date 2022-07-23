#!/bin/bash

dir=$1
mkdir -p $dir/Nil
reqd_folders=$(find ./$dir -name '*.*' -type f | sed -r 's/.*\.//' | sort  -u |sed -r "s/^/$dir\//" )

mkdir -p $reqd_folders

find ./$dir  -type f \
    | while IFS= read -r line 
        do
            file=$(basename $line)
            ext=${line##*.}
            [ ".$ext" = "$line" ] && mv $line $dir/Nil/$file  || mv $line $dir/$ext/$file            
        done

for itr in ./$dir/*;do
    if [ -d $itr  ]; then
        
        
        pat=$(echo $itr | sed 's/\.\///' )
        
        echo "$reqd_folders"  | grep -q -F -x "$pat"   || [ "$pat" = "$dir/Nil" ] || rm -r $itr
        
    fi
done











