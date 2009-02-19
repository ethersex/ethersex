#!/bin/bash

wavefile=aus1.wav
soxdfile=sox.dat
gnupfile=gnu.dat

sox $wavefile $soxdfile

tail -n +3 $soxdfile > $gnupfile

plotheader="# gnuplot file\n
set title 'SoX'\n
set grid xtics ytics\n
set key off\n
set style data lines\n"

# echo -e $plotheader | gnuplot

declare -a timearray
declare -a amplitude
declare -a code

echo "Read file $gnupfile"
timearray=($(cat "$gnupfile" | tr -s [:blank:] \\t | cut -f 2))
amplitude=($(cat "$gnupfile" | tr -s [:blank:] \\t | cut -f 3))


element_count_timearray=${#timearray[*]}
element_count_amplitude=${#amplitude[*]}

echo -e "Compute data..."
if [ $element_count_timearray -ne $element_count_amplitude ]
then
  echo fileformat error
  exit 1
fi

x=0
y=0
z=0
j=0
k=0
countrange=0
let element_count_timearray=element_count_timearray-10
for i in `seq 0 $element_count_timearray`
do
  rangelag=$(echo "${amplitude[$i]}<0.3&&${amplitude[$i]}>-0.7"|bc)
  if [ $rangelag -ne 0 ]
  then
    let countrange=$countrange+1
  else
    if [ $countrange == 0 ]
    then
      let j=i
    fi
    if [ $countrange -gt 70 ]
    then
      z=$( echo "${timearray[$i]}-${timearray[$j]}"|bc )
      y=$( echo "${timearray[$j]}-${timearray[$k]}-$z"|bc )
      flag=$(echo "$y<0.005"|bc)
      if [ $flag -ne 0 ]
      then
	code[x]=0
	echo -n ${code[x]}
	plotheader=$(echo -e "$plotheader\nset label '${code[x]}' at ${timearray[$k]},-0.6\n")
	let x=x+1
      else
	flag=$(echo "$y<0.013"|bc)
	if [ $flag -ne 0 ]
	then
	  code[x]=1
	  echo -n ${code[x]}
	  plotheader=$(echo -e "$plotheader\nset label '${code[x]}' at ${timearray[$k]},0.6\n")
	  let x=x+1
	fi
      fi
#       echo  ${timearray[$i]} $y $z
      let k=j
      code[x]=1
      echo -n ${code[x]}
      plotheader=$(echo -e "$plotheader\nset label '${code[x]}' at ${timearray[$i]},0.6\n")
      let x=x+1
    elif [ $countrange -gt 30 ]
    then
      z=$( echo "${timearray[$i]}-${timearray[$j]}"|bc )
      y=$( echo "${timearray[$j]}-${timearray[$k]}-$z"|bc )
      flag=$(echo "$y<0.005"|bc)
      if [ $flag -ne 0 ]
      then
	code[x]=0
	echo -n ${code[x]}
	plotheader=$(echo -e "$plotheader\nset label '${code[x]}' at ${timearray[$k]},-0.6\n")
	let x=x+1
      else
	flag=$(echo "$y<0.013"|bc)
	if [ $flag -ne 0 ]
	then
	  code[x]=1
	  echo -n ${code[x]}
	  plotheader=$(echo -e "$plotheader\nset label '${code[x]}' at ${timearray[$k]},0.6\n")
	  let x=x+1
	fi
      fi
#       echo ${timearray[$i]} $y $z
      let k=j
      code[x]=0
      echo -n ${code[x]}
      plotheader=$(echo -e "$plotheader\nset label '${code[x]}' at ${timearray[$i]},-0.6\n")
      let x=x+1
    fi
    countrange=0
  fi
  
done


element_count_codearray=${#code[*]}
echo -e "\nCodes:"
BIN=""
for i in `seq 0 7`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"

BIN=""
for i in `seq 8 15`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"

BIN=""
for i in `seq 16 23`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"


BIN=""
for i in `seq 25 32`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"

BIN=""
for i in `seq 33 40`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"


echo -e "$plotheader\n
plot '$gnupfile'\n
pause -1 'Hit return to continue'" > gnuplot.in #| gnuplot
echo -e "call:\n gnuplot gnuplot.in"
