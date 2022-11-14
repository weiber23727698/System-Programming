#!/bin/bash

col=5
row=834
epoch=1000

./gen $row $col $epoch
time ./main -t 2 ./in.txt ./out.txt

result=$(diff out.txt ans.txt)
if [ "$result" = "" ]; then
	echo "Correct"
else
	diff out.txt ans.txt
fi

exit
