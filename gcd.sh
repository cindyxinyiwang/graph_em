#!/bin/bash
TS=`date +"%d%b%y_%H%M"`
bname=`basename "$1" | cut -d'.' -f1`
#logname="~/Logs/"$bname"_"$TS".log"
bname=`basename "$1"`
bname="${bname#*.}"
cname=`echo "$2" | tr / _`
lognm="Results/"$1"gcd_"$TS".log"

python synth_graphs.py >$lognm 2>&1 
