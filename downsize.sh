#! /bin/bash
# usage: downsize x
# write out every x lines from stdin to stdout

if [ $# -lt 1 ]; then echo "usage: downsize x"; exit; fi

i=0
while read line; do
   if [ $i -eq 0 ]; then echo $line; fi
   i=$[$i+1]
   if [ $i -eq $1 ]; then i=0; fi
done

