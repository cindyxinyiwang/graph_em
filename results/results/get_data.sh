#!/bin/bash 
NAME="as"
for SPLIT in {1..4}
do
	grep -E "^smooth count:|^test loglikelihood:" $NAME/split_$SPLIT/* > tmp.txt 
	sed '$!N;s/\n/ /' tmp.txt | sed -n -e 's/.*smooth count:0.0 .*test loglikelihood:\(.*\)/\1/p' > "${NAME}_split_${SPLIT}.txt"
	rm tmp.txt
done
