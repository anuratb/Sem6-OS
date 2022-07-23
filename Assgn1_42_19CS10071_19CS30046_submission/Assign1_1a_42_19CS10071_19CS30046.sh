#!/bin/bash
n=$1
for((a=2;a<=n;a++))
do
    (!((n%a))) && echo -n "$a " 
    (!((n%a))) && ((n/=(a--)))
done
