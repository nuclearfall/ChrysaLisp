#!/bin/bash

#process args defaults
num_cpu=8
emu=""
for var in "$@"
do
	if [ $var == "-e" ]
	then
		emu=$var
	else
		num_cpu=$var
	fi
done

#not greater than 8
if [ $num_cpu -gt 8 ]
then
	num_cpu=8
fi

source funcs.sh

for ((cpu_y=$num_cpu-1; cpu_y>=0; cpu_y--))
do
	for ((cpu_x=$num_cpu-1; cpu_x>=0; cpu_x--))
	do
		cpu=$(($cpu_y*$num_cpu + $cpu_x))
		links=""
		for ((lcpu_y=$cpu_y-1; lcpu_y<=$cpu_y+1; lcpu_y++))
		do
			wrap $lcpu_y
			zero_pad $cpu
			c1=$zp
			zero_pad $(($wp*$num_cpu + $cpu_x))
			c2=$zp
			add_link $c1 $c2
		done
		for ((lcpu_x=$cpu_x-1; lcpu_x<=$cpu_x+1; lcpu_x++))
		do
			wrap $lcpu_x
			zero_pad $cpu
			c1=$zp
			zero_pad $(($cpu_y*$num_cpu + $wp))
			c2=$zp
			add_link $c1 $c2
		done
		boot_cpu_gui $cpu $emu "$links"
	done
done
