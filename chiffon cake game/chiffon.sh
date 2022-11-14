#!/bin/bash

#input
while getopts m:n:l: flag
do
    case "${flag}" in
        m) n_hosts=${OPTARG};;
        n) n_players=${OPTARG};;
        l) lucky_number=${OPTARG};;
    esac
done
#prepare fifo
count=0
fnum=3
while [ $count -le $n_hosts ]; do
    mkfifo fifo_$count.tmp
    if [ $fnum == 3 ]; then
        exec 3<> fifo_0.tmp
    elif [ $fnum == 4 ]; then
        exec 4<> fifo_1.tmp
    elif [ $fnum == 5 ]; then
        exec 5<> fifo_2.tmp
    elif [ $fnum == 6 ]; then
        exec 6<> fifo_3.tmp
    elif [ $fnum == 7 ]; then
        exec 7<> fifo_4.tmp
    elif [ $fnum == 8 ]; then
        exec 8<> fifo_5.tmp
    elif [ $fnum == 9 ]; then
        exec 9<> fifo_6.tmp
    elif [ $fnum == 10 ]; then
        exec 10<> fifo_7.tmp
    elif [ $fnum == 11 ]; then
        exec 11<> fifo_8.tmp
    elif [ $fnum == 12 ]; then
        exec 12<> fifo_9.tmp
    else
        exec 13<> fifo_10.tmp
    fi
    fnum=$(($fnum+1))
    count=$(($count+1))
done
#make host
for i in $(seq 1 $n_hosts); do
    ./host -m $i -d 0 -l $lucky_number &
    pid[${i}]=$!
done
points=(0 0 0 0 0 0 0 0 0 0 0 0 0) #分數
ready=(1 1 1 1 1 1 1 1 1 1 1 1 1 1) #每個host的狀況
readyhost=$n_hosts #有幾個host有空
#combination of players
declare -a player
x=0
for first in $(seq 1 $(($n_players-7))); do
    for second in $(seq $(($first+1)) $(($n_players-6))); do
        for third in $(seq $(($second+1)) $(($n_players-5))); do
            for forth in $(seq $(($third+1)) $(($n_players-4))); do
                for fifth in $(seq $(($forth+1)) $(($n_players-3))); do
                    for sixth in $(seq $(($fifth+1)) $(($n_players-2))); do
                        for seventh in $(seq $(($sixth+1)) $(($n_players-1))); do
                            for eighth in $(seq $(($seventh+1)) $(($n_players))); do
                                ((player[$(($x*8))] = $first))
                                ((player[$(($x*8+1))] = $second))
                                ((player[$(($x*8+2))] = $third))
                                ((player[$(($x*8+3))] = $forth))
                                ((player[$(($x*8+4))] = $fifth))
                                ((player[$(($x*8+5))] = $sixth))
                                ((player[$(($x*8+6))] = $seventh))
                                ((player[$(($x*8+7))] = $eighth))
                                #echo "set $i : ${player[$(($x*8))]} ${player[$(($x*8+1))]} ${player[$(($x*8+2))]} ${player[$(($x*8+3))]} ${player[$(($x*8+4))]} ${player[$(($x*8+5))]} ${player[$(($x*8+6))]} ${player[$(($x*8+7))]}"
                                x=$(($x+1))
                            done
                        done
                    done
                done
            done
        done
    done
done
# fifo
#echo "x = $x"
x=$(($x-1))
#echo "x = $x"
host_id=0
player_id=0
score=0
#read_count=0
for i in $(seq 0 $x); do
    #echo "i = $i"
    if [ $readyhost == 0 ]; then
        #echo "read when $i"
        #read_count=$(($read_count+1))
        read host_id < fifo_0.tmp
        for (( k = 1; k <= 8; k++ )); do
            read player_id score < fifo_0.tmp
            #echo "we got $player_id $score"
            ((points[$player_id]=$((${points[$player_id]}+$score))))
        done
        ready[$host_id]=1
        readyhost=1
    fi
    for j in $(seq 1 $n_hosts); do
            #echo "for loop when $i"
        if [ ${ready[$j]} == 1 ]; then #find ready host
            #echo "fifo when $i"
            #echo "set $i : ${player[$(($i*8))]} ${player[$(($i*8+1))]} ${player[$(($i*8+2))]} ${player[$(($i*8+3))]} ${player[$(($i*8+4))]} ${player[$(($i*8+5))]} ${player[$(($i*8+6))]} ${player[$(($i*8+7))]}"
            echo "${player[$(($i*8))]} ${player[$(($i*8+1))]} ${player[$(($i*8+2))]} ${player[$(($i*8+3))]} ${player[$(($i*8+4))]} ${player[$(($i*8+5))]} ${player[$(($i*8+6))]} ${player[$(($i*8+7))]}" > fifo_$j.tmp
            ready[$j]=0
            readyhost=$(($readyhost-1))
            break;
        fi
    done
done
#echo "r = $readyhost"
#echo "read for $read_count times"
while [ $readyhost -lt $n_hosts ]; do #確認都有讀到
    #read_count=$(($read_count+1))
    read host_id < fifo_0.tmp
    for i in $(seq 1 8); do
        read player_id score < fifo_0.tmp
        #echo "we got $player_id $score"
        ((points[$player_id]=$((${points[$player_id]}+$score))))
    done
    ready[$host_id]=1
    readyhost=$(($readyhost+1))
done
#echo "r = $readyhost"
for (( i = 1; i <= $n_hosts; i++ )); do
        echo "-1 -1 -1 -1 -1 -1 -1 -1" > fifo_$i.tmp
done
#ranking
#echo "points: ${points[*]}"
#echo "read for $read_count times"
for i in $(seq 1 $n_players); do
    big=1
    for j in $(seq 1 $n_players); do
        if [ ${points[$i]} -lt ${points[$j]} ]; then
            big=$(($big+1))
        fi
    done
    echo "$i $big"
done
#close
count=0
while [ $count -le $n_hosts ]; do
    fifo_name="fifo_$count.tmp"
    rm -r $fifo_name
    count=$(($count+1))
done
for i in ${pid[*]}; do
    wait $i
done
exit
