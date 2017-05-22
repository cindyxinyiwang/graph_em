#!/bin/bash
TS=`date +"%d%b%y_%H%M"`
bname=`basename "$1" | cut -d'.' -f1`
#logname="~/Logs/"$bname"_"$TS".log"
bname=`basename "$1"`
bname="${bname#*.}"
cname=`echo "$2" | tr / _`
lognm="Results/gem_"$cname"_"$TS".log"

echo $cname
echo $lognm

#parallel "python exp_gcd.py --orig "$1" --cgs "$2" >> Results/gem"$bname".log 2>&1" ::: {1..50}
seq 1 50 | parallel python "exp_gcd.py --orig "$1" --cgs "$2" --xnbr {}"  ">>" $lognm 2>&1 
