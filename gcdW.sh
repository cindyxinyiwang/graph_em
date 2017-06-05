#!/bin/bash
TS=`date +"%d%b%y_%H%M"`
bname=`basename "$1" | cut -d'.' -f1`
#logname="~/Logs/"$bname"_"$TS".log"
bname=`basename "$1"`
bname="${bname#*.}"
cname=`echo "$2" | tr / _`
lognm="Results/"$3"_gcdW_"$TS".log"

python synth_graphs_model_whole.py --train $1 --test $2 >$lognm 2>&1 
