#!/bin/bash
rm -rf input2.txt
touch input2.txt
for ((m=1;m<=10;++m));do
	for ((n=8;n<=12;++n));do
		(bash chiffon.sh -l 0 -m $m -n $n)>>input2.txt
		(bash chiffon.sh -l 323 -m $m -n $n)>>input2.txt
		(bash chiffon.sh -l 500 -m $m -n $n)>>input2.txt
		(bash chiffon.sh -l 1000 -m $m -n $n)>>input2.txt
	done
done
./check
wait -f
#rm -rf input2.txt
